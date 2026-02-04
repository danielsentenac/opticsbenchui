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
 * @file CameraUSB.h
 * @brief Camera backend for USB.
 */
#ifdef USBCAMERA
#ifndef CAMERAUSB_H
#define CAMERAUSB_H
#include <QtGlobal>
#include <QCamera>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QCameraInfo>
#include <QCameraImageCapture>
#include <QCameraViewfinderSettings>
#include <QCameraViewfinder>
#else
#include <QCameraDevice>
#include <QCameraFormat>
#include <QImageCapture>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#endif
#include "Camera.h"

/// \ingroup cameras
/// USB camera implementation using Qt multimedia.
class CameraUSB : public Camera
{
 Q_OBJECT
    
    public:
  /// Constructor.
  CameraUSB();
  /// Destructor.
  ~CameraUSB() override;
 
  /// Stop acquisition and release resources.
  void stop() override;
  /// Discover available cameras.
  int  findCamera() override;
  /// Attach a camera handle and id.
  /// \param _camera Backend camera pointer.
  /// \param _id Camera index.
  void setCamera(void *_camera, int _id) override;
  /// Query and cache camera features.
  void getFeatures() override;
  /// Acquire a single 8-bit image.
  uchar* getSnapshot() override;
  /// Acquire a single 16-bit image.
  ushort *getSnapshot16() override;
  /// Acquire a single 32-bit image.
  int* getSnapshot32() override;

 signals:
  /// Emit a new image frame.
  /// \param image The latest frame.
  void  getImage(const QImage &image);
  /// Emit warnings to the UI.
  /// \param message Warning message.
  void  showWarning(QString message);
  /// Notify UI to refresh feature values.
  void  updateFeatures();
  /// Update the min display value.
  /// \param min Minimum pixel value.
  void  updateMin(int min);
  /// Update the max display value.
  /// \param max Maximum pixel value.
  void  updateMax(int max);
  
  
  public slots:
  /// Set the image size for acquisition.
  /// \param _imageWidth Width in pixels.
  /// \param _imageHeight Height in pixels.
  void setImageSize(const int &_imageWidth, const int &_imageHeight) override;
  /// Set a numeric feature value.
  /// \param feature Feature identifier.
  /// \param value Feature value.
  void setFeature(int feature, double value) override;
  /// Enable or disable a feature mode.
  /// \param feature Feature identifier.
  /// \param value True for auto/enable, false for manual/disable.
  void setMode(int feature, bool value) override;
  /// Query the camera for its properties.
  void getProps() override;

 private:
  void run() override;
  int  connectCamera() override;
  int  acquireImage() override;
  void cleanup_and_exit() override;
  
 QCamera *camera = nullptr;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QCameraImageCapture *imageCapture = nullptr;
  QList<QCameraInfo> cameras;
  QCameraViewfinder *viewfinder = nullptr;
  QCameraViewfinderSettings viewfinderSettings;
#else
  QImageCapture *imageCapture = nullptr;
  QList<QCameraDevice> cameras;
  QMediaCaptureSession captureSession;
#endif
  
  int cameraIndex = 0;
  int imageWidth = 0;
  int imageHeight = 0;
};

#endif // CAMERAUSB_H
#endif
