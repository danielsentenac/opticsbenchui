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

class CameraAlliedVision : public Camera
{
  Q_OBJECT
    
    
    public:
  CameraAlliedVision();
  ~CameraAlliedVision();
 
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
  void  updateProps();
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
  /**
  * \brief IFrameObserver implementation for asynchronous image acquisition
  */
  class FrameObserver : public VmbCPP::IFrameObserver {
     public:
        FrameObserver(VmbCPP::CameraPtr camera) : VmbCPP::IFrameObserver(camera) {};

        void FrameReceived(const VmbCPP::FramePtr frame) {
           pBuf = NULL;
           if (frame != NULL) {
              VmbUint32_t width = 0, height = 0;
              frame->GetWidth(width);
              frame->GetHeight(height);
              QLOG_DEBUG() << "CameraAlliedVision::FrameObserver> width = " << QString::number((unsigned int)width);
              QLOG_DEBUG() << "CameraAlliedVision::FrameObserver> width = " << QString::number((unsigned int)height);
              frame->GetImage(pBuf);
           }
           else {
              pBuf = NULL;
              QLOG_WARN() << "CameraAlliedVision::FrameObserver> frame is NULL!!";
              }
           m_pCamera->QueueFrame(frame);          
        };
        
        uchar* GetImage() {
           return ((uchar*) pBuf);
        };
     private:
        VmbUchar_t *pBuf;
 
  };
  VmbCPP::VmbSystem& sys = VmbCPP::VmbSystem::GetInstance();  // Get a reference to the VimbaSystem singleton
  FrameObserver *frameObs;
  void PrintCameraInfo(const VmbCPP::CameraPtr& camera);
  int imageWidth;
  int imageHeight;
  VmbCPP::CameraPtr camera;
  VmbCPP::FeaturePtr features;
  VmbCPP::StreamPtrVector streams;
  VmbCPP::CameraPtrVector cameras;   // A vector of std::shared_ptr<AVT::VmbAPI::Camera> objects
  double brightness, sharpness, iso, saturation, contrast, frate;
  double exposure_compensation, shutterspeed;
  unsigned char *data;
  int encoding_num, awb_num, exposure_num, aoi_num, acq_num;
  int videostabilization_num, resolution_num;
  double eTimeTotal, frequency;
};

#endif // CAMERAALLIEDVISION_H
#endif
