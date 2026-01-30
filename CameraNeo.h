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
  ~CameraNeo() override;
 
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
  const char * sdkErrorString(int _i_errorCode);
  bool errorOk(int _i_err, const char * _sz_caller);

  int imageWidth = 0;
  int imageHeight = 0;
  // Neo Andor
  int i_err = 0;
  int i_available = 0;
  AT_H *camera = nullptr;
  double exposure = 0.0;
  double frate = 0.0;
  double temp = 0.0;
  int rrate = 0;
  AT_64 BufferSize = 0;
  AT_64 aoi_height = 0;
  AT_64 aoi_width = 0;
  AT_64 aoi_left = 0;
  AT_64 aoi_top = 0;
  ushort** AlignedBuffers = nullptr;
  int FrameNumber = 0;
  int encoding_num = 0;
  int gain_num = 0;
  int trigger_num = 0;
  int acq_num = 0;
  double eTimeTotal = 0.0;
  double frequency = 0.0;
};

#endif // CAMERANEO_H
#endif
