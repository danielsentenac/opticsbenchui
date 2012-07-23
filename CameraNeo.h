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
#ifdef NEOCAMERA
#ifndef CAMERANEO_H
#define CAMERANEO_H
#include "Camera.h"
#include "atcore.h"

class CameraNeo : public Camera
{
  Q_OBJECT
    
    public:
  CameraNeo();
  ~CameraNeo();
 
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
  const char * sdkErrorString(int _i_errorCode);
  bool errorOk(int _i_err, const char * _sz_caller);

  QImage *image;
  int imageWidth;
  int imageHeight;
  // Neo Andor
  int i_err;
  int i_available;
  AT_H *camera;
  double exposure, frate, temp;
  int rrate;
  AT_64 BufferSize, aoi_height, aoi_width, aoi_left, aoi_top; 
  ushort** AlignedBuffers;
  int FrameNumber, encoding_num, gain_num, trigger_num;
  double eTimeTotal, frequency;
};

#endif // CAMERANEO_H
#endif
