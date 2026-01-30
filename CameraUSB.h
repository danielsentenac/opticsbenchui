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
#ifndef CAMERAUSB_H
#define CAMERAUSB_H
#include <QCamera>
#include <QCameraInfo>
#include <QCameraImageCapture>
#include <QCameraViewfinderSettings>
#include <QCameraViewfinder>
#include "Camera.h"

class CameraUSB : public Camera
{
 Q_OBJECT
    
    public:
  CameraUSB();
  ~CameraUSB() override;
 
  void stop() override;
  int  findCamera() override;
  void setCamera(void *_camera, int _id) override;
  void getFeatures() override;
  uchar* getSnapshot() override;
  ushort *getSnapshot16() override;
  int* getSnapshot32() override;

 signals:
  void  getImage(const QImage &image);
  void  showWarning(QString message);
  void  updateFeatures();
  void  updateMin(int min);
  void  updateMax(int max);
  
  
  public slots:
  void setImageSize(const int &_imageWidth, const int &_imageHeight) override;
  void setFeature(int feature, double value) override;
  void setMode(int feature, bool value) override;
  void getProps() override;

 private:
  void run() override;
  int  connectCamera() override;
  int  acquireImage() override;
  void cleanup_and_exit() override;
  
  QCamera *camera = nullptr;
  QCameraImageCapture *imageCapture = nullptr;
  QList<QCameraInfo> cameras;
  QCameraViewfinder *viewfinder = nullptr;
  QCameraViewfinderSettings viewfinderSettings;
  
  int cameraIndex = 0;
  int imageWidth = 0;
  int imageHeight = 0;
};

#endif // CAMERAUSB_H
#endif
