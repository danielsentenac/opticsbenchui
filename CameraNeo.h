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
#ifdef NEOCAMERA
#ifndef CAMERANEO_H
#define CAMERANEO_H
#include "Camera.h"
#include "atcore.h"

/// \ingroup cameras
/// Andor Neo camera implementation.
class CameraNeo : public Camera
{
  Q_OBJECT
    
    public:
  /// Constructor.
  CameraNeo();
  /// Destructor.
  ~CameraNeo() override;
 
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
  /// Convert SDK error code to string.
  /// \param _i_errorCode SDK error code.
  const char * sdkErrorString(int _i_errorCode);
  /// Check and log SDK errors.
  /// \param _i_err SDK error code.
  /// \param _sz_caller Caller context string.
  bool errorOk(int _i_err, const char * _sz_caller);

  int imageWidth = 0;
  int imageHeight = 0;
  // Neo Andor
  int i_err = 0;
  int i_available = 0;
  AT_H *camera = nullptr;
  double exposure = 0.0;
  double frate = 0.0;
  double temp = 0.0;
  int rrate = 0;
  AT_64 BufferSize = 0;
  AT_64 aoi_height = 0;
  AT_64 aoi_width = 0;
  AT_64 aoi_left = 0;
  AT_64 aoi_top = 0;
  ushort** AlignedBuffers = nullptr;
  int FrameNumber = 0;
  int encoding_num = 0;
  int gain_num = 0;
  int trigger_num = 0;
  int acq_num = 0;
  double eTimeTotal = 0.0;
  double frequency = 0.0;
};

#endif // CAMERANEO_H
#endif
