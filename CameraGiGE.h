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
  ~CameraGiGE() override;
 
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

  int imageWidth = 0;
  int imageHeight = 0;
  // Aravis
  ArvCamera *camera = nullptr;
  ArvDevice *device = nullptr;
  ArvGc *genicam = nullptr;
  ArvStream *stream = nullptr;
  ArvBuffer *arvbuffer = nullptr;
  ArvBuffer *arvbuffer_last = nullptr;
  static const int arv_option_width = -1;
  static const int arv_option_height = -1;
  static const int arv_option_horizontal_binning = -1;
  static const int arv_option_vertical_binning = -1;
  static const double arv_option_exposure_time_us = -1;
  static const int arv_option_gain = -1;
  gint x = 0;
  gint y = 0;
  gint gwidth = 0;
  gint gheight = 0;
  gint dx = 0;
  gint dy = 0;
  gint payload = 0;
  const char *digital_gain = nullptr;
  static const int gain_values [];
  int gain = 0;
  double exposure = 0.0;
  QVector<QVector<QString> > featureChoiceList;
};

#endif // CAMERAGIGE_H
#endif
