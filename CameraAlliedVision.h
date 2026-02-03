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
#ifdef ALLIEDVISIONCAMERA
#ifndef CAMERAALLIEDVISION_H
#define CAMERAALLIEDVISION_H
#include <VmbCPP/VmbCPP.h>
#include "Camera.h"
#include <sstream>

using namespace std;

/// \ingroup cameras
/// Allied Vision camera implementation using Vimba.
class CameraAlliedVision : public Camera
{
  Q_OBJECT
    
    
    public:
  /// Constructor.
  CameraAlliedVision();
  /// Destructor.
  ~CameraAlliedVision() override;
 
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
  /**
  * \brief IFrameObserver implementation for asynchronous image acquisition
  */
  class FrameObserver : public VmbCPP::IFrameObserver {
     public:
        /// Construct a frame observer for a camera.
        /// \param camera Camera instance.
        FrameObserver(VmbCPP::CameraPtr camera) : VmbCPP::IFrameObserver(camera) {};

        /// Called when a frame is received.
        /// \param frame Frame pointer.
        void FrameReceived(const VmbCPP::FramePtr frame) {
           pBuf = nullptr;
           if (frame != nullptr) {
              VmbUint32_t width = 0, height = 0;
              frame->GetWidth(width);
              frame->GetHeight(height);
              QLOG_DEBUG() << "CameraAlliedVision::FrameObserver> width = " << QString::number((unsigned int)width);
              QLOG_DEBUG() << "CameraAlliedVision::FrameObserver> width = " << QString::number((unsigned int)height);
              frame->GetImage(pBuf);
           }
           else {
              pBuf = nullptr;
              QLOG_WARN() << "CameraAlliedVision::FrameObserver> frame is NULL!!";
              }
           m_pCamera->QueueFrame(frame);          
        };
        
        /// Return the last received image buffer.
        uchar* GetImage() {
           return ((uchar*) pBuf);
        };
     private:
        VmbUchar_t *pBuf = nullptr;
 
  };
  VmbCPP::VmbSystem& sys = VmbCPP::VmbSystem::GetInstance();  // Get a reference to the VimbaSystem singleton
  FrameObserver *frameObs = nullptr;
  void PrintCameraInfo(const VmbCPP::CameraPtr& camera);
  int imageWidth = 0;
  int imageHeight = 0;
  VmbCPP::CameraPtr camera;
  VmbCPP::FeaturePtr features;
  VmbCPP::StreamPtrVector streams;
  VmbCPP::CameraPtrVector cameras;   // A vector of std::shared_ptr<AVT::VmbAPI::Camera> objects
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

#endif // CAMERAALLIEDVISION_H
#endif
