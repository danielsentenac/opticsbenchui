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
#ifdef GIGECAMERA
#ifndef CAMERAGIGE_H
#define CAMERAGIGE_H
#include "Camera.h"
#undef signals     // Collides with GTK symbols
#include "arv.h"
#define signals protected

class CameraGiGE : public Camera
{
  Q_OBJECT
    
    public:
  CameraGiGE();
  ~CameraGiGE();
 
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

  QImage *image;
  int imageWidth;
  int imageHeight;
  // Aravis
  ArvCamera *camera;
  ArvDevice *device;
  ArvGc *genicam;
  ArvStream *stream;
  ArvBuffer *arvbuffer;
  static const int arv_option_width = -1;
  static const int arv_option_height = -1;
  static const int arv_option_horizontal_binning = -1;
  static const int arv_option_vertical_binning = -1;
  static const double arv_option_exposure_time_us = -1;
  static const int arv_option_gain = -1;
  gint x, y,gwidth,gheight;
  gint dx, dy, payload;
  const char *digital_gain;
  static const int gain_values [];
  int gain;
  double exposure;
  QVector<QVector<QString> > featureChoiceList;
};

#endif // CAMERAGIGE_H
#endif
