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
#ifdef IEEE1394CAMERA
#ifndef CAMERAIEEE1394_H
#define CAMERAIEEE1394_H
#include <dc1394/dc1394.h>
#include "Camera.h"

class CameraIEEE1394 : public Camera
{
  Q_OBJECT
    
    public:
  CameraIEEE1394();
  ~CameraIEEE1394();
 
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
  dc1394camera_t         *camera;
  dc1394_t               *d;
  dc1394camera_list_t *  list;
  dc1394error_t          err;
  dc1394framerates_t     framerates;
  dc1394video_modes_t    video_modes;
  dc1394framerate_t      framerate;
  dc1394color_coding_t   encoding_num;
  dc1394video_frame_t    *frame;
  dc1394featureset_t     features;
  dc1394video_mode_t     video_mode;
  int                    video_mode_feature, color_coding_feature;
  int imageWidth;
  int imageHeight;
};

#endif // CAMERAIEEE1394_H
#endif
