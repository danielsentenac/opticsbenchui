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
  
  Camera() {
  
   for (int i = 0; i < 255; i++) 
     gray.append(qRgb(i, i, i));

   hot <<
    qRgb( 8,   0,   0)<<
    qRgb( 9,   0,   0)<<
    qRgb( 10,   0,   0)<<
    qRgb( 11,   0,   0)<<
    qRgb( 19,   0,   0)<<
    qRgb( 20,   0,   0)<<
    qRgb( 21,   0,   0)<<
    qRgb( 22,   0,   0)<<
    qRgb( 29,   0,   0)<<
    qRgb( 30,   0,   0)<<
    qRgb( 31,   0,   0)<<
    qRgb( 32,   0,   0)<<
    qRgb( 40,   0,   0)<<
    qRgb( 41,   0,   0)<<
    qRgb( 42,   0,   0)<<
    qRgb( 43,   0,   0)<<
    qRgb( 51,   0,   0)<<
    qRgb( 52,   0,   0)<<
    qRgb( 53,   0,   0)<<
    qRgb( 54,   0,   0)<<
    qRgb( 61,   0,   0)<<
    qRgb( 62,   0,   0)<<
    qRgb( 63,   0,   0)<<
    qRgb( 64,   0,   0)<<
    qRgb( 72,   0,   0)<<
    qRgb( 73,   0,   0)<<
    qRgb( 74,   0,   0)<<
    qRgb( 75,   0,   0)<<
    qRgb( 82,   0,   0)<<
    qRgb( 83,   0,   0)<<
    qRgb( 84,   0,   0)<<
    qRgb( 85,   0,   0)<<
    qRgb( 93,   0,   0)<<
    qRgb( 94,   0,   0)<<
    qRgb( 95,   0,   0)<<
    qRgb( 96,   0,   0)<<
    qRgb(104,   0,   0)<<
    qRgb(105,   0,   0)<<
    qRgb(106,   0,   0)<<
    qRgb(107,   0,   0)<<
    qRgb(114,   0,   0)<<
    qRgb(115,   0,   0)<<
    qRgb(116,   0,   0)<<
    qRgb(117,   0,   0)<<
    qRgb(125,   0,   0)<<
    qRgb(126,   0,   0)<<
    qRgb(127,   0,   0)<<
    qRgb(128,   0,   0)<<
    qRgb(136,   0,   0)<<
    qRgb(137,   0,   0)<<
    qRgb(138,   0,   0)<<
    qRgb(139,   0,   0)<<
    qRgb(146,   0,   0)<<
    qRgb(147,   0,   0)<<
    qRgb(148,   0,   0)<<
    qRgb(149,   0,   0)<<
    qRgb(156,   0,   0)<<
    qRgb(157,   0,   0)<<
    qRgb(158,   0,   0)<<
    qRgb(159,   0,   0)<<
    qRgb(168,   0,   0)<<
    qRgb(169,   0,   0)<<
    qRgb(170,   0,   0)<<
    qRgb(171,   0,   0)<<
    qRgb(178,   0,   0)<<
    qRgb(179,   0,   0)<<
    qRgb(180,   0,   0)<<
    qRgb(181,   0,   0)<<
    qRgb(189,   0,   0)<<
    qRgb(190,   0,   0)<<
    qRgb(191,   0,   0)<<
    qRgb(192,   0,   0)<<
    qRgb(198,   0,   0)<<
    qRgb(199,   0,   0)<<
    qRgb(201,   0,   0)<<
    qRgb(202,   0,   0)<<
    qRgb(210,   0,   0)<<
    qRgb(211,   0,   0)<<
    qRgb(212,   0,   0)<<
    qRgb(213,   0,   0)<<
    qRgb(221,   0,   0)<<
    qRgb(222,   0,   0)<<
    qRgb(223,   0,   0)<<
    qRgb(224,   0,   0)<<
    qRgb(231,   0,   0)<<
    qRgb(232,   0,   0)<<
    qRgb(233,   0,   0)<<
    qRgb(234,   0,   0)<<
    qRgb(242,   0,   0)<<
    qRgb(243,   0,   0)<<
    qRgb(244,   0,   0)<<
    qRgb(245,   0,   0)<<
    qRgb(252,   0,   0)<<
    qRgb(253,   0,   0)<<
    qRgb(254,   0,   0)<<
    qRgb(255,   0,   0)<<
    qRgb(255,  8,   0)<<
    qRgb(255,  9,   0)<<
    qRgb(255,  10,   0)<<
    qRgb(255,  11,   0)<<
    qRgb(255,  19,   0)<<
    qRgb(255,  20,   0)<<
    qRgb(255,  21,   0)<<
    qRgb(255,  22,   0)<<
    qRgb(255,  29,   0)<<
    qRgb(255,  30,   0)<<
    qRgb(255,  31,   0)<<
    qRgb(255,  32,   0)<<
    qRgb(255,  40,   0)<<
    qRgb(255,  41,   0)<<
    qRgb(255,  42,   0)<<
    qRgb(255,  43,   0)<<
    qRgb(255,  51,   0)<<
    qRgb(255,  52,   0)<<
    qRgb(255,  53,   0)<<
    qRgb(255,  54,   0)<<
    qRgb(255,  61,   0)<<
    qRgb(255,  62,   0)<<
    qRgb(255,  63,   0)<<
    qRgb(255,  64,   0)<<
    qRgb(255,  72,   0)<<
    qRgb(255,  73,   0)<<
    qRgb(255,  74,   0)<<
    qRgb(255,  75,   0)<<
    qRgb(255,  82,   0)<<
    qRgb(255,  83,   0)<<
    qRgb(255,  84,   0)<<
    qRgb(255,  85,   0)<<
    qRgb(255,  93,   0)<<
    qRgb(255,  94,   0)<<
    qRgb(255,  95,   0)<<
    qRgb(255,  96,   0)<<
    qRgb(255, 104,   0)<<
    qRgb(255, 105,   0)<<
    qRgb(255, 106,   0)<<
    qRgb(255, 107,   0)<<
    qRgb(255, 114,   0)<<
    qRgb(255, 115,   0)<<
    qRgb(255, 116,   0)<<
    qRgb(255, 117,   0)<<
    qRgb(255, 125,   0)<<
    qRgb(255, 126,   0)<<
    qRgb(255, 127,   0)<<
    qRgb(255, 128,   0)<<
    qRgb(255, 136,   0)<<
    qRgb(255, 137,   0)<<
    qRgb(255, 138,   0)<<
    qRgb(255, 139,   0)<<
    qRgb(255, 146,   0)<<
    qRgb(255, 147,   0)<<
    qRgb(255, 148,   0)<<
    qRgb(255, 149,   0)<<
    qRgb(255, 157,   0)<<
    qRgb(255, 158,   0)<<
    qRgb(255, 159,   0)<<
    qRgb(255, 160,   0)<<
    qRgb(255, 168,   0)<<
    qRgb(255, 169,   0)<<
    qRgb(255, 170,   0)<<
    qRgb(255, 171,   0)<<
    qRgb(255, 178,   0)<<
    qRgb(255, 179,   0)<<
    qRgb(255, 180,   0)<<
    qRgb(255, 181,   0)<<
    qRgb(255, 189,   0)<<
    qRgb(255, 190,   0)<<
    qRgb(255, 191,   0)<<
    qRgb(255, 192,   0)<<
    qRgb(255, 199,   0)<<
    qRgb(255, 200,   0)<<
    qRgb(255, 201,   0)<<
    qRgb(255, 202,   0)<<
    qRgb(255, 210,   0)<<
    qRgb(255, 211,   0)<<
    qRgb(255, 212,   0)<<
    qRgb(255, 213,   0)<<
    qRgb(255, 221,   0)<<
    qRgb(255, 222,   0)<<
    qRgb(255, 223,   0)<<
    qRgb(255, 224,   0)<<
    qRgb(255, 231,   0)<<
    qRgb(255, 232,   0)<<
    qRgb(255, 233,   0)<<
    qRgb(255, 234,   0)<<
    qRgb(255, 242,   0)<<
    qRgb(255, 243,   0)<<
    qRgb(255, 244,   0)<<
    qRgb(255, 245,   0)<<
    qRgb(255, 252,   0)<<
    qRgb(255, 253,   0)<<
    qRgb(255, 254,   0)<<
    qRgb(255, 255,   0)<<
    qRgb(255, 255,  13)<<
    qRgb(255, 255,  14)<<
    qRgb(255, 255,  15)<<
    qRgb(255, 255,  16)<<
    qRgb(255, 255,  29)<<
    qRgb(255, 255,  30)<<
    qRgb(255, 255,  31)<<
    qRgb(255, 255,  32)<<
    qRgb(255, 255,  45)<<
    qRgb(255, 255,  46)<<
    qRgb(255, 255,  47)<<
    qRgb(255, 255,  48)<<
    qRgb(255, 255,  61)<<
    qRgb(255, 255,  62)<<
    qRgb(255, 255,  63)<<
    qRgb(255, 255,  64)<<
    qRgb(255, 255,  77)<<
    qRgb(255, 255,  78)<<
    qRgb(255, 255,  79)<<
    qRgb(255, 255,  80)<<
    qRgb(255, 255,  93)<<
    qRgb(255, 255,  94)<<
    qRgb(255, 255,  95)<<
    qRgb(255, 255,  96)<<
    qRgb(255, 255, 109)<<
    qRgb(255, 255, 110)<<
    qRgb(255, 255, 111)<<
    qRgb(255, 255, 112)<<
    qRgb(255, 255, 125)<<
    qRgb(255, 255, 126)<<
    qRgb(255, 255, 127)<<
    qRgb(255, 255, 128)<<
    qRgb(255, 255, 141)<<
    qRgb(255, 255, 142)<<
    qRgb(255, 255, 143)<<
    qRgb(255, 255, 144)<<
    qRgb(255, 255, 157)<<
    qRgb(255, 255, 158)<<
    qRgb(255, 255, 159)<<
    qRgb(255, 255, 160)<<
    qRgb(255, 255, 173)<<
    qRgb(255, 255, 174)<<
    qRgb(255, 255, 175)<<
    qRgb(255, 255, 176)<<
    qRgb(255, 255, 189)<<
    qRgb(255, 255, 190)<<
    qRgb(255, 255, 191)<<
    qRgb(255, 255, 192)<<
    qRgb(255, 255, 205)<<
    qRgb(255, 255, 206)<<
    qRgb(255, 255, 207)<<
    qRgb(255, 255, 208)<<
    qRgb(255, 255, 221)<<
    qRgb(255, 255, 222)<<
    qRgb(255, 255, 223)<<
    qRgb(255, 255, 224)<<
    qRgb(255, 255, 237)<<
    qRgb(255, 255, 238)<<
    qRgb(255, 255, 239)<<
    qRgb(255, 255, 240)<<
    qRgb(255, 255, 252)<<
    qRgb(255, 255, 253)<<
    qRgb(255, 255, 254)<<
    qRgb(255, 255, 255);

    table = &gray;
  };
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
  virtual void setColorTable(int id) {
    if ( id == -3 ) {
     QLOG_INFO() << "Camera::setColorTable> changing color to hot";
     table = &hot;
    }
    if (id == -2) {
      QLOG_INFO() << "Camera::setColorTable> changing color to gray";
     table = &gray;
    }
    acquireMutex->lock();
    delete image;
    image = new QImage(buffer,width,height,width,QImage::Format_Indexed8);
    image->setColorTable(*table);
    acquireMutex->unlock(); 
  }
 protected:
  virtual void run() = 0;
  virtual int  connectCamera() = 0;
  virtual int  acquireImage() = 0;
  virtual void cleanup_and_exit() = 0;
  int vflip, hflip;
  double eTimeTotal, frequency;
  QVector<QRgb> hot;
  QVector<QRgb> gray;
  QVector<QRgb> *table;
  QImage *image;
};


#endif // CAMERA_H
