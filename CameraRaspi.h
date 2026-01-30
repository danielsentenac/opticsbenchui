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
  ~CameraRaspi() override;
 
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
  void  updateProps();
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
  int imageWidth = 0;
  int imageHeight = 0;
  // Raspi Camera
  RaspiCam *camera = nullptr;
  double brightness = 0.0;
  double sharpness = 0.0;
  double iso = 0.0;
  double saturation = 0.0;
  double contrast = 0.0;
  double frate = 0.0;
  double exposure_compensation = 0.0;
  double shutterspeed = 0.0;
  unsigned char *data = nullptr;
  int encoding_num = 0;
  int awb_num = 0;
  int exposure_num = 0;
  int aoi_num = 0;
  int acq_num = 0;
  int videostabilization_num = 0;
  int resolution_num = 0;
  double eTimeTotal = 0.0;
  double frequency = 0.0;
};

#endif // CAMERARASPI_H
#endif
