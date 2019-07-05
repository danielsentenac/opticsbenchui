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
#ifdef RASPICAMERA
#ifndef CAMERARASPI_H
#define CAMERARASPI_H
#include "raspicam.h"
#include "Camera.h"

using namespace raspicam;

class CameraRaspi : public Camera
{
  Q_OBJECT
    
    public:
  CameraRaspi();
  ~CameraRaspi();
 
  void stop();
  int  findCamera();
  void setCamera(void *_camera, int _id);
  void getFeatures();
  uchar* getSnapshot();
  int* getSnapshot32();
  
 signals:
  void  getImage(const QImage &image);
  void  showWarning(QString message);
  void  updateFeatures();
  void  updateProps();
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
  int imageWidth;
  int imageHeight;
  // Raspi Camera
  RaspiCam *camera;
  double brightness, sharpness, iso, saturation, contrast, frate;
  double exposure_compensation, shutterspeed;
  unsigned char *data;
  int encoding_num, awb_num, exposure_num, aoi_num, acq_num;
  int videostabilization_num, resolution_num;
  double eTimeTotal, frequency;
};

#endif // CAMERARASPI_H
#endif
