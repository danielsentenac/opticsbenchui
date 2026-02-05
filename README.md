## OpticsBenchUI

OpticsBenchUI is a Qt application for controlling optics-bench experiments and
automated scans across cameras, motors, DACs, lasers, and counters.

Hardware families include USB/IEEE1394/GigE/Andor/Raptor/Raspberry Pi/Allied Vision
cameras, Newport/New Focus/PI/Micos/Standa motors, NKT SuperK lasers, and
Advantech/Comedi/Raspberry Pi DACs.

### API Documentation (GitHub Pages)

API documentation can be published with GitHub Pages. Enable Pages on the repo
and point it at a `gh-pages` branch (or the `docs/` folder on `main`).

Suggested URL (update with your GitHub org/user):
- https://<github-username>.github.io/OPTICSBENCHUI/api/

Publish flow (gh-pages branch):
1) Build docs: `./make_api_doc.run`
2) Copy `docs/api/html/*` into the publish root (e.g., `docs/api/` or repo root).
3) Commit/push the `gh-pages` branch.

### External Libraries (.pro)

The `OpticsBenchUI.pro` file wires optional third‑party SDKs/libraries through
`DEFINES`, `INCLUDEPATH`, and `LIBS`. If a dependency is missing, the related
feature is disabled at build time.

