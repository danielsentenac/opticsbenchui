/*******************************************************************
This file is part of OpticsBenchUI.

OpticsBenchUI is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

/**
 * @file CameraUSB.cpp
 * @brief Camera backend for USB.
 */
#ifdef USBCAMERA

#include <QCoreApplication>
#include <QCamera>
#include <QImage>
#include <QDebug>
#include <algorithm>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QCameraImageCapture>
#include <QVideoFrame>
#include <QVideoProbe>
#include <QAbstractVideoBuffer>
#include <QCameraImageProcessing>
#endif
#include "CameraUSB.h"
#include "Utils.h"

namespace {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QImage ImageFromVideoFrame(QVideoFrame frame)
{
    if (!frame.isValid()) {
        return QImage();
    }

    if (!frame.map(QAbstractVideoBuffer::ReadOnly)) {
        return QImage();
    }

    const QImage::Format format = QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat());
    QImage image;
    if (format != QImage::Format_Invalid) {
        image = QImage(frame.bits(),
                       frame.width(),
                       frame.height(),
                       frame.bytesPerLine(),
                       format).copy();
    }
    frame.unmap();
    return image;
}

bool IsLikelyIpu6Camera(const QString &text)
{
    const QString lower = text.toLower();
    return lower.contains("ipu6") || lower.contains("intel ipu6");
}
#endif

enum UsbFeatureId {
    UsbBrightness = 0,
    UsbContrast = 1,
    UsbSaturation = 2,
    UsbSharpening = 3
};
} // namespace

namespace {
constexpr double kUiMin = 0.0;
constexpr double kUiMax = 100.0;
constexpr double kQtMin = -1.0;
constexpr double kQtMax = 1.0;

double Clamp(double value, double minValue, double maxValue)
{
    return std::max(minValue, std::min(maxValue, value));
}

double QtToUi(double qtValue)
{
    const double clamped = Clamp(qtValue, kQtMin, kQtMax);
    const double normalized = (clamped - kQtMin) / (kQtMax - kQtMin);
    return kUiMin + normalized * (kUiMax - kUiMin);
}

double UiToQt(double uiValue)
{
    const double clamped = Clamp(uiValue, kUiMin, kUiMax);
    const double normalized = (clamped - kUiMin) / (kUiMax - kUiMin);
    return kQtMin + normalized * (kQtMax - kQtMin);
}
} // namespace

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
namespace {
QString PixelFormatToString(QVideoFrame::PixelFormat format)
{
    switch (format) {
        case QVideoFrame::Format_ARGB32:
            return "ARGB32";
        case QVideoFrame::Format_ARGB32_Premultiplied:
            return "ARGB32_Premultiplied";
        case QVideoFrame::Format_RGB32:
            return "RGB32";
        case QVideoFrame::Format_RGB24:
            return "RGB24";
        case QVideoFrame::Format_RGB565:
            return "RGB565";
        case QVideoFrame::Format_RGB555:
            return "RGB555";
        case QVideoFrame::Format_BGR24:
            return "BGR24";
        case QVideoFrame::Format_BGR32:
            return "BGR32";
        case QVideoFrame::Format_YUYV:
            return "YUYV";
        case QVideoFrame::Format_UYVY:
            return "UYVY";
        case QVideoFrame::Format_YUV420P:
            return "YUV420P";
        case QVideoFrame::Format_NV12:
            return "NV12";
        case QVideoFrame::Format_NV21:
            return "NV21";
        case QVideoFrame::Format_Jpeg:
            return "JPEG";
        case QVideoFrame::Format_Invalid:
            return "Invalid";
        default:
            return "Unknown";
    }
}
} // namespace
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
namespace {
int PixelFormatBitsPerPixel(QVideoFrame::PixelFormat format)
{
    switch (format) {
        case QVideoFrame::Format_ARGB32:
        case QVideoFrame::Format_ARGB32_Premultiplied:
        case QVideoFrame::Format_RGB32:
        case QVideoFrame::Format_BGR32:
            return 32;
        case QVideoFrame::Format_RGB24:
        case QVideoFrame::Format_BGR24:
            return 24;
        case QVideoFrame::Format_RGB565:
        case QVideoFrame::Format_RGB555:
            return 16;
        case QVideoFrame::Format_YUYV:
        case QVideoFrame::Format_UYVY:
            return 16;
        case QVideoFrame::Format_YUV420P:
        case QVideoFrame::Format_NV12:
        case QVideoFrame::Format_NV21:
            return 12;
        case QVideoFrame::Format_Jpeg:
            return 0;
        case QVideoFrame::Format_Invalid:
        default:
            return 0;
    }
}
} // namespace
#endif

