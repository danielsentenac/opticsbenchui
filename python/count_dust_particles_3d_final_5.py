#!/usr/bin/env python3
import matplotlib
matplotlib.use('Agg')  # Safe for threads
import os
import sys
import argparse
import numpy as np
import h5py
import matplotlib.pyplot as plt
from skimage.measure import label, regionprops
from scipy.spatial.distance import cdist
from collections import Counter, defaultdict
from multiprocessing import Pool
from matplotlib.patches import Patch
from matplotlib.backends.backend_pdf import PdfPages
from pathlib import Path
from astropy.stats import sigma_clipped_stats
from mpl_toolkits.mplot3d import Axes3D
from tqdm import tqdm
from concurrent.futures import ProcessPoolExecutor, wait, FIRST_COMPLETED
from concurrent.futures.process import BrokenProcessPool
from scipy.ndimage import binary_dilation

# === Globals ===
size_bins = [(0.5, 1.0), (1.0, 2.5), (2.5, 5.0), (5.0, 10.0), (10.0, 50.0), (50.0, float('inf'))]
bin_labels = ["0.5–1 μm", "1–2.5 μm", "2.5–5 μm", "5–10 μm", "10–50 μm", ">50 μm"]
bin_color_map = {
    0: (1.0, 0, 0.5),
    1: (1.0, 0.498, 0.054),
    2: (0.173, 0.627, 0.173),
    3: (0.839, 0.153, 0.157),
    4: (0.580, 0.404, 0.741),
    5: (0.549, 0.337, 0.294)
}

class UnionFind:
    def __init__(self):
        self.parent = {}
        self.size = {}

    def find(self, x):
        if x not in self.parent:
            self.parent[x] = x
            self.size[x] = 0
        if self.parent[x] != x:
            self.parent[x] = self.find(self.parent[x])
        return self.parent[x]

    def union(self, x, y):
        rx, ry = self.find(x), self.find(y)
        if rx == ry:
            return
        if self.size[rx] < self.size[ry]:
            rx, ry = ry, rx
        self.parent[ry] = rx
        self.size[rx] += self.size[ry]

    def add_area(self, x, area):
        root = self.find(x)
        self.size[root] += area

    def get_area(self, x):
        return self.size[self.find(x)]
        
 
def is_structured_artifact(image, min_unique_row_frac=0.20):
    """
    Fast detector for striped / banded / structured frames.
    O(N) cost, no heavy ops.
    """
    import numpy as np

    if image.size == 0:
        return True, "empty image"

    rows = image.reshape(image.shape[0], -1)
    unique_rows = np.unique(rows, axis=0).shape[0]
    frac = unique_rows / image.shape[0]

    if frac < min_unique_row_frac:
        return True, f"row-banding (unique rows frac={frac:.3f})"

    return False, ""



# === Multiprocessing Support ===
def set_args_for_multiprocessing(a):
    global mp_args
    mp_args = a


def pixel_threshold_arg(value):
    value = float(value)
    if not 0.0 <= value <= 255.0:
        raise argparse.ArgumentTypeError("pixel threshold must be between 0 and 255")
    return value


