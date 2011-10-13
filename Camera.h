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

#ifndef CAMERA_H
#define CAMERA_H

#include <QThread>
#include <QPixmap>
#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <QMutex>
#include <QWaitCondition>

#include <dc1394/dc1394.h>
#include "QsLog.h"

extern char *iidc_features[];
extern char *iidc_video_modes[];
extern int VIDEO_MODES_OFFSET;

class Camera : public QThread
{
  Q_OBJECT
    

    public:
  Camera(QObject* parent = 0);
  ~Camera();
  int  connectCamera();
  void stop();
  int  acquireImage();
  int  findCamera();
  void setCamera(dc1394camera_t* _camera, int _id);
  void getFeatures();
  uchar* getSnapshot();
  void cleanup_and_exit();

  QMutex *mutex, *snapshotMutex;
  QWaitCondition *acqstart, *acqend;
  bool has_started;
  
  bool suspend;
  unsigned int           width;
  unsigned int           height;
  int                    snapShotMin, snapShotMax,min, max;
  dc1394camera_t         **cameralist; /* Available camera list*/
  dc1394camera_t         *camera;      /* current camera*/
  dc1394featureset_t     features;
  dc1394video_mode_t     video_mode;
  int                    camera_err;
  int                    num;          /* camera total number*/
  int                     id;          /*camera position id in list*/

 signals:
  void  getImage(const QImage &image);
  void  showWarning(QString message);
  void  updateFeatures();
  void  updateMinMax(int min, int max);

  public slots:
  void setImageSize(const int &_imageWidth, const int &_imageHeight);
  void setFeature(int feature, int value);
  

 protected:
  virtual void run();

 private:
  
  dc1394_t               *d;
  dc1394camera_list_t *  list;
  dc1394error_t          err;
  dc1394framerates_t     framerates;
  dc1394video_modes_t    video_modes;
  dc1394framerate_t      framerate;
  dc1394color_coding_t   coding;
  dc1394video_frame_t    *frame;
  static const int frequency = 15; 
  QImage *image;
  uchar *buffer,*snapshot;
  int imageWidth;
  int imageHeight; 
  
};

#endif // CAMERA_H

