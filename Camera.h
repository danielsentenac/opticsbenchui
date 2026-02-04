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

/**
 * @file Camera.h
 * @brief Camera backend for .
 */

#ifndef CAMERA_H
#define CAMERA_H

#define _GNU_SOURCE

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

/// Pixel encoding types reported by camera backends.
typedef enum {
  B8,
  B10,
  B12,
  B12P,
  B14,
  B16,
  UNKNOWN
} pixel_encodings;

/// \ingroup cameras
/// Base camera thread with shared image handling utilities.
///
/// Concrete camera implementations derive from this class to provide
/// device-specific acquisition and configuration.
class Camera : public QThread
{
  Q_OBJECT
    
    public:
  
  Camera()
    : QThread()
  {
   for (int i = 0; i < 255; i++) {
     gray.append(qRgb(i, i, i));
   }

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
  }
  ~Camera() override = default;
  /// Stop acquisition and release resources.
  virtual void stop() = 0;
  /// Discover available cameras.
  virtual int  findCamera() = 0;
  /// Attach a backend camera handle.
  /// \param _camera Backend-specific camera pointer.
  /// \param _id Camera index within the manager.
  virtual void setCamera(void *_camera, int _id) = 0;
  /// Query and cache camera features.
  virtual void getFeatures() = 0;
  /// Query and cache camera properties.
  virtual void getProps() = 0;
  /// Acquire a single 8-bit snapshot.
  virtual uchar *getSnapshot() = 0;
  /// Acquire a single 16-bit snapshot.
  virtual ushort *getSnapshot16() = 0;
  /// Acquire a single 32-bit snapshot.
  virtual int *getSnapshot32() = 0;
 
  /// General mutex for camera operations.
  QMutex *mutex = nullptr;
  /// Mutex for snapshot operations.
  QMutex *snapshotMutex = nullptr;
  /// Mutex for acquisition operations.
  QMutex *acquireMutex = nullptr;
  /// Wait condition for acquisition start.
  QWaitCondition *acqstart = nullptr;
  /// Wait condition for acquisition end.
  QWaitCondition *acqend = nullptr;
  /// True if acquisition has started.
  bool has_started = false;
  /// True if acquisition is suspended.
  bool suspend = false;
  /// True if acquisition optimization is enabled.
  bool optimizeAcquisition = false;
  /// Current image width.
  unsigned int width = 0;
  /// Current image height.
  unsigned int height = 0;
  /// Video mode identifier.
  unsigned int video_mode = 0;
  /// Pixel encoding identifier.
  unsigned int pixel_encoding = 0;
  /// 8-bit acquisition buffer.
  uchar *buffer = nullptr;
  /// 8-bit snapshot buffer.
  uchar *snapshot = nullptr;
  /// 16-bit acquisition buffer.
  ushort *buffer16 = nullptr;
  /// 16-bit snapshot buffer.
  ushort *snapshot16 = nullptr;
  /// 32-bit acquisition buffer.
  int *buffer32 = nullptr;
  /// 32-bit snapshot buffer.
  int *snapshot32 = nullptr;
  /// Buffer size (bytes or elements, backend-specific).
  int BufSize = 0;
  /// Snapshot minimum.
  int snapShotMin = 0;
  /// Snapshot maximum.
  int snapShotMax = 0;
  /// Snapshot average.
  int snapShotAvg = 0;
  /// Current minimum.
  int min = 0;
  /// Current maximum.
  int max = 0;
  /// Current average.
  int avg = 0;
  /// Error flag.
  int camera_err = 0;
  /// Camera total number for camera manager.
  int num = 0;
  /// Camera position id in list for camera manager.
  int id = 0;
  /// Available camera list for camera manager.
  QVector<void *> cameralist;
  /// Available vendor list for camera manager.
  QVector<QString> vendorlist;
  /// Available model list for camera manager.
  QVector<QString> modelist;
  /// Current vendor.
  QString vendor;
  /// Current model.
  QString model;
  /// AUTO/MANUAL mode check enabled.
  bool modeCheckEnabled = false;
  /// Available feature list.
  QVector<QString> featureNameList;
  /// Available feature id list.
  QVector<int> featureIdList;
  /// Available feature value list.
  QVector<double> featureValueList;
  /// Available feature min list.
  QVector<double> featureMinList;
  /// Available feature max list.
  QVector<double> featureMaxList;
  /// Available feature mode list.
  QVector<bool> featureModeAutoList;
  /// Available feature mode auto capable list.
  QVector<bool> featureAutoCapableList;
  /// Available feature abs value list.
  QVector<double> featureAbsValueList;
  /// Available feature abs capable list.
  QVector<int> featureAbsCapableList;
  
  /// Available properties list.
  QVector<QString> propList;

 signals:
  /// Emit a new image frame.
  /// \param image The latest frame.
  void  getImage(const QImage &image);
  /// Emit warnings to the UI.
  /// \param message Warning message.
  void  showWarning(QString message);
  /// Notify UI to refresh feature values.
  void  updateFeatures();
  /// Notify UI to refresh property values.
  void  updateProps();
  /// Update the min display value.
  /// \param min Minimum pixel value.
  void  updateMin(int min);
  /// Update the max display value.
  /// \param max Maximum pixel value.
  void  updateMax(int max);
  /// Update the average display value.
  /// \param avg Average pixel value.
  void  updateAvg(int avg);

