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

#define FREQUENCY_AVERAGE_COUNT 50

#include <math.h>
#include <QThread>
#include <QPixmap>
#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <QMutex>
#include <QWaitCondition>
#include <QTime>
#include "QsLog.h"

typedef enum {
  B8,
  B10,
  B12,
  B12P,
  B16,
  UNKNOWN
} pixel_encodings;

class Camera : public QThread
{
  Q_OBJECT
    
    public:
  
  virtual void stop() = 0;
  virtual int  findCamera() = 0;
  virtual void setCamera(void *_camera, int _id) = 0;
  virtual void getFeatures() = 0;
  virtual void getProps() = 0;
  virtual uchar *getSnapshot() = 0;
  virtual int *getSnapshot32() = 0;
 
  QMutex *mutex, *snapshotMutex, *acquireMutex;
  QWaitCondition *acqstart, *acqend;
  bool has_started;  
  bool suspend;
  unsigned int width;
  unsigned int height;
  unsigned int video_mode;
  unsigned int pixel_encoding;
  uchar *buffer,*snapshot;
  int *buffer32, *snapshot32;
  int BufSize;
  int snapShotMin, snapShotMax, min, max;
  int camera_err;                     /* Error flag */
  int num;                            /* Camera total number for camera manager*/
  int id;                             /* Camera position id in list for camera manager*/
  QVector<void *> cameralist;         /* Available camera list for camera manager*/
  QVector<QString> vendorlist;        /* Available vendor list for camera manager*/
  QVector<QString> modelist;          /* Available model list for camera manager*/
  QString vendor;                     /* Current vendor*/
  QString model;                      /* Current model*/
  bool modeCheckEnabled;              /* AUTO/MANUAL modeCheck enabled */
  QVector<QString> featureNameList;   /* Available feature list*/
  QVector<int> featureIdList;         /* Available feature id list*/
  QVector<double> featureValueList;   /* Available feature value list*/
  QVector<int> featureMinList;        /* Available feature min list*/
  QVector<int> featureMaxList;        /* Available feature max list*/
  QVector<bool> featureModeAutoList;  /* Available feature mode list*/
  QVector<bool> featureAutoCapableList; /* Available feature mode auto capable list*/
  QVector<double> featureAbsValueList; /* Available feature abs value list*/
  QVector<int> featureAbsCapableList; /* Available feature abs capable list*/
  
  QVector<QString> propList;          /* Available properties list*/

 signals:
  void  getImage(const QImage &image);
  void  showWarning(QString message);
  void  updateFeatures();
  void  updateProps();
  void  updateMin(int min);
  void  updateMax(int max);
  

  public slots:
  virtual void setImageSize(const int &_imageWidth, const int &_imageHeight) = 0;
  virtual void setFeature(int feature, double value) = 0;
  virtual void setMode(int feature, bool value) = 0;
  virtual void  vflipImage(int state) {
      vflip = state;
  }
  virtual void  hflipImage(int state) {
      hflip = state;
  }
  virtual uchar* reversebytes(uchar *buffer, int buffersize) {
     if (buffer == NULL) return NULL;
     uchar swap;
     uchar *lo = buffer;
     uchar *hi = buffer + buffersize - 1;
     while ( lo < hi ) {
          swap = *lo;
          *lo++ = *hi;
          *hi-- = swap;
     }
     return buffer;
  }
  virtual int* reversebytes(int *buffer, int buffersize) {
     if (buffer == NULL) return NULL;
     int swap;
     int *lo = buffer;
     int *hi = buffer + buffersize - 1;
     while ( lo < hi ) {
          swap = *lo;
          *lo++ = *hi;
          *hi-- = swap;
     }
     return buffer;
  }
  virtual uchar* fliphorizontal(uchar *buffer, int buffersize, int width) {
     if (buffer == NULL) return NULL;
     int ncol = 0;
     int nline = 0;
     while ( nline < buffersize / width - 1 ) {
       nline++;
       uchar swap;
       uchar *lo = buffer + nline*width;
       uchar *hi = buffer + (nline + 1)*width - 1;
       while ( lo < hi ) {
          swap = *lo;
          *lo++ = *hi;
          *hi-- = swap;
       }
     }
     return buffer;
  }
  virtual int* fliphorizontal(int *buffer, int buffersize, int width) {
     if (buffer == NULL) return NULL;
     int ncol = 0;
     int nline = 0;
     while ( nline < buffersize / width - 1 ) {
       nline++;
       int swap;
       int *lo = buffer + nline*width;
       int *hi = buffer + (nline + 1)*width - 1;
       while ( lo < hi ) {
          swap = *lo;
          *lo++ = *hi;
          *hi-- = swap;
       }
     }
     return buffer;
  }
 protected:
  virtual void run() = 0;
  virtual int  connectCamera() = 0;
  virtual int  acquireImage() = 0;
  virtual void cleanup_and_exit() = 0;
  int vflip, hflip;
  double eTimeTotal, frequency;

};


#endif // CAMERA_H