def compress_pdf_with_ghostscript(in_pdf, out_pdf, preset="/ebook"):
    """
    Compress PDF with Ghostscript. Presets:
      /screen (lowest), /ebook (good), /printer, /prepress (highest)
    """
    import shutil, subprocess
    gs = shutil.which("gs")
    if not gs:
        return False, "Ghostscript 'gs' not found"
    cmd = [
        gs, "-sDEVICE=pdfwrite",
        f"-dPDFSETTINGS={preset}",
        "-dCompatibilityLevel=1.4",
        "-dDetectDuplicateImages=true",
        "-dDownsampleColorImages=true",
        "-dColorImageResolution=150",
        "-dDownsampleGrayImages=true",
        "-dGrayImageResolution=150",
        "-dDownsampleMonoImages=true",
        "-dMonoImageResolution=300",
        "-dNOPAUSE", "-dBATCH",
        f"-sOutputFile={out_pdf}", in_pdf
    ]
    try:
        subprocess.run(cmd, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return True, ""
    except subprocess.CalledProcessError as e:
        return False, e.stderr.decode("utf-8", "ignore")


def evaluate_histogram_anomaly(image, threshold):
    """
    SciPy-free detector of 'non-conventional' histograms.

    Flags as anomalous when:
      • there are ≥2 significant local maxima, AND
      • at least one peak is near the high end (≥85% of dynamic range), AND
      • either the threshold is near the max OR the high-end tail is heavy (≥10% pixels).
    Returns (is_bad: bool, reason: str).
    """
    import numpy as np

    vals = image.ravel().astype(float)
    if vals.size == 0 or not np.isfinite(vals).all():
        return True, "empty or non-finite image"

    vmin = float(np.min(vals)); vmax = float(np.max(vals))
    if not np.isfinite(vmin) or not np.isfinite(vmax):
        return True, "non-finite image"
    if vmax <= vmin:
        # All-zero is a valid clean frame; treat as NOT anomalous
        if vmax == 0.0 and vmin == 0.0:
            return False, ""
        # Any other constant (nonzero) image is suspicious
        return True, "constant image"


    # 256-bin histogram over observed range
    hist, edges = np.histogram(vals, bins=256, range=(vmin, vmax))
    if hist.max() == 0:
        return True, "empty histogram"

    # Simple local-max detection with a prominence floor (≥5% of max bin count)
    prom = max(1.0, 0.05 * float(hist.max()))
    peaks = []
    for i in range(1, len(hist) - 1):
        if hist[i] >= hist[i-1] and hist[i] >= hist[i+1] and hist[i] >= prom:
            peaks.append(i)

    centers = 0.5 * (edges[:-1] + edges[1:])
    high_edge = vmin + 0.85 * (vmax - vmin)
    high_peaks = [i for i in peaks if centers[i] >= high_edge]

    # Is the chosen threshold near the high end?
    if np.issubdtype(image.dtype, np.integer):
        thr_near_max = (threshold >= vmax - 1)
    else:
        thr_near_max = (threshold >= vmax - 1e-6) or (threshold >= vmin + 0.95 * (vmax - vmin))

    # Heavy high-end tail?
    frac_high = float((vals >= high_edge).sum()) / float(vals.size)

    if len(peaks) >= 2 and len(high_peaks) >= 1 and (thr_near_max or frac_high >= 0.10):
        return True, f"double-peaked/high-end (thr_near_max={thr_near_max}, frac_high={frac_high:.2f})"

    return False, ""


def process(entry):
    import h5py
    import os
    import numpy as np
    import matplotlib.pyplot as plt
    from matplotlib.patches import Patch
    from skimage.measure import label, regionprops
    from collections import Counter

    h5file = h5py.File(mp_args.h5file, 'r')        
    path, x, y, i, j = entry
    image = h5file[path][()]
    
    img_h, img_w = image.shape
    img_h, img_w = image.shape
    img_area_um2 = (img_h * img_w) * (mp_args.pixel_size ** 2)

    # ----- EARLY EXIT FOR CLEAN FRAMES (NO PLOT) -----
    if image.size > 0 and np.min(image) == 0 and np.max(image) == 0:
        h5file.close()
        return (
            path,
            x.item(),
            y.item(),
            Counter(),
            0.0,
            "OK_CLEAN_ZERO",
            0,
            img_area_um2,
        )

    threshold = float(mp_args.min_pixel_threshold)
    binary = image >= threshold
    dust_pixels = int(np.count_nonzero(binary))   # illuminated pixels in this frame
        # === HARD GUARD #2: too many illuminated pixels ===
    if dust_pixels > 0.25 * image.size:
        try:
            with open(os.path.join(mp_args.outdir, "warnings.txt"), "a") as wf:
                wf.write(
                    f"[SKIP] {path}: too many bright pixels "
                    f"({dust_pixels}/{image.size} = {dust_pixels/image.size:.2%})\n"
                )
        except Exception:
            pass

        h5file.close()
        return (
            path,
            x.item(),
            y.item(),
            None,
            threshold,
            "SKIPPED_TOO_MANY_PIXELS",
            None,
            img_area_um2,
        )

    # Detect non-conventional histograms (double-peaked with high-end peak)
    _is_bad, _reason = evaluate_histogram_anomaly(image, threshold)
    if _is_bad:
        try:
            with open(os.path.join(mp_args.outdir, "warnings.txt"), "a") as wf:
                wf.write(f"[SKIP] {path}: anomalous histogram ({_reason}); skipping.\n")
        except Exception:
            pass
        # If your code returns status to the summary (6-tuple):
        h5file.close()
        return path, x.item(), y.item(), None, threshold, "SKIPPED_ANOMALY", None, None
        
    if mp_args.debug:
        counts, overlay, label_map, region_bin_map = classify_particles_colored(binary, pixel_size=mp_args.pixel_size, proximity_threshold=mp_args.proximity_threshold, debug=True)
    else:
        counts, overlay = classify_particles_colored(binary, pixel_size=mp_args.pixel_size, proximity_threshold=mp_args.proximity_threshold)
        label_map = region_bin_map = None

    base = os.path.join(mp_args.outdir, path.replace("/", "_"))
    # If debug, write per-particle diagnostics CSV (one row per connected component)
    if mp_args.debug and label_map is not None and region_bin_map is not None:
        import csv
        csv_path = os.path.join(mp_args.outdir, path.replace('/', '_') + '_particles.csv')
        pix_per_label = np.bincount(label_map.ravel())
        unique_labels = np.nonzero(pix_per_label)[0]
        unique_labels = unique_labels[unique_labels != 0]
        with open(csv_path, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(['label_id','bin','pixel_count','diameter_um'])
            for lbl in unique_labels:
                pix = int(pix_per_label[lbl])
                area_um2 = pix * (mp_args.pixel_size ** 2)
                diam_um = (4 * area_um2 / np.pi) ** 0.5
                writer.writerow([int(lbl), int(region_bin_map.get(int(lbl), -1)), pix, float(diam_um)])

    
    if mp_args.debug:
        save_threshold_histogram(image, threshold, base + "_thresh.png")
        generate_overlay_debug(path, image, overlay, mp_args.outdir, label_map, region_bin_map)
        save_zoom_on_smallest(image, overlay, path, mp_args.outdir, proximity_threshold=mp_args.proximity_threshold)

    fig, ax = plt.subplots(figsize=(13, 10))
    ax.imshow(image, cmap='gray', alpha=0.1)
    ax.imshow(overlay, alpha=1.0)
    legend = [Patch(color=bin_color_map[i], label=f"{bin_labels[i]} ({counts[i]})") for i in sorted(counts.keys())]
    ax.legend(handles=legend, loc='lower right', fontsize=6)
    ax.set_title(f"{path} (Th={threshold:.2f})")
    ax.axis("off")
    fig.savefig(base + ".png", bbox_inches="tight")
    plt.close(fig)

    h5file.close()
    # Long-lived pool workers keep their peak RSS; release the full-frame
    # arrays and matplotlib caches before picking up the next frame.
    del image, binary, overlay, label_map, region_bin_map
    plt.close('all')
    import gc
    gc.collect()
    return path, x.item(), y.item(), counts, threshold, "OK", dust_pixels, img_area_um2

# === Processing Functions ===
def save_threshold_histogram(image, threshold, out_path):
    fig, ax = plt.subplots()
    ax.hist(image.ravel(), bins=256, color='gray', alpha=0.7)
    ax.axvline(threshold, color='red', linestyle='--', label=f"Threshold = {threshold:.2f}")
    ax.set_title("Pixel Histogram with Threshold")
    ax.set_xlabel("Pixel Value")
    ax.set_ylabel("Frequency (log)")
    ax.set_yscale("log")
    ax.legend()
    fig.tight_layout()
    fig.savefig(out_path)
    plt.close(fig)

def classify_particles_colored(binary_image, pixel_size=0.32, proximity_threshold=4.1, debug=False):
    """
    Classify connected components by size bins and build a color overlay.
    Robust to large streak-like artifacts: elongated, high-area components
    are treated as a single big dust (bin 5) and bypass the heavy merge logic.
    """
    import numpy as np
    from skimage.measure import label, regionprops
    from scipy.spatial import KDTree
    from collections import Counter

    # --- Heuristics for streak detection ---
    BIG_BIN5_DIAM_UM = 50.0                     # standard bin-5 threshold
    BIG_STREAK_ELONGATION = 6.0                 # major/minor ratio to call it a streak
    BIG_STREAK_MIN_MAJOR_UM = 50.0              # streak length in µm to consider "big dust"
    MAX_KDTREE_PTS = 3000                       # safety: skip KDTree for larger regions

    height, width = binary_image.shape
    label_map0 = label(binary_image, connectivity=2)
    props = regionprops(label_map0)

    # Prepare outputs — compact dtypes: with several workers on multi-MP
    # frames, float64/int64 full-frame arrays are what OOMs the node.
    color_mask = np.zeros((height, width, 3), dtype=np.float32)
    bin_index_map = np.full((height, width), -1, dtype=np.int8)
    counts = Counter()

    # We’ll collect only “normal” (small/medium) regions for merging
    # using the old per-pixel coord sets; streaks go straight to output.
    regions = {}

    # --- First pass: classify or fast-path streaks ---
    for rp in props:
        area_px = rp.area
        area_um2 = area_px * (pixel_size ** 2)
        diam_um = (4.0 * area_um2 / np.pi) ** 0.5

        major_um = (rp.major_axis_length or 0.0) * pixel_size
        minor_um = (rp.minor_axis_length or 0.0) * pixel_size
        elongation = (major_um / (minor_um + 1e-6)) if minor_um > 0 else np.inf

        # Decide bin by diameter
        def bin_for(d_um):
            for b, (low, high) in enumerate(size_bins):
                if low < d_um <= high:
                    return b
            return None

        # --- FAST PATH: streak-like artifact or very large particle ---
        if (diam_um > BIG_BIN5_DIAM_UM) or \
           (major_um >= BIG_STREAK_MIN_MAJOR_UM and elongation >= BIG_STREAK_ELONGATION):
            # Paint directly as bin 5
            b = 5
            rr, cc = rp.coords[:, 0], rp.coords[:, 1]
            color_mask[rr, cc, :] = bin_color_map[b]
            bin_index_map[rr, cc] = b
            counts[b] += 1
            continue

        # --- Normal component: keep for potential proximity merging ---
        b = bin_for(diam_um)
        if b is None:
            continue

        # For small-ish components we can afford per-pixel coords as a set.
        # For larger ones, keep a compact representation and avoid KDTree.
        coords = rp.coords
        if coords.shape[0] <= MAX_KDTREE_PTS:
            coord_set = set(map(tuple, coords))  # used by the merge loop
        else:
            coord_set = None  # mark as "large" to skip KDTree-based merging

        regions[rp.label] = {
            "coords": coord_set,
            "coords_arr": coords,    # still used to paint at the end
            "bin": b,
            "bbox": rp.bbox,
            "large": (coord_set is None)
        }

    # --- Proximity-based merging (only among “normal” small/medium regions) ---
    if regions:
        merged = True
        while merged:
            merged = False
            label_ids = list(regions.keys())

            # Build a lightweight label map for neighbor search
            lm = np.zeros((height, width), dtype=np.int32)
            for lid, R in regions.items():
                pts = R["coords_arr"]
                lm[pts[:, 0], pts[:, 1]] = lid

            for i_idx, lid_i in enumerate(label_ids):
                if lid_i not in regions:
                    continue
                Ri = regions[lid_i]
                # Skip merging for "large" regions to avoid KDTree cost
                if Ri["large"]:
                    continue

                coords_i_arr = Ri["coords_arr"]
                coords_i_set = Ri["coords"]
                bin_i = Ri["bin"]
                minr, minc, maxr, maxc = Ri["bbox"]

                pad = int(np.ceil(proximity_threshold))
                y1 = max(minr - pad, 0); y2 = min(maxr + pad, height)
                x1 = max(minc - pad, 0); x2 = min(maxc + pad, width)

                # Find neighbor labels in padded bbox
                nbr_labels = np.unique(lm[y1:y2, x1:x2])

                # KDTree for i
                tree_i = KDTree(coords_i_arr)

                merged_once = False
                for lid_j in nbr_labels:
                    if lid_j == 0 or lid_j == lid_i or lid_j not in regions:
                        continue
                    Rj = regions[lid_j]
                    # Skip merging if either side is "large"
                    if Rj["large"]:
                        continue

                    coords_j_arr = Rj["coords_arr"]
                    # Quick reject on extremes
                    if coords_j_arr.shape[0] == 0:
                        continue

                    # KDTree for j
                    tree_j = KDTree(coords_j_arr)
                    d_ij = tree_i.query(coords_j_arr, k=1)[0].min()
                    d_ji = tree_j.query(coords_i_arr, k=1)[0].min()
                    if min(d_ij, d_ji) > proximity_threshold:
                        continue

                    # Union (work on sets — safe, both are “small”)
                    merged_coords_set = coords_i_set | set(map(tuple, coords_j_arr))
                    area_um2 = len(merged_coords_set) * (pixel_size ** 2)
                    diam_um = (4.0 * area_um2 / np.pi) ** 0.5

                    # If merging would create a >50 µm particle, keep as-is (disallow creating new bin 5 here)
                    if 5 not in (bin_i, Rj["bin"]) and diam_um > BIG_BIN5_DIAM_UM:
                        continue

                    # Decide new bin
                    new_bin = 5 if (bin_i == 5 and Rj["bin"] == 5) else bin_for(diam_um)

                    # Update region i, drop j
                    new_coords_arr = np.array(list(merged_coords_set), dtype=int)
                    new_minr = max(new_coords_arr[:, 0].min() - pad, 0)
                    new_maxr = min(new_coords_arr[:, 0].max() + pad, height)
                    new_minc = max(new_coords_arr[:, 1].min() - pad, 0)
                    new_maxc = min(new_coords_arr[:, 1].max() + pad, width)

                    regions[lid_i] = {
                        "coords": merged_coords_set,
                        "coords_arr": new_coords_arr,
                        "bin": new_bin,
                        "bbox": (new_minr, new_minc, new_maxr, new_maxc),
                        "large": (new_coords_arr.shape[0] > MAX_KDTREE_PTS)
                    }
                    del regions[lid_j]
                    merged = True
                    merged_once = True
                    break  # restart outer loop

                if merged_once:
                    break
            # end for label_ids
    # end if regions

    # --- Final painting: add the merged “normal” regions to the overlay ---
     # NOTE: also track per-pixel bin indices so debug can get true labels.
    bin_index_map = np.full((height, width), -1, dtype=np.int8)  # <— NEW map
    # Paint streaks already colored in color_mask: mark them as bin 5
    streak_mask = (color_mask.sum(axis=-1) > 0)
    bin_index_map[streak_mask] = 5

    for R in regions.values():
        pts = R["coords_arr"]
        b = R["bin"]
        color_mask[pts[:, 0], pts[:, 1], :] = bin_color_map[b]
        bin_index_map[pts[:, 0], pts[:, 1]] = b
        counts[b] += 1

    if debug:
        # Build a label map from where we actually drew particles
        from skimage.measure import label, regionprops
        mask_any = (bin_index_map >= 0)
        final_label_map = label(mask_any, connectivity=2)

        # Majority bin per region → region_bin_map
        region_bin_map = {}
        for rp in regionprops(final_label_map):
            rr, cc = rp.coords[:, 0], rp.coords[:, 1]
            bins_here = bin_index_map[rr, cc]
            bins_here = bins_here[bins_here >= 0]
            if bins_here.size:
                # choose the most frequent bin in this region
                b = int(np.bincount(bins_here).argmax())
            else:
                b = 5  # defensive fallback
            region_bin_map[int(rp.label)] = b

        return counts, color_mask, final_label_map, region_bin_map
    else:
        return counts, color_mask





def find_image_paths(h5file, dataset_name, exclude_coords):
    entries = []
    for x_key, x_group in h5file.items():
        if not x_key.startswith("SCAN_X_"): continue
        i = int(x_key.split("_")[-1])
        ts_x = x_group.attrs.get("TS_X", i)
        for y_key, y_group in x_group.items():
            if not y_key.startswith("SCAN_Y_"): continue
            j = int(y_key.split("_")[-1])
            ts_y = y_group.attrs.get("TS_Y", j)
            if (i, j) in exclude_coords: continue
            if dataset_name in y_group:
                path = y_group[dataset_name].name
                entries.append((path, ts_x, ts_y, i, j))
    return entries

def _format_attr_value(val):
    """Render an HDF5 attribute value as a clean string (decode bytes, flatten arrays)."""
    if isinstance(val, (bytes, np.bytes_)):
        return val.decode("utf-8", "replace")
    if isinstance(val, np.ndarray):
        flat = val.reshape(-1)
        if flat.size == 1:
            return _format_attr_value(flat[0])
        return ", ".join(_format_attr_value(v) for v in flat.tolist())
    return str(val)


# HDF5 image-standard attributes and per-image stats to skip when listing
# camera properties (these are not camera settings).
_NON_CAMERA_ATTRS = {
    "CLASS", "IMAGE_VERSION", "IMAGE_SUBCLASS", "IMAGE_MINMAXRANGE",
    "IMAGE_WHITE_IS_ZERO", "IMAGE_COLORMODEL", "INTERLACE_MODE",
    "DISPLAY_ORIGIN", "PALETTE", "min", "max",
}


def extract_camera_properties(h5file, entries):
    """Read camera-property HDF5 attributes from the first image's dataset.

    Camera properties (exposure, gain, resolution, ...) are constant across a
    scan, so we only query the first image. Returns a list of (name, value)
    pairs, excluding the HDF5 image-standard attributes and per-image min/max.
    """
    if not entries:
        return []
    path = entries[0][0]
    try:
        dset = h5file[path]
    except KeyError:
        return []
    props = []
    for name in sorted(dset.attrs.keys()):
        if name in _NON_CAMERA_ATTRS:
            continue
        props.append((name, _format_attr_value(dset.attrs[name])))
    return props


def normalize_for_display(image, noise_sigmas=3.0):
    """Contrast-stretch an image for debug rendering only (never for analysis).

    Faint particles sit only a few counts above background while one bright
    particle can dominate matplotlib's auto-scale, rendering everything else
    near-black. Use an asinh stretch anchored on the sigma-clipped background
    (linear near the noise floor, logarithmic for bright pixels) so dim and
    bright particles are visible in the same frame.
    """
    img = np.asarray(image, dtype=np.float32)
    _, median, std = sigma_clipped_stats(img, sigma=3.0)
    scale = noise_sigmas * std
    if scale <= 0:
        scale = 1.0
    x = np.maximum(img - np.float32(median), np.float32(0)) / np.float32(scale)
    top = float(np.arcsinh(x.max()))
    if top <= 0:
        return np.zeros_like(img)
    return np.arcsinh(x) / np.float32(top)

def generate_overlay_debug(path, image, color_mask, outdir, label_map=None, region_bin_map=None):
    import matplotlib.pyplot as plt
    from matplotlib.patches import Rectangle
    import os

    base_path = os.path.join(outdir, path.replace("/", "_"))
    disp = normalize_for_display(image)

    for i in range(len(size_bins)):
        # isclose, not ==: color_mask is float32, the color table is float64
        mask = np.isclose(color_mask, bin_color_map[i], atol=1e-3).all(axis=-1)
        if np.count_nonzero(mask) > 0:
            # Dilate for display only: 1–2 px particles vanish when the full
            # frame is downsampled into the figure.
            mask_disp = binary_dilation(mask, iterations=2)
            # Same rendering as the all-particles overview image, but the
            # overlay contains only this bin's particles, in white for
            # maximum visibility.
            bin_overlay = np.zeros_like(color_mask)
            bin_overlay[mask_disp] = (1.0, 1.0, 1.0)
            fig, ax = plt.subplots()
            ax.imshow(image, cmap='gray', alpha=0.1)
            # nearest, not the default antialiasing: downsampling would
            # average the small white dots into gray (<255)
            ax.imshow(bin_overlay, alpha=1.0, interpolation='nearest')
            ax.set_title(f"{path} – {bin_labels[i]}")
            fig.savefig(f"{base_path}_debug_bin{i}.png", dpi=200)
            plt.close(fig)

    if label_map is not None and region_bin_map is not None:
        fig, ax = plt.subplots()
        ax.imshow(disp, cmap='gray', vmin=0.0, vmax=1.0, alpha=0.5)
        ax.imshow(color_mask, alpha=0.7)

        props = regionprops(label_map)

        for region in props:
            label_id = region.label
            bin_idx = region_bin_map.get(label_id, None)
            if bin_idx is None:
                continue
            minr, minc, maxr, maxc = region.bbox
            color = bin_color_map[bin_idx]
            rect = Rectangle((minc, minr), maxc - minc, maxr - minr,
                             linewidth=1.5, edgecolor=color, facecolor='none')
            ax.add_patch(rect)
            ax.text(minc, minr, f"{bin_labels[bin_idx]}", color='white', fontsize=6,
                    bbox=dict(facecolor='black', alpha=0.5, edgecolor='none', boxstyle='round,pad=0.2'))
        ax.set_title(f"{path} – Region Bounding Boxes")
        ax.axis("off")
        fig.savefig(f"{base_path}_debug_boxes.png")
        plt.close(fig)

def save_zoom_on_smallest(image, overlay, path, outdir, proximity_threshold=4.1, size=50):
    import matplotlib.pyplot as plt
    from matplotlib.patches import Rectangle
    import numpy as np
    from skimage.measure import label, regionprops
    from scipy.spatial.distance import cdist  # ensure available

    color = np.array(bin_color_map[0])
    mask = np.all(np.isclose(overlay, color, atol=1e-2), axis=-1)
    labeled = label(mask)
    regions = regionprops(labeled)
    if not regions:
        return

    centroids = np.array([region.centroid for region in regions])
    if len(centroids) <= 5:
        selected_regions = regions
    else:
        # Bounded farthest-point sampling (no while-loop risk)
        selected = [0]
        for _ in range(1, 5):
            dists = cdist([centroids[i] for i in selected], centroids)
            min_dists = dists.min(axis=0)
            next_idx = int(np.argmax(min_dists))
            if next_idx in selected:
                break
            selected.append(next_idx)
        selected_regions = [regions[i] for i in selected]

    for idx, region in enumerate(selected_regions):
        r, c = map(int, region.centroid)
        r1, r2 = max(0, r - size), min(image.shape[0], r + size)
        c1, c2 = max(0, c - size), min(image.shape[1], c + size)
        zoom_img = image[r1:r2, c1:c2]
        zoom_overlay = overlay[r1:r2, c1:c2]

        fig, ax = plt.subplots()
        ax.imshow(zoom_img, cmap='gray', alpha=0.3)
        ax.imshow(zoom_overlay, alpha=1.0)

        # Draw bbox
        minr, minc, maxr, maxc = region.bbox
        pad = int(np.ceil(proximity_threshold))
        box_r1 = max(minr - pad, r1)
        box_r2 = min(maxr + pad, r2)
        box_c1 = max(minc - pad, c1)
        box_c2 = min(maxc + pad, c2)
        rect = Rectangle((box_c1 - c1, box_r1 - r1),
                         box_c2 - box_c1, box_r2 - box_r1,
                         linewidth=1.5, edgecolor='yellow', facecolor='none')
        ax.add_patch(rect)

        ax.set_title(f"Zoom on 0.5–1μm @ ({r},{c})")
        ax.axis('off')
        # NOTE: no tight_layout() — it can hang on pathological elements
        base = os.path.join(outdir, f"{path.replace('/', '_')}_zoom_{idx}.png")
        fig.savefig(base, bbox_inches="tight")
        plt.close(fig)



def plot_3d_histogram(stats_per_bin, outdir):
    """
    3D histogram of particle counts per image per size bin.
    - Excludes zero-count entries (no bar at 0).
    - Uses a GLOBAL X scale computed from the max across all bins.
    - Z values are % occurrence = (count in bin / total # images) * 100,
      where the denominator is total images for that bin (including zeros).
    """
    import numpy as np
    import os
    import matplotlib.pyplot as plt

    # Precompute global xmax across bins (positive-only values)
    global_all = []
    for _b in range(0, len(size_bins)):
        _vals = stats_per_bin[_b]
        global_all.extend([vv for vv in _vals if vv > 0])

    def _nice_upper(n):
        if n <= 1:
            return 1
        if n <= 10:
            return 10 if n > 8 else int(np.ceil(n / 2.0) * 2)
        if n <= 50:
            return int(5 * np.ceil(n / 5.0))
        return int(10 * np.ceil(n / 10.0))

    global_max_bins = int(max(global_all)) if len(global_all) > 0 else 0
    global_xmax = _nice_upper(global_max_bins) if global_max_bins > 0 else 1

    # Build integer bins from 1..global_xmax (no 0 bin)
    bins = np.arange(1, global_xmax + 2, 1)

    fig = plt.figure(figsize=(13, 13))
    ax = fig.add_subplot(111, projection='3d')

    xpos_list, ypos_list, zpos_list = [], [], []
    dx_list, dy_list, dz_list = [], [], []

    for bin_idx in range(0, len(size_bins)):
        raw = stats_per_bin[bin_idx]
        n_total = len(raw) if raw is not None else 0
        values = [v for v in raw if v > 0]  # remove zeros

        if n_total == 0 or len(values) == 0:
            continue

        counts, edges = np.histogram(values, bins=bins)
        # convert to percentage of images
        perc = (counts.astype(float) / n_total) * 100.0

        xpos = edges[:-1]
        ypos = np.full_like(xpos, bin_idx)
        zpos = np.zeros_like(xpos)
        dx = (edges[1] - edges[0]) * np.ones_like(xpos)
        dy = 0.8 * np.ones_like(xpos)
        dz = perc

        xpos_list.extend(xpos)
        ypos_list.extend(ypos)
        zpos_list.extend(zpos)
        dx_list.extend(dx)
        dy_list.extend(dy)
        dz_list.extend(dz)

    # Guard against an empty scan (no particles in any bin): bar3d would raise
    # on zero-size arrays. Emit an empty figure instead of crashing.
    if xpos_list:
        colors = [bin_color_map[i] for i in ypos_list]
        ax.bar3d(xpos_list, ypos_list, zpos_list, dx_list, dy_list, dz_list,
                 color=colors, shade=True)

    ax.set_xlabel('Number of particles per Image (2.06mm^2)')
    ax.set_ylabel('Size Bin')
    ax.set_zlabel('% occurrence')
    ax.set_xlim(0, global_xmax + 10)
    ax.set_yticks(range(0, len(size_bins)))
    ax.set_yticklabels(bin_labels)
    ax.set_title('3D Distribution (% occurrence) of Particle Counts per Size Bin')

    fig.subplots_adjust(left=0.1, right=0.9, top=0.9, bottom=0.1)
    fig.savefig(os.path.join(outdir, "summary_3d_histogram.png"))
    plt.close()
def generate_pdf_report(stats_per_bin, outdir):
    
    pdf_path = os.path.join(outdir, "particle_report_full.pdf")
    all_pngs = sorted(Path(outdir).glob("*.png"))
    with PdfPages(pdf_path) as pdf:
        for img_path in all_pngs:
            fig, ax = plt.subplots(figsize=(15, 15))
            img = plt.imread(img_path)
            ax.imshow(img)
            ax.axis('off')
            ax.set_title(img_path.name, fontsize=8)
            pdf.savefig(fig)
            plt.close()

        stats_txt = os.path.join(outdir, "summary_stats.txt")
        if os.path.exists(stats_txt):
            with open(stats_txt, "r") as f:
                lines = f.readlines()
            for i in range(0, len(lines), 50):
                fig, ax = plt.subplots(figsize=(13, 10))
                ax.axis('off')
                ax.text(0.01, 0.99, ''.join(lines[i:i+50]), va='top', fontsize=10, family='monospace')
                fig.tight_layout()
                pdf.savefig(fig)
                plt.close()



def plot_per_bin_histograms_autoscale(stats_per_bin, outdir):
    """
    Plot per-bin histograms of "number of particles per image" with autoscaled axes.
    - Excludes zero-count entries (no bar at 0).
    - X-axis upper bound is chosen from the GLOBAL max across all bins (rounded up nicely).
    - Y-axis shows % occurrence = (count in bin / total # images) * 100.
      The denominator is the total number of images for that bin (including zeros).
    """
    import numpy as np
    import os
    import matplotlib.pyplot as plt

    # Collect all positive values across all bins to determine a global xmax
    all_pos = []
    for bin_idx in range(len(size_bins)):
        vals = stats_per_bin.get(bin_idx, [])
        all_pos.extend([v for v in vals if v > 0])

    def nice_upper(n):
        if n <= 1:
            return 1
        if n <= 10:
            return 10 if n > 8 else int(np.ceil(n / 2.0) * 2)
        if n <= 50:
            return int(5 * np.ceil(n / 5.0))
        return int(10 * np.ceil(n / 10.0))

    global_max = int(max(all_pos)) if len(all_pos) > 0 else 0
    xmax = nice_upper(global_max) if global_max > 0 else 1  # ensure at least [0..1]

    # Subplots (6 bins → 3x2)
    fig, axes = plt.subplots(3, 2, figsize=(14, 12), constrained_layout=True)
    axes = axes.ravel()

    # Build global integer bin edges from 1..xmax (inclusive)
    bins = np.arange(1, xmax + 2, 1)  # +1 to include the right edge


    for bin_idx in range(len(size_bins)):
        ax = axes[bin_idx]
        raw = stats_per_bin.get(bin_idx, [])
        values = [v for v in raw if v > 0]  # exclude zeros
        n_total = len(raw) if raw is not None else 0

        if n_total == 0 or len(values) == 0:
            # Draw empty hist with the correct bins/limits for consistent axes
            ax.hist([], bins=bins, edgecolor='black')
        else:
            weights = np.ones_like(values, dtype=float) * (100.0 / n_total)
            ax.hist(values, bins=bins, weights=weights, color=bin_color_map[bin_idx], alpha=0.9, edgecolor='black')

        ax.set_xlim(0, xmax + 10)  # keep 0 for context; no bar at 0 since bins start at 1
        ax.set_xlabel('Particles per image')
        ax.set_ylabel('% occurrence')
        ax.set_title(f'{bin_labels[bin_idx]}')
        ax.grid(True, which='both', axis='both', alpha=0.3, linestyle='--')

    # Hide any extra axes
    for k in range(len(size_bins), len(axes)):
        axes[k].axis('off')

    fig.suptitle('Distribution of particle counts per image (per size bin) — % occurrence (zeros removed)', fontsize=14)
    fig.savefig(os.path.join(outdir, 'summary_per_bin_histograms.png'), dpi=150)
    plt.close(fig)
    
def run_processing(entries, args):
    """Run process() over all entries with a stall guard that always terminates.

    Uses an idle timeout: if no image completes within ``args.timeout`` seconds,
    the pool is considered stalled (a worker crashed or deadlocked), the
    outstanding images are marked TIMEOUT, and the workers are force-killed so
    the script proceeds to write the summary instead of hanging forever.

    Worker shutdown never blocks (wait=False) — this avoids the ProcessPool
    join deadlock that can occur after the final task.
    """
    def _skip(entry, status):
        # Mirror the 8-tuple shape returned by process() for skipped frames.
        return (entry[0], entry[1], entry[2], None,
                float(args.min_pixel_threshold), status, None, None)

    idle_timeout = args.timeout if (args.timeout and args.timeout > 0) else None
    results = []
    # Recycle workers every few frames so RSS can't ratchet up to the peak
    # frame cost times the worker count (max_tasks_per_child needs py3.11+
    # and forces the spawn start method, hence the explicit initializer —
    # spawn workers do not inherit the fork-time mp_args global).
    try:
        ex = ProcessPoolExecutor(max_workers=max(1, args.threads),
                                 max_tasks_per_child=4,
                                 initializer=set_args_for_multiprocessing,
                                 initargs=(args,))
    except (TypeError, ValueError):
        ex = ProcessPoolExecutor(max_workers=max(1, args.threads),
                                 initializer=set_args_for_multiprocessing,
                                 initargs=(args,))
    fut_to_entry = {ex.submit(process, e): e for e in entries}
    pending = set(fut_to_entry)
    stalled = False
    bar = tqdm(total=len(fut_to_entry))
    try:
        while pending:
            done, pending = wait(pending, timeout=idle_timeout,
                                 return_when=FIRST_COMPLETED)
            if not done:
                # No image finished within the idle window -> treat as a stall.
                stalled = True
                break
            for fut in done:
                entry = fut_to_entry[fut]
                try:
                    results.append(fut.result())
                except BrokenProcessPool:
                    stalled = True
                    results.append(_skip(entry, "ERROR"))
                except Exception:
                    results.append(_skip(entry, "ERROR"))
                bar.update(1)
            if stalled:
                break
    finally:
        bar.close()
        if pending:
            if stalled:
                print(f"[WARN] processing stalled (no progress for {idle_timeout}s) — "
                      f"marking {len(pending)} unfinished image(s) as TIMEOUT and "
                      f"aborting workers so the run can finalize.", flush=True)
            for fut in pending:
                results.append(_skip(fut_to_entry[fut], "TIMEOUT"))
        # Capture worker handles BEFORE shutdown (shutdown may clear the dict),
        # SIGKILL them so nothing lingers holding the camera/pipes/CPU, then
        # shut down without ever blocking on a join.
        procs = list((getattr(ex, "_processes", None) or {}).values())
        for proc in procs:
            try:
                proc.kill()
            except Exception:
                pass
        try:
            ex.shutdown(wait=False, cancel_futures=True)
        except TypeError:
            ex.shutdown(wait=False)
    return results


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--min-pixel-threshold",
        "--pixel-threshold",
        dest="min_pixel_threshold",
        type=pixel_threshold_arg,
        default=5.0,
        help="Minimum pixel value kept for dust analysis (0-255, default: 5)",
    )
    parser.add_argument("--bg-percentile", type=float, default=None, help=argparse.SUPPRESS)
    parser.add_argument("--bg-min-intensity", type=float, default=None, help=argparse.SUPPRESS)
    parser.add_argument("h5file", help="HDF5 file")
    parser.add_argument("--dataset", default="IMAGE")
    parser.add_argument("--outdir", default="results")
    parser.add_argument("--threads", type=int, default=1)
    parser.add_argument("--pixel-size", type=float, default=0.32, help="Pixel size in µm/pixel")
    parser.add_argument("--exclude", nargs="*", default=[])
    parser.add_argument("--debug", action="store_true", help="Enable overlay debug and zoom")
    parser.add_argument("--proximity-threshold", type=float, default=4.1,
                        help="Distance in pixels to merge small particles (default: 4.1)")
    parser.add_argument("--pdf", action="store_true",
                        help="Also build the (slow) combined PDF report. Off by default; "
                             "the PNGs, summary_stats.txt and histograms are always produced.")
    parser.add_argument("--timeout", type=float, default=1800.0,
                        help="Stall guard (seconds): max time with NO image completing "
                             "before the pool is declared stalled, workers are killed, and "
                             "the run finalizes with partial results. MUST exceed the slowest "
                             "single-image processing time (with margin: parallel workers "
                             "complete in bursts, so the first completion can take as long as "
                             "one whole image). Set higher for very dusty/slow frames; 0 "
                             "disables the guard entirely. Default: 1800 (30 min).")
    args = parser.parse_args()

    if args.bg_percentile is not None or args.bg_min_intensity is not None:
        print(
            "[WARN] --bg-percentile and --bg-min-intensity are deprecated and ignored. "
            "Use --min-pixel-threshold instead.",
            flush=True,
        )

    os.makedirs(args.outdir, exist_ok=True)
    exclude_coords = {tuple(map(int, pair.split(","))) for pair in args.exclude if "," in pair}
    stats_per_bin = defaultdict(list)

    # Run processing in parallel (same as before)
    with h5py.File(args.h5file, "r") as f:
        entries = find_image_paths(f, args.dataset, exclude_coords)
        # Camera settings are constant across the scan: read them once, from
        # the first image's dataset attributes.
        camera_props = extract_camera_properties(f, entries)
    set_args_for_multiprocessing(args)
    results = run_processing(entries, args)

    # --- Aggregation (now with illuminated area & coverage) ---
    total_dust_pixels = 0                 # numerator (sum of illuminated pixels across frames)
    total_scanned_area_um2 = 0.0          # denominator (sum of per-frame sensor areas)
    n_images_seen = 0
    # For backward-compat, if img_area isn't returned, we’ll compute later using HDF5
    _paths_needing_area = []
    _have_any_img_area = False

    # Write the per-image lines and accumulate bin stats (same style as before)
    with open(os.path.join(args.outdir, "summary_stats.txt"), "w") as f:
        # --- Threshold configuration ---
        f.write("=== Pixel Threshold Parameters ===\n")
        f.write(f"Minimum pixel threshold: {args.min_pixel_threshold}\n")
        f.write("\n")

        # --- Camera properties (read once, from the first image) ---
        f.write("=== Camera properties (from first image) ===\n")
        if camera_props:
            f.write(f"Source: {entries[0][0]}\n")
            for _name, _val in camera_props:
                f.write(f"{_name}: {_val}\n")
        else:
            f.write("(no camera property attributes found)\n")
        f.write("\n")

        for res in results:
            n_images_seen += 1

            # Accept 5-, 6-, or 8-tuples:
            # 5: path, x, y, counts, threshold
            # 6: path, x, y, counts, threshold, status
            # 8: path, x, y, counts, threshold, status, dust_pixels, img_area_um2
            path, x, y = res[0], res[1], res[2]
            counts     = res[3] if len(res) >= 4 else None
            threshold  = res[4] if len(res) >= 5 else float("nan")
            status     = res[5] if len(res) >= 6 else ("OK" if counts is not None else "SKIPPED")
            dust_px    = res[6] if len(res) >= 7 else None
            img_area   = res[7] if len(res) >= 8 else None

            # Accumulate illuminated pixels if available
            if isinstance(dust_px, (int, float)):
                total_dust_pixels += int(dust_px)

            # Accumulate scanned area if provided; otherwise mark for later computation
            if isinstance(img_area, (int, float)) and np.isfinite(img_area):
                total_scanned_area_um2 += float(img_area)
                _have_any_img_area = True
            else:
                _paths_needing_area.append(path)

            # --- existing per-image logging ---
            if counts is None:
                tag = "SKIPPED"
                if status == "SKIPPED_ANOMALY":
                    tag = "SKIPPED (anomalous histogram)"
                elif status == "TIMEOUT":
                    tag = "TIMEOUT"
                elif status == "ERROR":
                    tag = "ERROR"
                try:
                    th_str = f"{threshold:.2f}"
                except Exception:
                    th_str = str(threshold)
                f.write(f"{path} (TS_X={x}, TS_Y={y}, Th={th_str})  {tag}\n")
                continue

            f.write(f"{path} (TS_X={x}, TS_Y={y}, Th={threshold:.2f})")
            for i in range(len(size_bins)):
                count = counts.get(i, 0)
                f.write(f", {bin_labels[i]}: {count}")
                stats_per_bin[i].append(count)
            f.write("\n")

        # If no per-frame area was returned, compute it now from HDF5 by summing each frame’s sensor area
        if not _have_any_img_area and len(_paths_needing_area) > 0:
            with h5py.File(args.h5file, "r") as hf:   # NB: not 'f' — that's the summary text file
                for p in _paths_needing_area:
                    img = hf[p][()]
                    h, w = img.shape[:2]
                    total_scanned_area_um2 += (h * w) * (args.pixel_size ** 2)

        # --- Existing summary over counts ---
        f.write("\n=== Summary ===\n")
        for i in range(len(size_bins)):
            vals = stats_per_bin[i]
            if vals:
                f.write(f"{bin_labels[i]}: avg = {np.mean(vals):.2f}, std = {np.std(vals):.2f}\n")

        f.write("\n=== Totals across whole scan ===\n")
        grand_total = 0
        for i in range(len(size_bins)):
            total_i = int(np.sum(stats_per_bin[i])) if stats_per_bin[i] else 0
            grand_total += total_i
            f.write(f"{bin_labels[i]}: total = {total_i}\n")
        f.write(f"ALL BINS (grand total): {grand_total}\n")

        # --- NEW: Area coverage ---
        px_size_um = args.pixel_size
        illuminated_area_um2 = float(total_dust_pixels) * (px_size_um ** 2)
        illuminated_area_mm2 = illuminated_area_um2 * 1e-6
        total_scanned_area_mm2 = total_scanned_area_um2 * 1e-6
        coverage_pct = (100.0 * illuminated_area_um2 / total_scanned_area_um2) if total_scanned_area_um2 > 0 else float("nan")

        f.write("\n=== Area coverage ===\n")
        f.write(f"Pixel size: {px_size_um:.4f} µm\n")
        f.write(f"Frames (seen): {n_images_seen}\n")
        f.write(f"Illuminated pixels (sum across frames): {total_dust_pixels}\n")
        f.write(f"Illuminated area: {illuminated_area_um2:.2f} µm²  ({illuminated_area_mm2:.6f} mm²)\n")
        f.write(f"Total scanned area: {total_scanned_area_um2:.2f} µm²  ({total_scanned_area_mm2:.6f} mm²)\n")
        f.write(f"Coverage: {coverage_pct:.4f} %\n")

    # --- Plots & PDF (unchanged from your script) ---
    x = np.arange(len(size_bins))
    means = [np.mean(stats_per_bin[i]) for i in x]
    stds  = [np.std(stats_per_bin[i]) for i in x]
    plt.figure(figsize=(13, 10))
    for i in x:
        plt.bar(i, means[i], yerr=stds[i], color=bin_color_map[i], capsize=5)
    plt.xticks(x, bin_labels, rotation=45)
    plt.ylabel("Avg Count")
    plt.tight_layout()
    plt.savefig(os.path.join(args.outdir, "summary_bar_chart.png"))
    plt.close()

    plot_per_bin_histograms_autoscale(stats_per_bin, args.outdir)
    plot_3d_histogram(stats_per_bin, args.outdir)

    # --- Combined PDF report (opt-in: slow on large/--debug runs) ---
    if args.pdf:
        generate_pdf_report(stats_per_bin, args.outdir)
        # try to compress it
        in_pdf  = os.path.join(args.outdir, "particle_report_full.pdf")
        out_pdf = os.path.join(args.outdir, "particle_report_ebook.pdf")
        ok, err = compress_pdf_with_ghostscript(in_pdf, out_pdf, preset="/ebook")
        if ok:
            print(f"[INFO] Compressed PDF written to {out_pdf}")
        else:
            print(f"[WARN] PDF compression skipped: {err}")
    else:
        print("[INFO] PDF report skipped (pass --pdf to generate it). "
              "PNGs, summary_stats.txt and histograms were written.", flush=True)

    # Guarantee termination: all outputs are written and flushed above. Bypass
    # the interpreter's normal shutdown, which can hang joining a stuck
    # multiprocessing manager thread / worker after a stall.
    print("[INFO] Done.", flush=True)
    sys.stdout.flush()
    sys.stderr.flush()
    os._exit(0)

if __name__ == "__main__":
    main()