namespace {
void UpdateStatsFromGrayscale(const QImage &image, int &minValue, int &maxValue, int &avgValue)
{
    if (image.isNull()) {
        minValue = 0;
        maxValue = 0;
        avgValue = 0;
        return;
    }

    QImage gray = image;
    if (gray.format() != QImage::Format_Grayscale8) {
        gray = image.convertToFormat(QImage::Format_Grayscale8);
    }

    const int width = gray.width();
    const int height = gray.height();
    if (width <= 0 || height <= 0) {
        minValue = 0;
        maxValue = 0;
        avgValue = 0;
        return;
    }

    int minPixel = 255;
    int maxPixel = 0;
    qint64 sum = 0;
    for (int y = 0; y < height; y++) {
        const uchar *row = gray.constScanLine(y);
        for (int x = 0; x < width; x++) {
            const int value = row[x];
            minPixel = std::min(minPixel, value);
            maxPixel = std::max(maxPixel, value);
            sum += value;
        }
    }

    const int total = width * height;
    minValue = minPixel;
    maxValue = maxPixel;
    avgValue = total > 0 ? static_cast<int>(sum / total) : 0;
}
} // namespace

CameraUSB::CameraUSB()
    : Camera(),
      camera(nullptr),
      imageCapture(nullptr),
      cameraIndex(-1),
      imageWidth(0),
      imageHeight(0)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    cameras = QCameraInfo::availableCameras();
    viewfinder = new QCameraViewfinder();
#else
    cameras = QMediaDevices::videoInputs();
#endif
    suspend = true;
    has_started = false;
    mutex = new QMutex(QMutex::NonRecursive);
    snapshotMutex = new QMutex(QMutex::Recursive);
    acquireMutex = new QMutex(QMutex::Recursive);
    acqstart = new QWaitCondition();
    acqend = new QWaitCondition();
    frameAvailable = false;
}

CameraUSB::~CameraUSB()
{
    stop();
  if (imageCapture) {
    imageCapture->setParent(nullptr);
    delete imageCapture;
    imageCapture = nullptr;
  }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (videoProbe) {
        videoProbe->setSource(static_cast<QMediaObject*>(nullptr));
        delete videoProbe;
        videoProbe = nullptr;
    }
    if (viewfinder) {
        delete viewfinder;
        viewfinder = nullptr;
    }
#endif
    if (camera) {
        delete camera;
        camera = nullptr;
    }
    delete mutex;
    delete snapshotMutex;
    delete acquireMutex;
    delete acqstart;
    delete acqend;
}

void CameraUSB::stop()
{
  suspend = true;
  has_started = false;
  wait();
  exit();
}

void CameraUSB::resetBackend()
{
  if (imageCapture) {
    imageCapture->setParent(nullptr);
    delete imageCapture;
    imageCapture = nullptr;
  }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (videoProbe) {
        videoProbe->setSource(static_cast<QMediaObject*>(nullptr));
        delete videoProbe;
        videoProbe = nullptr;
    }
#endif

    if (camera) {
        camera->stop();
        delete camera;
        camera = nullptr;
    }
}

int CameraUSB::findCamera()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    cameras = QCameraInfo::availableCameras();
#else
    cameras = QMediaDevices::videoInputs();
#endif

    if (cameras.isEmpty()) {
        Utils::EmitWarning(this, __FUNCTION__, "No cameras found.");
        num = 0;
        return -1;
    }

    cameralist.clear();
    vendorlist.clear();
    modelist.clear();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QList<QCameraInfo> filtered;
    for (const QCameraInfo &info : cameras) {
        const QString desc = info.description();
        const QString name = info.deviceName();
        if (IsLikelyIpu6Camera(desc) || IsLikelyIpu6Camera(name)) {
            continue;
        }
        filtered.push_back(info);
    }
    cameras = filtered;
    for (const QCameraInfo &info : cameras) {
        cameralist.push_back(new QCameraInfo(info));
        vendorlist.push_back(QStringLiteral("USB"));
        const QString model = info.description().isEmpty()
                                  ? info.deviceName()
                                  : info.description();
        modelist.push_back(model);
    }
