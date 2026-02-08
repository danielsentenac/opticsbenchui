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

   auto appendHotRedRange = [this](int start, int end) {
     for (int value = start; value <= end; ++value) {
       hot << qRgb(value, 0, 0);
     }
   };
   auto appendHotGreenRange = [this](int start, int end) {
     for (int value = start; value <= end; ++value) {
       hot << qRgb(255, value, 0);
     }
   };
   auto appendHotBlueRange = [this](int start, int end) {
     for (int value = start; value <= end; ++value) {
       hot << qRgb(255, 255, value);
     }
   };

   appendHotRedRange(8, 11);
   appendHotRedRange(19, 22);
   appendHotRedRange(29, 32);
   appendHotRedRange(40, 43);
   appendHotRedRange(51, 54);
   appendHotRedRange(61, 64);
   appendHotRedRange(72, 75);
   appendHotRedRange(82, 85);
   appendHotRedRange(93, 96);
   appendHotRedRange(104, 107);
   appendHotRedRange(114, 117);
   appendHotRedRange(125, 128);
   appendHotRedRange(136, 139);
   appendHotRedRange(146, 149);
   appendHotRedRange(156, 159);
   appendHotRedRange(168, 171);
   appendHotRedRange(178, 181);
   appendHotRedRange(189, 192);
   appendHotRedRange(198, 199);
   appendHotRedRange(201, 202);
   appendHotRedRange(210, 213);
   appendHotRedRange(221, 224);
   appendHotRedRange(231, 234);
   appendHotRedRange(242, 245);
   appendHotRedRange(252, 255);

   appendHotGreenRange(8, 11);
   appendHotGreenRange(19, 22);
   appendHotGreenRange(29, 32);
   appendHotGreenRange(40, 43);
   appendHotGreenRange(51, 54);
   appendHotGreenRange(61, 64);
   appendHotGreenRange(72, 75);
   appendHotGreenRange(82, 85);
   appendHotGreenRange(93, 96);
   appendHotGreenRange(104, 107);
   appendHotGreenRange(114, 117);
   appendHotGreenRange(125, 128);
   appendHotGreenRange(136, 139);
   appendHotGreenRange(146, 149);
   appendHotGreenRange(157, 160);
   appendHotGreenRange(168, 171);
   appendHotGreenRange(178, 181);
   appendHotGreenRange(189, 192);
   appendHotGreenRange(199, 202);
   appendHotGreenRange(210, 213);
   appendHotGreenRange(221, 224);
   appendHotGreenRange(231, 234);
   appendHotGreenRange(242, 245);
   appendHotGreenRange(252, 255);

   appendHotBlueRange(13, 16);
   appendHotBlueRange(29, 32);
   appendHotBlueRange(45, 48);
   appendHotBlueRange(61, 64);
   appendHotBlueRange(77, 80);
   appendHotBlueRange(93, 96);
   appendHotBlueRange(109, 112);
   appendHotBlueRange(125, 128);
   appendHotBlueRange(141, 144);
   appendHotBlueRange(157, 160);
   appendHotBlueRange(173, 176);
   appendHotBlueRange(189, 192);
   appendHotBlueRange(205, 208);
   appendHotBlueRange(221, 224);
   appendHotBlueRange(237, 240);
   appendHotBlueRange(252, 255);

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