  public slots:
  /// Set the desired image size for acquisition.
  /// \param _imageWidth Width in pixels.
  /// \param _imageHeight Height in pixels.
  virtual void setImageSize(const int &_imageWidth, const int &_imageHeight) = 0;
  /// Set a numeric camera feature.
  /// \param feature Feature identifier.
  /// \param value Feature value.
  virtual void setFeature(int feature, double value) = 0;
  /// Enable or disable a feature mode.
  /// \param feature Feature identifier.
  /// \param value True for auto/enable, false for manual/disable.
  virtual void setMode(int feature, bool value) = 0;
  /// Enable or disable vertical flip.
  /// \param state Nonzero to enable.
  virtual void  vflipImage(int state) {
      vflip = state;
  }
  /// Enable or disable horizontal flip.
  /// \param state Nonzero to enable.
  virtual void  hflipImage(int state) {
      hflip = state;
  }
  /// Reverse byte order in an 8-bit buffer.
  /// \param buffer Buffer to modify in place.
  /// \param buffersize Number of bytes in the buffer.
  /// \return The modified buffer or nullptr.
  virtual uchar* reversebytes(uchar *buffer, int buffersize) {
     if (buffer == nullptr) return nullptr;
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
  /// Reverse byte order in a 16-bit buffer.
  /// \param buffer Buffer to modify in place.
  /// \param buffersize Number of elements in the buffer.
  /// \return The modified buffer or nullptr.
  virtual ushort* reversebytes(ushort *buffer, int buffersize) {
     if (buffer == nullptr) return nullptr;
     ushort swap;
     ushort *lo = buffer;
     ushort *hi = buffer + buffersize - 1;
     while ( lo < hi ) {
          swap = *lo;
          *lo++ = *hi;
          *hi-- = swap;
     }
     return buffer;
  }
  /// Reverse byte order in a 32-bit buffer.
  /// \param buffer Buffer to modify in place.
  /// \param buffersize Number of elements in the buffer.
  /// \return The modified buffer or nullptr.
  virtual int* reversebytes(int *buffer, int buffersize) {
     if (buffer == nullptr) return nullptr;
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
  /// Flip an 8-bit image horizontally in place.
  /// \param buffer Buffer to modify.
  /// \param buffersize Total number of pixels.
  /// \param width Image width in pixels.
  /// \return The modified buffer or nullptr.
  virtual uchar* fliphorizontal(uchar *buffer, int buffersize, int width) {
     if (buffer == nullptr) return nullptr;
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
  /// Flip a 16-bit image horizontally in place.
  /// \param buffer Buffer to modify.
  /// \param buffersize Total number of pixels.
  /// \param width Image width in pixels.
  /// \return The modified buffer or nullptr.
  virtual ushort* fliphorizontal(ushort *buffer, int buffersize, int width) {
     if (buffer == nullptr) return nullptr;
     int nline = 0;
     while ( nline < buffersize / width - 1 ) {
       nline++;
       ushort swap;
       ushort *lo = buffer + nline*width;
       ushort *hi = buffer + (nline + 1)*width - 1;
       while ( lo < hi ) {
          swap = *lo;
          *lo++ = *hi;
          *hi-- = swap;
       }
     }
     return buffer;
  }
  /// Flip a 32-bit image horizontally in place.
  /// \param buffer Buffer to modify.
  /// \param buffersize Total number of pixels.
  /// \param width Image width in pixels.
  /// \return The modified buffer or nullptr.
  virtual int* fliphorizontal(int *buffer, int buffersize, int width) {
     if (buffer == nullptr) return nullptr;
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
  /// Select the color table for 8-bit display.
  /// \param id Color table identifier (-3 = hot, -2 = gray).
  virtual void setColorTable(int id) {
    if ( id == -3 ) {
     QLOG_INFO() << "Camera::setColorTable> changing color to hot";
     table = &hot;
    }
    if (id == -2) {
      QLOG_INFO() << "Camera::setColorTable> changing color to gray";
     table = &gray;
    }
    while (acquireMutex->tryLock() == false) {
         usleep(100);
    }
    delete image;
    image = new QImage(buffer,width,height,width,QImage::Format_Indexed8);
    image->setColorTable(*table);
    acquireMutex->unlock(); 
  }
 protected:
  /// Thread entry point for acquisition.
  virtual void run() = 0;
  /// Connect to the camera backend.
  virtual int  connectCamera() = 0;
  /// Acquire an image from the camera backend.
  virtual int  acquireImage() = 0;
  /// Cleanup backend resources.
  virtual void cleanup_and_exit() = 0;
  /// Vertical flip flag.
  int vflip = 0;
  /// Horizontal flip flag.
  int hflip = 0;
  /// Total exposure time.
  double eTimeTotal = 0.0;
  /// Acquisition frequency.
  double frequency = 0.0;
  /// Hot color table.
  QVector<QRgb> hot;
  /// Gray color table.
  QVector<QRgb> gray;
  /// Active color table pointer.
  QVector<QRgb> *table = nullptr;
  /// Image object for display.
  QImage *image = nullptr;
};


#endif // CAMERA_H