#else
    for (const QCameraDevice &device : cameras) {
        cameralist.push_back(new QCameraDevice(device));
        const QString vendor = device.manufacturer().isEmpty()
                                   ? QStringLiteral("USB")
                                   : device.manufacturer();
        const QString model = device.description().isEmpty()
                                  ? QStringLiteral("Camera")
                                  : device.description();
        vendorlist.push_back(vendor);
        modelist.push_back(model);
    }
#endif

    num = cameras.size();
    return 0;
}

void CameraUSB::setCamera(void *_camera, int _id)
{
    stop();
    resetBackend();
    
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (_camera != nullptr) {
        const QCameraInfo *info = static_cast<QCameraInfo *>(_camera);
        camera = new QCamera(*info);
        vendor = "USB";
        model = info->description().isEmpty()
                    ? info->deviceName()
                    : info->description();
    } else {
        if (_id < 0 || _id >= cameras.size()) {
            Utils::EmitWarning(this, __FUNCTION__, "Invalid camera ID.");
            return;
        }
        camera = new QCamera(cameras[_id]);
        vendor = "USB";
        model = cameras[_id].description().isEmpty()
                    ? cameras[_id].deviceName()
                    : cameras[_id].description();
    }
#else
    if (_camera != nullptr) {
        const QCameraDevice *device = static_cast<QCameraDevice *>(_camera);
        camera = new QCamera(*device);
        vendor = device->manufacturer().isEmpty()
                     ? QStringLiteral("USB")
                     : device->manufacturer();
        model = device->description().isEmpty()
                    ? QStringLiteral("Camera")
                    : device->description();
    } else {
        if (_id < 0 || _id >= cameras.size()) {
            Utils::EmitWarning(this, __FUNCTION__, "Invalid camera ID.");
            return;
        }
        camera = new QCamera(cameras[_id]);
        vendor = cameras[_id].manufacturer().isEmpty()
                     ? QStringLiteral("USB")
                     : cameras[_id].manufacturer();
        model = cameras[_id].description().isEmpty()
                    ? QStringLiteral("Camera")
                    : cameras[_id].description();
    }
#endif

    cameraIndex = _id;
    camera_err = connectCamera();
    cameraIndex = _id;
    getProps();
}

void CameraUSB::getFeatures()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    featureNameList.clear();
    featureIdList.clear();
    featureValueList.clear();
    featureMinList.clear();
    featureMaxList.clear();
    featureModeAutoList.clear();
    featureAutoCapableList.clear();
    featureAbsValueList.clear();
    featureAbsCapableList.clear();

    modeCheckEnabled = false;

    if (!camera) {
        return;
    }

    QCameraImageProcessing *proc = camera->imageProcessing();
    if (!proc) {
        return;
    }

    struct FeatureSpec {
        int id;
        const char *name;
        qreal value;
    };

    const FeatureSpec specs[] = {
        {UsbBrightness, "Brightness", QtToUi(proc->brightness())},
        {UsbContrast, "Contrast", QtToUi(proc->contrast())},
        {UsbSaturation, "Saturation", QtToUi(proc->saturation())},
        {UsbSharpening, "Sharpening", QtToUi(proc->sharpeningLevel())}
    };

    for (const FeatureSpec &spec : specs) {
        featureIdList.push_back(spec.id);
        featureNameList.push_back(QString::fromLatin1(spec.name));
        featureValueList.push_back(spec.value);
        featureMinList.push_back(kUiMin);
        featureMaxList.push_back(kUiMax);
        featureModeAutoList.push_back(false);
        featureAutoCapableList.push_back(false);
        featureAbsValueList.push_back(spec.value);
        featureAbsCapableList.push_back(true);
    }
#endif
}

void CameraUSB::processImage(const QImage &image)
{
    if (image.isNull()) {
        return;
    }

    QImage processed = image;

    if (vflip || hflip) {
        processed = processed.mirrored(hflip != 0, vflip != 0);
    }

    QImage grayImage = processed.convertToFormat(QImage::Format_Grayscale8);
    QImage display = processed;
    if (table == &this->hot || table == &this->gray) {
        QImage indexed = grayImage.convertToFormat(QImage::Format_Indexed8);
        indexed.setColorTable(*table);
        display = indexed.convertToFormat(QImage::Format_RGB32);
    }

    width = grayImage.width();
    height = grayImage.height();

    UpdateStatsFromGrayscale(grayImage, min, max, avg);
    emit updateMin(min);
    emit updateMax(max);
    emit updateAvg(avg);

    snapshotMutex->lock();
    lastImage = display;
    snapshotMutex->unlock();

    frameMutex.lock();
    frameAvailable = true;
    frameReady.wakeAll();
    frameMutex.unlock();

    emit getImage(display);
}

