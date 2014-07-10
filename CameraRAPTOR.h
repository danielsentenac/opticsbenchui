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
#ifdef RAPTORCAMERA
#ifndef CAMERARAPTOR_H
#define CAMERARAPTOR_H
#include <xcliball.h>
#include "Camera.h"

class CameraRAPTOR : public Camera
{
  Q_OBJECT
    
    public:
  CameraRAPTOR();
  ~CameraRAPTOR();
 
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
  int  readFeature(char* sendreg, int size_sreg, char* readreg, int size_rreg, uchar* data, int size_data);
  int  writeFeature(char* sendreg, int size_sreg);
  double getPCBtemperature();
  double getCCDtemperature();
  QString getBinningFactor();
  void setBinningFactor(int value);
  void updateFrameGrabberAOI(int v0, int v1, int v2, int v3);
  double getEMgain();
  void   setEMgain(int g);
  void   setAOI(int left, int width, int top, int height);
  double getExposure();
  void   setExposure(double e);
  QString getBloomState();
  void   setBloomState(int value);
  struct xclibs *xc;
  struct pxvidstate   *vidstate;
  pxbuffer_t framenum;
  ushort *image16;
  QImage *image;
  int imageWidth;
  int imageHeight;
  double frate, frameTotal;
  bool binning_changed, aoi_changed;
};

#endif // CAMERARAPTOR_H
#endif
