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
 * @file CameraRAPTORNINOX640.h
 * @brief Camera backend for RAPTORNINOX 640.
 */
#ifdef RAPTORNINOX640CAMERA
#ifndef CAMERARAPTORNINOX640_H
#define CAMERARAPTORNINOX640_H
#include <xcliball.h>
#include "Camera.h"

/// \ingroup cameras
/// Raptor Ninox 640 camera implementation.
class CameraRAPTORNINOX640 : public Camera
{
  Q_OBJECT
    
    public:
  /// Constructor.
  CameraRAPTORNINOX640();
  /// Destructor.
  ~CameraRAPTORNINOX640() override;
 
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
  /// Read a camera feature register.
  /// \param sendreg Command buffer.
  /// \param size_sreg Command size.
  /// \param readreg Read buffer.
  /// \param size_rreg Read size.
  /// \param data Output data buffer.
  /// \param size_data Output size.
  int  readFeature(char* sendreg, int size_sreg, char* readreg, int size_rreg, uchar* data, int size_data);
  /// Write a camera feature register.
  /// \param sendreg Command buffer.
  /// \param size_sreg Command size.
  int  writeFeature(char* sendreg, int size_sreg);
  double getPCBtemperature();
  double getCCDtemperature();
  QString getBinningFactor();
  /// Set the sensor binning factor.
  /// \param value Binning value.
  void setBinningFactor(int value);
  /// Update frame grabber AOI.
  /// \param v0 Left.
  /// \param v1 Right.
  /// \param v2 Top.
  /// \param v3 Bottom.
  void updateFrameGrabberAOI(int v0, int v1, int v2, int v3);
  /// Set area of interest.
  /// \param left Left position.
  /// \param width Width in pixels.
  /// \param top Top position.
  /// \param height Height in pixels.
  void   setAOI(int left, int width, int top, int height);
  double getExposure();
  /// Set exposure time.
  /// \param e Exposure value.
  void   setExposure(double e);
  double getFramerate();
  /// Set framerate.
  /// \param f Framerate value.
  void   setFramerate(double f);
  int    getGain();
  /// Set analog gain.
  /// \param g Gain value.
  void   setGain(int g);
  double getDGain();
  /// Set digital gain.
  /// \param g Gain value.
  void   setDGain(int g);
  int    getNUC();
  /// Set NUC mode.
  /// \param n NUC value.
  void   setNUC(int n);
  QString getBloomState();
  /// Set bloom state.
  /// \param value Bloom setting.
  void   setBloomState(int value);
  struct xclibs *xc = nullptr;
  struct pxvidstate   *vidstate = nullptr;
  struct xcdevservice xcdev = {};
  pxbuffer_t framenum = 0;
  ushort *image16 = nullptr;
  int imageWidth = 0;
  int imageHeight = 0;
  double frate = 0.0;
  double frameTotal = 0.0;
  bool binning_changed = false;
  bool aoi_changed = false;
  int readManufacturingInfo();
  int adcCal0 = 0;
  int adcCal40 = 0;
  int dacCal0 = 0;
  int dacCal40 = 0;
};

#endif // CAMERARAPTORNINOX640_H
#endif