uchar* CameraUSB::getSnapshot()
{
    snapshotMutex->lock();
    if (lastImage.isNull()) {
        snapshotMutex->unlock();
        return nullptr;
    }

    QImage gray = lastImage.convertToFormat(QImage::Format_Grayscale8);
    width = gray.width();
    height = gray.height();
    const int size = width * height;
    if (snapshot == nullptr || BufSize != size) {
        if (snapshot) {
            free(snapshot);
        }
        snapshot = (uchar *)malloc(sizeof(uchar) * size);
        BufSize = size;
    }
    memcpy(snapshot, gray.constBits(), size);
    snapShotMin = min;
    snapShotMax = max;
    snapShotAvg = avg;
    snapshotMutex->unlock();
    return snapshot;
}

ushort* CameraUSB::getSnapshot16()
{
    // Not implemented
    return nullptr;
}

int* CameraUSB::getSnapshot32()
{
    // Not implemented
    return nullptr;
}

void CameraUSB::setImageSize(const int &_imageWidth, const int &_imageHeight)
{
    imageWidth = _imageWidth;
    imageHeight = _imageHeight;
    width = imageWidth;
    height = imageHeight;
    
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (camera) {
        const QList<QCameraViewfinderSettings> supported = camera->supportedViewfinderSettings();
        if (!supported.isEmpty()) {
            auto isCompatible = [](const QCameraViewfinderSettings &setting) {
                return QVideoFrame::imageFormatFromPixelFormat(setting.pixelFormat())
                       != QImage::Format_Invalid;
            };

            QCameraViewfinderSettings chosen;
            bool hasChosen = false;
            for (const QCameraViewfinderSettings &setting : supported) {
                if (setting.resolution().width() == imageWidth
                    && setting.resolution().height() == imageHeight
                    && isCompatible(setting)) {
                    chosen = setting;
                    hasChosen = true;
                    break;
                }
            }
            if (!hasChosen) {
                for (const QCameraViewfinderSettings &setting : supported) {
                    if (isCompatible(setting)) {
                        chosen = setting;
                        hasChosen = true;
                        break;
                    }
                }
            }
            if (!hasChosen) {
                chosen = supported.first();
            }
            camera->setViewfinderSettings(chosen);
        }
    }
#else
    if (!camera) {
        return;
    }

    const QList<QCameraFormat> formats = camera->cameraDevice().videoFormats();
    for (const QCameraFormat &format : formats) {
        if (format.resolution().width() == imageWidth
            && format.resolution().height() == imageHeight) {
            camera->setCameraFormat(format);
            break;
        }
    }
#endif
}

void CameraUSB::setFeature(int feature, double value)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (!camera) {
        return;
    }
    QCameraImageProcessing *proc = camera->imageProcessing();
    if (!proc) {
        return;
    }

    const double qtValue = UiToQt(value);
    switch (feature) {
        case UsbBrightness:
            proc->setBrightness(qtValue);
            break;
        case UsbContrast:
            proc->setContrast(qtValue);
            break;
        case UsbSaturation:
            proc->setSaturation(qtValue);
            break;
        case UsbSharpening:
            proc->setSharpeningLevel(qtValue);
            break;
        default:
            break;
    }

    for (int i = 0; i < featureIdList.size(); i++) {
        if (featureIdList.at(i) == feature) {
            featureValueList[i] = value;
            featureAbsValueList[i] = value;
            break;
        }
    }
#endif
}

void CameraUSB::setMode(int feature, bool value)
{
    // Not implemented
}

