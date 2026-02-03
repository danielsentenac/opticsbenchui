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
#ifdef USBCAMERA

#include <QCoreApplication>
#include <QCamera>
#include <QImage>
#include <QDebug>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QCameraImageCapture>
#include <QVideoFrame>
#endif
#include "CameraUSB.h"
#include "Utils.h"

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
}

CameraUSB::~CameraUSB()
{
    stop();
}

void CameraUSB::stop()
{
    if (camera) {
        camera->stop();
        delete camera;
        camera = nullptr;
    }
    
    if (imageCapture) {
        delete imageCapture;
        imageCapture = nullptr;
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (viewfinder) {
        delete viewfinder;
        viewfinder = nullptr;
    }
#endif
}

int CameraUSB::findCamera()
{
    if (cameras.isEmpty()) {
        Utils::EmitWarning(this, __FUNCTION__, "No cameras found.");
        return -1;
    }
    
    // Choose the first available camera
    cameraIndex = 0;
    
    return cameraIndex;
}

void CameraUSB::setCamera(void *_camera, int _id)
{
    stop();
    
    if (_id < 0 || _id >= cameras.size()) {
        Utils::EmitWarning(this, __FUNCTION__, "Invalid camera ID.");
        return;
    }
    
    camera = (QCamera*)_camera;
    cameraIndex = _id;
    
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    imageCapture = new QCameraImageCapture(camera);
#else
    imageCapture = new QImageCapture;
    imageCapture->setParent(camera);
    captureSession.setCamera(camera);
    captureSession.setImageCapture(imageCapture);
#endif
}

void CameraUSB::getFeatures()
{
    // Not implemented
}

uchar* CameraUSB::getSnapshot()
{
    // Not implemented
    return nullptr;
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
    
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Set the image capture's capture destination format to QImage
    imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
    
    // Set the image capture's capture resolution
    QSize resolution(imageWidth, imageHeight);
    viewfinderSettings.setResolution(imageWidth, imageHeight);
    viewfinderSettings.setMaximumFrameRate(5.0);
    viewfinderSettings.setPixelFormat(QVideoFrame::Format_YUYV);
    camera->setViewfinderSettings(viewfinderSettings);
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
    // Not implemented
}

void CameraUSB::setMode(int feature, bool value)
{
    // Not implemented
}

void CameraUSB::getProps()
{
    // Not implemented
}

void CameraUSB::run()
{
    
    // Find the camera
    int cameraId = findCamera();
    if (cameraId == -1) {
        return;
    }
    
    // Create a QCamera object for the chosen camera
    QCamera *camera = new QCamera(cameras[cameraId]);
    
    // Set the camera
    setCamera(camera, cameraId);
    
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    camera->setViewfinder(viewfinder);
#endif
    // Start the camera
    camera->start();
    
    
    // Cleanup and exit
    cleanup_and_exit();
}

int CameraUSB::connectCamera()
{
    // Not implemented
    return -1;
}

int CameraUSB::acquireImage()
{
   
        
     if (!camera || !imageCapture)
    {
        Utils::EmitWarning(this, __FUNCTION__, "Camera or image capture is not initialized.");
        return -1;
    }

    // Capture an image
    imageCapture->capture();

    // Connect a slot to handle the captured image
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QObject::connect(imageCapture, &QCameraImageCapture::imageCaptured, [this](int id, const QImage& image)
    {
        // Emit the captured image
        emit getImage(image);

        // Disconnect the slot
        QObject::disconnect(imageCapture, &QCameraImageCapture::imageCaptured, nullptr, nullptr);
    });
#else
    QObject::connect(imageCapture, &QImageCapture::imageCaptured, [this](int id, const QImage& image)
    {
        // Emit the captured image
        emit getImage(image);

        // Disconnect the slot
        QObject::disconnect(imageCapture, &QImageCapture::imageCaptured, nullptr, nullptr);
    });
#endif
    
    
    return 0;
}

void CameraUSB::cleanup_and_exit()
{
    stop();
    
    // Clean up any resources
    
}
#endif