Data and plotting (`HDF5` / `NO_HDF5`, `COMEDICOUNTER`, `COMEDIDAC`):
- HDF5 data files (HDF5 / NO_HDF5): HDF Group HDF5 library. (https://www.hdfgroup.org/solutions/hdf5/)
- Comedi drivers/libraries (COMEDICOUNTER, COMEDIDAC / NO_COMEDI): Comedi project. (http://www.comedi.org)
- Qwt plotting (COMEDICOUNTER): Qwt project. (https://qwt.sourceforge.io/)

Cameras (`USBCAMERA` / `NO_MULTIMEDIA`, `IEEE1394CAMERA`, `GIGECAMERA`, `NEOCAMERA`, `ZYLACAMERA`, `RAPTORFALCONCAMERA`, `RAPTORNINOX640CAMERA`, `RASPICAMERA`, `ALLIEDVISIONCAMERA`, `NO_USB`):
- libusb (NO_USB / USB comm): USB device access. (https://libusb.info/)
- libdc1394 (IEEE1394CAMERA / NO_DC1394): IEEE1394/DCAM camera control. (http://damien.douxchamps.net/ieee1394/libdc1394/)
- libraw1394 (IEEE1394CAMERA / NO_RAW1394): Linux FireWire stack. (https://www.kernel.org/doc/html/latest/driver-api/firewire.html)
- Aravis (GIGECAMERA / NO_ARAVIS): GenICam GigE/USB3 Vision. (https://github.com/AravisProject/aravis)
- Andor SDK3 (NEOCAMERA, ZYLACAMERA / NO_NEO): Andor/Oxford Instruments SDK. (https://andor.oxinst.com/products/software-development-kit)
- Allied Vision Vimba X (ALLIEDVISIONCAMERA): Allied Vision SDK. (https://www.alliedvision.com/en/products/software/vimba-x-sdk/)
- Raspberry Pi raspicam (RASPICAMERA / NO_RASPI): UCO Raspberry Pi camera library. (http://www.uco.es/investiga/grupos/ava/node/40)
- Raptor xclib (RAPTORFALCONCAMERA, RAPTORNINOX640CAMERA / NO_RAPTOR): Raptor/EPIX frame grabber SDKs. (https://www.raptorphotonics.com/support/) and (https://www.epixinc.com/products/xclib.htm)

Motors (`NO_STANDA` / Standa when available):
- Standa XIMC (NO_STANDA): Standa/XIMC SDK. (https://libximc.xisupport.com/)

DACs and SPI (`ADVANTECHDAC` / `NO_ADVDAQ`, `RASPIDAC` / `NO_SPI`):
- Advantech DAQNavi (ADVANTECHDAC / NO_ADVDAQ): Advantech SDK. (https://us2.advantech.com/DAQNavi/)
- bcm2835 SPI (RASPIDAC / NO_SPI): bcm2835 C library. (http://www.airspayce.com/mikem/bcm2835/)

### Hardware Protocols (by Build Option)

#### Cameras

| Hardware (define) | Protocol | SDK/Library | Notes |
| --- | --- | --- | --- |
| USB camera (`USBCAMERA`) | USB | Qt Multimedia (https://doc.qt.io/qt-6/qtmultimedia-index.html) | Uses OS camera backend via Qt Multimedia. |
| IEEE1394 camera (`IEEE1394CAMERA`) | IEEE 1394 (FireWire) | libdc1394 (https://vcpkg.link/ports/libdc1394) + libraw1394 (https://ieee1394.docs.kernel.org/en/latest/libraw1394.html) | Requires FireWire host/controller. |
| GigE/USB3 GenICam (`GIGECAMERA`) | GigE Vision (Ethernet) / USB3 Vision | Aravis (https://github.com/AravisProject/aravis) | GenICam stack for GigE/USB3 cameras. |
| Andor Zyla (`ZYLACAMERA`) | USB 3.0 or Camera Link | Andor SDK3 (https://andor.oxinst.com/products/software-development-kit/software-development-kit) | Vendor SDK required. |
| Andor Neo (`NEOCAMERA`) | Camera Link (3-tap) | Andor SDK3 (https://andor.oxinst.com/products/software-development-kit/software-development-kit) | Requires Camera Link frame grabber. |
| Allied Vision (`ALLIEDVISIONCAMERA`) | GigE Vision or USB 3.0 | Vimba X SDK (https://www.alliedvision.com/en/products/vimba-sdk) | Transport layer depends on camera model. |
| Raptor Falcon/Ninox640 (`RAPTORFALCONCAMERA`, `RAPTORNINOX640CAMERA`) | Camera Link (frame grabber) | EPIX XCLIB (https://www.epixinc.com/products/xclib.htm) | Requires frame grabber + XCLIB. |
| Raspberry Pi camera (`RASPICAMERA`) | CSI-2 (Unicam) | raspicam (https://www.uco.es/investiga/grupos/ava/portfolio/raspicam/) | Requires Raspberry Pi CSI-2 connector. |

#### Motors

| Hardware (define) | Protocol | SDK/Library | Notes |
| --- | --- | --- | --- |
| New Focus 8750 (`DriverNewFocus_8750_*`) | RS-232 / RS-485 / Ethernet | New Focus 8750 manual (https://artisantg.manualzilla.com/85/8750-Manual.610325539.pdf) | Serial or TCP/IP depending on controller setup. |
| Newport NSC200/PZC200 (`DriverNewPort_NSC200`) | RS-485 | Newport PZC200 (https://www.newport.com/p/pzc200) | RS-485 bus; USB adapters OK. |
| Newport AG-UC2 (`DriverNewPort_AGUC2`) | USB | AG-UC2 controller (https://www.newport.com/p/AG-UC2) | USB device controller. |
| PI C-862/C-863 Mercury (`DriverPI_C862`) | RS-232 or USB | PI Mercury C-863 (https://www.pi-usa.us/en/products/controllers-drivers/mercury-embedded-motion-controller-10244/) | Mercury controller; serial or USB. |
| PI E-725 (`DriverPI_E725`) | Ethernet / USB / RS-232 | E-725 datasheet (https://static.pimicos.com/fileadmin/user_upload/pimicos/physik_instrumente/files/1_Datasheets/E-725_DataSheet.pdf) | Ethernet preferred for multi-axis. |
| Micos SMC Pollux (`DriverMicos_Pollux`) | RS-232 | Micos SMC Pollux (https://www.mic-os.com/products/gearheads-stages-and-actuators/servo/8smc5.html) | Serial controller. |
| Standa uSMC2 (`NO_STANDA` / available) | USB / RS-232 / Ethernet | libximc (https://pypi.org/project/libximc/) | Vendor library provides all transports. |
| Native serial/Ethernet | RS-232/RS-485, TCP/IP | POSIX termios (https://pubs.opengroup.org/onlinepubs/7908799/xsh/tcsetattr.html) + POSIX sockets (https://pubs.opengroup.org/onlinepubs/009695399/functions/connect.html) | Used for controllers without SDK. |

#### Lasers

| Hardware (define) | Protocol | SDK/Library | Notes |
| --- | --- | --- | --- |
| NKT SuperK (`SUPERK`) | USB / RS-232 / Ethernet | NKT CONTROL software (https://www.nktphotonics.com/products/femtosecond-lasers/nkt-photonics-laser-control-software/) | Control via NKT software/SDK. |

#### SLM

| Hardware (define) | Protocol | SDK/Library | Notes |
| --- | --- | --- | --- |
| Spatial Light Modulator (`SLM`) | HDMI/DisplayPort (secondary display) | OS display stack (X11/Wayland/Windows) | Appears as an external monitor; masks are rendered full-screen. |

### Build

- Qt 5: `qmake-qt5 OpticsBenchUI.pro` then `make -j4`
- Qt 6: `qmake6 OpticsBenchUI.pro` then `make -j4`
