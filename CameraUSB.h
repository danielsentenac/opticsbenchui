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
  ~CameraUSB();
 
  void stop();
  int  findCamera();
  void setCamera(void *_camera, int _id);
  void getFeatures();
  uchar* getSnapshot();
  ushort *getSnapshot16();
  int* getSnapshot32();

 signals:
  void  getImage(const QImage &image);
  void  showWarning(QString message);
  void  updateFeatures();
  void  updateMin(int min);
  void  updateMax(int max);
  
  
  public slots:
  void setImageSize(const int &_imageWidth, const int &_imageHeight);
  void setFeature(int feature, double value);
  void setMode(int feature, bool value);
  void getProps();

 private:
  void run();
  int  connectCamera();
  int  acquireImage();
  void cleanup_and_exit();
  
  QCamera *camera;
  QCameraImageCapture *imageCapture;
  QList<QCameraInfo> cameras;
  QCameraViewfinder *viewfinder;
  QCameraViewfinderSettings viewfinderSettings;
  
  int cameraIndex;
  int imageWidth;
  int imageHeight;
};

#endif // CAMERAUSB_H
#endif