void CameraUSB::getProps()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    propList.clear();

    if (!camera) {
        return;
    }

    QSize resolution = camera->viewfinderSettings().resolution();
    if (!resolution.isValid()) {
        resolution = QSize(width, height);
    }

    QString resStr = "Resolution : " + QString::number(resolution.width()) +
                     "x" + QString::number(resolution.height());
    propList.push_back(resStr);

    QCameraViewfinderSettings settings = camera->viewfinderSettings();
    const QVideoFrame::PixelFormat fmt = settings.pixelFormat();
    const int bpp = PixelFormatBitsPerPixel(fmt);
    QString formatStr = "Pixel Format : " + PixelFormatToString(fmt);
    if (bpp > 0) {
        formatStr.append(" (" + QString::number(bpp) + " bpp)");
    } else {
        formatStr.append(" (" + QString::number((int)fmt) + ")");
    }
    propList.push_back(formatStr);

    const double fps = settings.maximumFrameRate() > 0.0
                           ? settings.maximumFrameRate()
                           : settings.minimumFrameRate();
    QString fpsStr = "Frame Rate : " + QString::number((int)fps) + " Hz";
    propList.push_back(fpsStr);

    QCameraImageProcessing *proc = camera->imageProcessing();
    if (proc) {
        propList.push_back("Brightness : " + QString::number(QtToUi(proc->brightness()), 'f', 1));
        propList.push_back("Contrast : " + QString::number(QtToUi(proc->contrast()), 'f', 1));
        propList.push_back("Saturation : " + QString::number(QtToUi(proc->saturation()), 'f', 1));
        propList.push_back("Sharpening : " + QString::number(QtToUi(proc->sharpeningLevel()), 'f', 1));
    }

    emit updateProps();
#else
    // Qt6 properties not implemented yet.
#endif
}

void CameraUSB::run()
{
    int acq_err = 0;
    int acq_cnt = 0;
    if (camera_err == 0 && suspend == true) {
        suspend = false;
        eTimeTotal = 0;
        while (suspend == false) {
            QLOG_DEBUG() << "CameraUSB::run> " << id << " : start new Acquisition";
            double eTime = Utils::GetTimeMicroseconds();
            acqstart->wakeAll();
            acq_err = acquireImage();
            QLOG_DEBUG() << "CameraUSB::run> " << id << " : done";
            acqend->wakeAll();
            if (acq_err == 1) {
                eTimeTotal += (Utils::GetTimeMicroseconds() - eTime);
            }
            acq_cnt++;
            if (acq_cnt == FREQUENCY_AVERAGE_COUNT) {
                eTimeTotal /= 1e6;
                eTimeTotal /= FREQUENCY_AVERAGE_COUNT;
                if (eTimeTotal > 0) {
                    frequency = 1.0 / eTimeTotal;
                } else {
                    frequency = 0;
                }
                QLOG_DEBUG() << "CameraUSB::run> Acquisition "
                             << "freq " << (int)frequency << " Hz";
                eTimeTotal = 0;
                acq_cnt = 0;
            }
            has_started = true;
            msleep(10);
        }
    }
}

int CameraUSB::connectCamera()
{
    if (!camera) {
        return -1;
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (!imageCapture) {
        imageCapture = new QCameraImageCapture(camera, this);
        imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
        QObject::connect(
            imageCapture,
            &QCameraImageCapture::imageCaptured,
            this,
            [this](int id, const QImage &image) {
                Q_UNUSED(id);
                processImage(image);
            },
            Qt::UniqueConnection);
    }

    if (viewfinder) {
        camera->setViewfinder(viewfinder);
    }

    camera->setCaptureMode(QCamera::CaptureStillImage);
#else
    if (!imageCapture) {
        imageCapture = new QImageCapture;
        imageCapture->setParent(camera);
        captureSession.setCamera(camera);
        captureSession.setImageCapture(imageCapture);
        QObject::connect(
            imageCapture,
            &QImageCapture::imageCaptured,
            this,
            [this](int id, const QImage &image) {
                Q_UNUSED(id);
                emit getImage(image);
            },
            Qt::UniqueConnection);
    }
#endif

    if (imageWidth > 0 && imageHeight > 0) {
        setImageSize(imageWidth, imageHeight);
    }
    camera->start();
    pixel_encoding = B8;
    getFeatures();
    return 0;
}

int CameraUSB::acquireImage()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (!camera || !imageCapture) {
        return 0;
    }

    if (!imageCapture->isReadyForCapture()) {
        return 0;
    }

    frameMutex.lock();
    frameAvailable = false;
    frameMutex.unlock();

    imageCapture->capture();
    frameMutex.lock();
    if (!frameAvailable) {
        frameReady.wait(&frameMutex, 1000);
    }
    const bool ready = frameAvailable;
    frameMutex.unlock();
    return ready ? 1 : 0;
#else
    if (!camera || !imageCapture) {
        return 0;
    }

    if (!imageCapture->isReadyForCapture()) {
        return 0;
    }

    // Capture an image
    imageCapture->capture();
    return 1;
#endif
}

void CameraUSB::cleanup_and_exit()
{
    stop();
    
    // Clean up any resources
    
}
#endif
