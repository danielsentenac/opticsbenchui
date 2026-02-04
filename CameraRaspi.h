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
 * @file CameraRaspi.h
 * @brief Camera backend for Raspi.
 */
#ifdef RASPICAMERA
#ifndef CAMERARASPI_H
#define CAMERARASPI_H
#include "raspicam.h"
#include "Camera.h"

using namespace raspicam;

/// \ingroup cameras
/// Raspberry Pi camera implementation.
class CameraRaspi : public Camera
{
  Q_OBJECT
    
    public:
  /// Constructor.
  CameraRaspi();
  /// Destructor.
  ~CameraRaspi() override;
 
  /// Stop acquisition and release resources.
  void stop() override;
  /// Discover available cameras.
  int  findCamera() override;
  /// Attach a camera handle and id.
  /// \param _camera Backend camera pointer.
  /// \param _id Camera index.
  void setCamera(void *_camera, int _id) override;
  /// Query and cache camera features.
  void getFeatures() override;
  /// Acquire a single 8-bit image.
  uchar* getSnapshot() override;
  /// Acquire a single 16-bit image.
  ushort *getSnapshot16() override;
  /// Acquire a single 32-bit image.
  int* getSnapshot32() override;
  
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
  
  
  public slots:
  /// Set the image size for acquisition.
  /// \param _imageWidth Width in pixels.
  /// \param _imageHeight Height in pixels.
  void setImageSize(const int &_imageWidth, const int &_imageHeight) override;
  /// Set a numeric feature value.
  /// \param feature Feature identifier.
  /// \param value Feature value.
  void setFeature(int feature, double value) override;
  /// Enable or disable a feature mode.
  /// \param feature Feature identifier.
  /// \param value True for auto/enable, false for manual/disable.
  void setMode(int feature, bool value) override;
  /// Query the camera for its properties.
  void getProps() override;

 private:
  void run() override;
  int  connectCamera() override;
  int  acquireImage() override;
  void cleanup_and_exit() override;
  int imageWidth = 0;
  int imageHeight = 0;
  // Raspi Camera
  RaspiCam *camera = nullptr;
  double brightness = 0.0;
  double sharpness = 0.0;
  double iso = 0.0;
  double saturation = 0.0;
  double contrast = 0.0;
  double frate = 0.0;
  double exposure_compensation = 0.0;
  double shutterspeed = 0.0;
  unsigned char *data = nullptr;
  int encoding_num = 0;
  int awb_num = 0;
  int exposure_num = 0;
  int aoi_num = 0;
  int acq_num = 0;
  int videostabilization_num = 0;
  int resolution_num = 0;
  double eTimeTotal = 0.0;
  double frequency = 0.0;
};

#endif // CAMERARASPI_H
#endif
