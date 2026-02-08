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
 * @file CameraAlliedVision.cpp
 * @brief Camera backend for Allied Vision.
 */
#ifdef ALLIEDVISIONCAMERA
#include "CameraAlliedVision.h"

#include <sstream>
#include "Utils.h"

static QString AutoEnumToLabel(const std::string &value) {
  if (value == "On") {
    return QStringLiteral("Continuous");
  }
  if (value == "Once") {
    return QStringLiteral("Once");
  }
  if (value == "Continuous") {
    return QStringLiteral("Continuous");
  }
  return QStringLiteral("Off");
}
#include <exception>
#include <iostream>
#define ENCODING_NUMBER 4
#define AOI_NUMBER 6
#define OFFON_NUMBER 2 # ON = 1, OFF = 0

char *alliedvision_features[]  = {
                        (char*)"Exposure",
                        (char*)"Gain",
                        (char*)"ExposureAuto",
                        (char*)"GainAuto",
                        (char*)"Brightness",
                        (char*)"Saturation",
			(char*)"VideoStabilization",
                        (char*)"ExposureCompensation",
                        (char*)"ShutterSpeed",
                        (char*)"FrameRate",
                        (char*)"Resolution",
                        (char*)"Encoding"
                        };
char *alliedvision_props[]  = {
                        (char*)"Exposure",
                        (char*)"Gain",
                        (char*)"ExposureAuto",
                        (char*)"GainAuto",
                        (char*)"Brightness",
                        (char*)"Saturation",
                        (char*)"Video Stabilization",
                        (char*)"Exposure Compensation",
			(char*)"ShutterSpeed",
			(char*)"Resolution",
                        (char*)"Encoding",
                        (char*)"Frame Rate",
			(char*)"PC Rate"
                        };
char *alliedvision_encodings[]  = {
                           (char*)"YUV420", 
                           (char*)"GRAY", 
                           (char*)"BGR", 
                           (char*)"RGB" 
			};
char *alliedvision_offon[]  = {
                        (char*)"OFF",
                        (char*)"ON"
                        };
char *alliedvision_awb[]  = {
                        (char*)"OFF",
                        (char*)"AUTO",
                        (char*)"SUNLIGHT",
                        (char*)"CLOUDY",
                        (char*)"SHADE",
                        (char*)"TUNGSTEN",
                        (char*)"FLUORESCENT",
                        (char*)"INCANDESCENT",
                        (char*)"FLASH",
                        (char*)"HORIZON"
                        };

int alliedvision_aoi_settings[][2] = {
		       	  {480,320},
			  {960,640},
                          {1440,960},
			  {1920,1280},
			  {2400,1600},
			  {2880,1920}
                          };


CameraAlliedVision::CameraAlliedVision()
  :Camera()
{
  vflip = 0;
  hflip = 0;
  image = nullptr;
  data = nullptr;
  buffer = nullptr;
  snapshot = nullptr;
  buffer32 = nullptr;
  snapshot32 = nullptr;
  suspend = true;
  has_started = false;
  mutex = new QMutex(QMutex::NonRecursive);
  snapshotMutex = new QMutex(QMutex::Recursive);
  acquireMutex = new QMutex(QMutex::Recursive);
  acqstart = new QWaitCondition();
  acqend = new QWaitCondition();
  id = 0;
  frequency = 0.;
  modeCheckEnabled = false;
}

CameraAlliedVision::~CameraAlliedVision()
{
  QLOG_INFO() << "Deleting CameraAlliedVision";
  stop();
  delete mutex;
  delete snapshotMutex;
  delete acquireMutex;
  delete acqstart;
  delete acqend;
  try
    {
         camera->StopContinuousImageAcquisition();
         sys.Shutdown();
         //cleanup_and_exit();
    }
    catch(std::runtime_error& e)
    {
        std::cout << e.what() << std::endl;
    }
    
}
void
CameraAlliedVision::setCamera(void* _camera, int _id)
{
 QLOG_DEBUG () << " Getting camera pointer " << _camera;
 camera = ((VmbCPP::CameraPtr*)_camera)[0];
 vendor = "AlliedVision";
 string modelStr;
 camera->GetName(modelStr);
 model = QString::fromStdString(modelStr);
 QLOG_INFO() << "CameraAlliedVision::setCamera> model name : " << model;

 int featureCnt = -1;
 VmbCPP::FeaturePtr feature;
 
 // Exposure feature
 camera->GetFeatureByName("ExposureTime",feature);
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(alliedvision_features[featureCnt]);
 double exp_min, exp_max;
 feature->GetRange(exp_min, exp_max);
 featureMinList.push_back(exp_min);
 featureMaxList.push_back(exp_max);
 double exposure;
 feature->GetValue(exposure);
 featureValueList.push_back( exposure );
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back( exposure );
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraAlliedVision::setCamera> get exposure feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraAlliedVision::setCamera> value " << QString::number(exposure) << "(min "
             << QString::number(exp_min) << " max " << QString::number(exp_max) << ")";
             
 // Gain feature
 camera->GetFeatureByName("Gain",feature);
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(alliedvision_features[featureCnt]);
 double gain_min, gain_max;
 feature->GetRange(gain_min, gain_max);
 featureMinList.push_back(gain_min);
 featureMaxList.push_back(gain_max);
 double gain;
 feature->GetValue(gain);
 featureValueList.push_back( gain );
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back( gain );
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraAlliedVision::setCamera> get gain feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraAlliedVision::setCamera> value " << QString::number(gain) << "(min "
             << QString::number(gain_min) << " max " << QString::number(gain_max) << ")";

 // Exposure Auto feature
 camera->GetFeatureByName("ExposureAuto",feature);
 {
   VmbCPP::EnumEntryVector entries;
   if (VmbErrorSuccess == feature->GetEntries(entries)) {
     std::ostringstream oss;
     for (size_t i = 0; i < entries.size(); ++i) {
       std::string name;
       if (VmbErrorSuccess == entries[i].GetName(name)) {
         if (i > 0) {
           oss << ", ";
         }
         oss << name;
       }
     }
     QLOG_INFO() << "CameraAlliedVision::setCamera> ExposureAuto entries: "
                 << QString::fromStdString(oss.str());
   } else {
     QLOG_WARN() << "CameraAlliedVision::setCamera> Failed to get ExposureAuto entries";
   }
 }
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(alliedvision_features[featureCnt]);
 double expauto_min = 0, expauto_max = 2;
 featureMinList.push_back(expauto_min);
 featureMaxList.push_back(expauto_max);
 string exposureauto;
 double exposureautoNum = 0;
 feature->GetValue(exposureauto);
 if (exposureauto.compare("Once") == 0) {
    exposureautoNum = 1;
 } else if (exposureauto.compare("Continuous") == 0 ||
            exposureauto.compare("On") == 0) {
    exposureautoNum = 2;
 } else if (exposureauto.compare("Off") == 0) {
    exposureautoNum = 0;
 }
 featureValueList.push_back( exposureautoNum );
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back( exposureautoNum );
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraAlliedVision::setCamera> get exposure auto feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraAlliedVision::setCamera> value " << QString::number(exposureautoNum) << "(min "
             << QString::number(expauto_min) << " max " << QString::number(expauto_max) << ")";
             
 // Gain Auto feature
 camera->GetFeatureByName("GainAuto",feature);
 {
   VmbCPP::EnumEntryVector entries;
   if (VmbErrorSuccess == feature->GetEntries(entries)) {
     std::ostringstream oss;
     for (size_t i = 0; i < entries.size(); ++i) {
       std::string name;
       if (VmbErrorSuccess == entries[i].GetName(name)) {
         if (i > 0) {
           oss << ", ";
         }
         oss << name;
       }
     }
     QLOG_INFO() << "CameraAlliedVision::setCamera> GainAuto entries: "
                 << QString::fromStdString(oss.str());
   } else {
     QLOG_WARN() << "CameraAlliedVision::setCamera> Failed to get GainAuto entries";
   }
 }
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(alliedvision_features[featureCnt]);
 double gainauto_min = 0, gainauto_max = 2;
 featureMinList.push_back(gainauto_min);
 featureMaxList.push_back(gainauto_max);
 string gainauto;
 double gainautoNum = 0;
 feature->GetValue(gainauto);
 if (gainauto.compare("Once") == 0) {
    gainautoNum = 1;
 } else if (gainauto.compare("Continuous") == 0 ||
            gainauto.compare("On") == 0) {
    gainautoNum = 2;
 } else if (gainauto.compare("Off") == 0) {
    gainautoNum = 0;
 }
 featureValueList.push_back( gainautoNum );
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back( gainautoNum );
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraAlliedVision::setCamera> get gain auto feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraAlliedVision::setCamera> value " << QString::number(gainautoNum) << "(min "
             << QString::number(gainauto_min) << " max " << QString::number(gainauto_max) << ")";
             
 // Properties 
 int propCnt = -1;

 // Exposure prop
 QString exposureStr = alliedvision_props[++propCnt];
 QLOG_INFO () << "CameraAlliedVision::setCamera> exposure "
              << exposure;
 exposureStr.append(" : " + QString::number(exposure));
 propList.push_back(exposureStr);
 
 // Gain prop
 QString gainStr = alliedvision_props[++propCnt];
 QLOG_INFO () << "CameraAlliedVision::setCamera> gain "
              << gain;
 gainStr.append(" : " + QString::number(gain));
 propList.push_back(gainStr);
 
 // ExposureAuto prop
 QString expautoStr = alliedvision_props[++propCnt];
 QLOG_INFO () << "CameraAlliedVision::setCamera> exposure auto "
              << exposureautoNum;
 expautoStr.append(" : " + AutoEnumToLabel(exposureauto));
 propList.push_back(expautoStr);
 
 // GainAuto prop
 QString gainautoStr = alliedvision_props[++propCnt];
 QLOG_INFO () << "CameraAlliedVision::setCamera> gain auto "
              << gainautoNum;
 gainautoStr.append(" : " + AutoEnumToLabel(gainauto));
 propList.push_back(gainautoStr);

 // Start Camera acquisition 
 camera_err = connectCamera();

 
 QLOG_DEBUG() << "CameraAlliedVision::setCamera " << vendor << " model : "
              << model << " - Err : " << camera_err;
}
uchar* 
CameraAlliedVision::getSnapshot() {
  snapshotMutex->lock();
  QLOG_INFO() << "CameraAlliedVision::getSnapshot> Image pixel size " << width * height;
  memcpy(snapshot,buffer, width * height * sizeof(uchar));
  snapShotMin = min;
  snapShotMax = max;
  snapshotMutex->unlock();
  return snapshot;
}
ushort*
CameraAlliedVision::getSnapshot16() {
  snapshotMutex->lock();
  QLOG_DEBUG() << "CameraAlliedVision::getSnapshot> Image pixel size " << width * height;
  memcpy(snapshot16,buffer16, width * height * sizeof(ushort));
  snapShotMin = min;
  snapShotMax = max;
  snapshotMutex->unlock();
  return snapshot16;
}

int*
CameraAlliedVision::getSnapshot32() {
  snapshotMutex->lock();
  QLOG_DEBUG() << "CameraAlliedVision::getSnapshot> Image pixel size " << width * height;
  memcpy(snapshot32,buffer32, width * height * sizeof(int));
  snapShotMin = min;
  snapShotMax = max;
  snapshotMutex->unlock();
  return snapshot32;
}
void 
CameraAlliedVision::stop() {
  suspend = true;
  has_started = false;
  wait();
  exit();  
}

void 
CameraAlliedVision::run() {
  int acq_err = 0;
  int acq_cnt = 0;
  if (camera_err == 0 && suspend == true ) {
    suspend = false;
    eTimeTotal = 0;
    while (suspend == false) {
      QLOG_DEBUG () << "CameraAlliedVision::run> " << id << " : start new Acquisition";
      double eTime = Utils::GetTimeMicroseconds();
      acqstart->wakeAll();
      acq_err = acquireImage();
      QLOG_DEBUG () << "CameraAlliedVision::run> " << id << " : done";
      acqend->wakeAll();
      if (acq_err == 1) 
       eTimeTotal+= (Utils::GetTimeMicroseconds() - eTime);
      acq_cnt++;
      if (acq_cnt == FREQUENCY_AVERAGE_COUNT) {
       eTimeTotal/=1e6;
       eTimeTotal/=FREQUENCY_AVERAGE_COUNT;
       if ( eTimeTotal > 0 )
        frequency = 1.0 / eTimeTotal;
       else
        frequency = 0;   
       QLOG_DEBUG() << "CameraAlliedVision::run> Acquisition " 
                   << "freq " << (int) frequency << " Hz";
       eTimeTotal = 0;
       acq_cnt = 0;
       // Update props
       //getProps();
      }
      has_started = true;
    }
    QLOG_DEBUG() << "CameraAlliedVision thread exiting";
  }
  
}

void
CameraAlliedVision::setFeature(int feature, double value) {

   VmbCPP::FeaturePtr featurec;
   string valueStr;
   VmbErrorType err;
   switch ( feature ) {
    case 0:
    QLOG_INFO() << "CameraAlliedVision::setFeature> Update feature " << QString(alliedvision_features[0])
	       << " value " << QString::number(value);
    camera->GetFeatureByName("ExposureTime",featurec);
    featurec->SetValue(value);
    break;
    case 1:
    QLOG_INFO() << "CameraAlliedVision::setFeature> Update feature " << QString(alliedvision_features[1])
	       << " value " << QString::number(value);
    camera->GetFeatureByName("Gain",featurec);
    featurec->SetValue(value);
    break;
    case 2:
    QLOG_INFO() << "CameraAlliedVision::setFeature> Update feature auto " << QString(alliedvision_features[2])
	       << " value " << QString::number(value);
    camera->GetFeatureByName("ExposureAuto",featurec);
    if ( value < 0.5 ) {
       valueStr = "Off";
       err = featurec->SetValue(valueStr.c_str());
    }
    else {
       if (value < 1.5) {
         valueStr = "Once";
         err = featurec->SetValue(valueStr.c_str());
         if (err != VmbErrorSuccess) {
           valueStr = "Continuous";
           err = featurec->SetValue(valueStr.c_str());
         }
       } else {
         valueStr = "Continuous";
         err = featurec->SetValue(valueStr.c_str());
         if (err != VmbErrorSuccess) {
           valueStr = "Once";
           err = featurec->SetValue(valueStr.c_str());
         }
       }
       if (err != VmbErrorSuccess) {
         valueStr = "On";
         err = featurec->SetValue(valueStr.c_str());
       }
    }
    if (err != VmbErrorSuccess) {
       QLOG_ERROR() << "CameraAlliedVision::setFeature> Could not set Exposure Auto value; error = " << QString::number(err);
    } else {
       std::string current;
       if (VmbErrorSuccess == featurec->GetValue(current)) {
         QLOG_INFO() << "CameraAlliedVision::setFeature> ExposureAuto now " << QString::fromStdString(current);
       }
    }
    break;
    case 3:
    QLOG_INFO() << "CameraAlliedVision::setFeature> Update feature " << QString(alliedvision_features[3])
	       << " value " << QString::number(value);
    camera->GetFeatureByName("GainAuto",featurec);
    if ( value < 0.5 ) {
       valueStr = "Off";
       err = featurec->SetValue(valueStr.c_str());
    }
    else {
       if (value < 1.5) {
         valueStr = "Once";
         err = featurec->SetValue(valueStr.c_str());
         if (err != VmbErrorSuccess) {
           valueStr = "Continuous";
           err = featurec->SetValue(valueStr.c_str());
         }
       } else {
         valueStr = "Continuous";
         err = featurec->SetValue(valueStr.c_str());
         if (err != VmbErrorSuccess) {
           valueStr = "Once";
           err = featurec->SetValue(valueStr.c_str());
         }
       }
       if (err != VmbErrorSuccess) {
         valueStr = "On";
         err = featurec->SetValue(valueStr.c_str());
       }
    }
    if (err != VmbErrorSuccess) {
       QLOG_ERROR() << "CameraAlliedVision::setFeature> Could not set Gain Auto value; error = " << QString::number(err);
    } else {
       std::string current;
       if (VmbErrorSuccess == featurec->GetValue(current)) {
         QLOG_INFO() << "CameraAlliedVision::setFeature> GainAuto now " << QString::fromStdString(current);
       }
    }
    break;
    default:
    break;
   }
   getFeatures();
   getProps();
}
void
CameraAlliedVision::setMode(int feature, bool value) {
  switch ( feature ) {
   default:
   break;
 }
}

void
CameraAlliedVision::getProps() {
/*-----------------------------------------------------------------------
 *  report camera's properties
 *-----------------------------------------------------------------------*/
  // Properties
 int propCnt = -1;
 VmbCPP::FeaturePtr feature;
 
 // Exposure prop
 QString exposureStr = alliedvision_props[++propCnt];
 camera->GetFeatureByName("ExposureTime",feature);
 double exposure;
 feature->GetValue(exposure);
 exposureStr.append(" : " + QString::number(exposure));
 propList.replace(propCnt, exposureStr);

 // Gain prop
 QString gainStr = alliedvision_props[++propCnt];
 camera->GetFeatureByName("Gain",feature);
 double gain;
 feature->GetValue(gain);
 gainStr.append(" : " + QString::number(gain));
 propList.replace(propCnt, gainStr);

 // ExposureAuto prop
 QString expautoStr = alliedvision_props[++propCnt];
 camera->GetFeatureByName("ExposureAuto",feature);
 std::string exposureauto;
 feature->GetValue(exposureauto);
 expautoStr.append(" : " + AutoEnumToLabel(exposureauto));
 propList.replace(propCnt, expautoStr);

 // GainAuto prop
 QString gainautoStr = alliedvision_props[++propCnt];
 camera->GetFeatureByName("GainAuto",feature);
 std::string gainauto;
 feature->GetValue(gainauto);
 gainautoStr.append(" : " + AutoEnumToLabel(gainauto));
 propList.replace(propCnt, gainautoStr);

 
 
 QLOG_DEBUG() << "CameraAlliedVision::getProps> Properties updated";
 emit updateProps();
}

void
CameraAlliedVision::getFeatures() {
  /*-----------------------------------------------------------------------
   *  report camera's features
   *-----------------------------------------------------------------------*/
  // Exposure feature
  int featureCnt = -1;
  VmbCPP::FeaturePtr feature;
  // Exposure feature
  camera->GetFeatureByName("ExposureTime",feature);
  double exposure;
  feature->GetValue(exposure);
  featureValueList.replace(++featureCnt, exposure);
  featureAbsValueList.replace(featureCnt, exposure);
  
  // Gain feature
  camera->GetFeatureByName("Gain",feature);
  double gain;
  feature->GetValue(gain);
  featureValueList.replace(++featureCnt, gain);
  featureAbsValueList.replace(featureCnt, gain);

  // Exposure Auto feature
  camera->GetFeatureByName("ExposureAuto",feature);
  string exposureauto;
  double exposureautoNum = 0;
  feature->GetValue(exposureauto);
  if (exposureauto.compare("Once") == 0) {
    exposureautoNum = 1;
  } else if (exposureauto.compare("Continuous") == 0 ||
             exposureauto.compare("On") == 0) {
    exposureautoNum = 2;
  }
  featureValueList.replace(++featureCnt, exposureautoNum);
  featureAbsValueList.replace(featureCnt, exposureautoNum);

  // Gain Auto feature
  camera->GetFeatureByName("GainAuto",feature);
  string gainauto;
  double gainautoNum = 0;
  feature->GetValue(gainauto);
  if (gainauto.compare("Once") == 0) {
    gainautoNum = 1;
  } else if (gainauto.compare("Continuous") == 0 ||
             gainauto.compare("On") == 0) {
    gainautoNum = 2;
  }
  featureValueList.replace(++featureCnt, gainautoNum);
  featureAbsValueList.replace(featureCnt, gainautoNum);

  emit updateFeatures();
}

int 
CameraAlliedVision::findCamera() {
  int i;
  /* Find available AlliedVision cameras*/
  // Allocate camera list
  // Open Camera with id = 0;
  num = 0; // no camera found yet
  VmbVersionInfo_t versionInfo;
  sys.QueryVersion(versionInfo);
  QLOG_INFO() << "Vmb Version Major: " << versionInfo.major << " Minor: " << versionInfo.minor << " Patch: " << versionInfo.patch;

  VmbErrorType err = sys.Startup();           // Initialize the Vmb API
  
  if (VmbErrorSuccess == err) {

      VmbCPP::TransportLayerPtrVector transportlayers;     // A vector of std::shared_ptr<AVT::VmbAPI::TransportLayer> objects
      err = sys.GetTransportLayers(transportlayers);       // Fetch all transport layers
      if (VmbErrorSuccess == err) 
          QLOG_INFO() << "TransportLayers found: " << transportlayers.size();

          VmbCPP::InterfacePtrVector interfaces;     // A vector of std::shared_ptr<AVT::VmbAPI::Interface> objects
          err = sys.GetInterfaces(interfaces);       // Fetch all interfaces
          if (VmbErrorSuccess == err) 
          QLOG_INFO() << "Interfaces found: " << interfaces.size();

          
          err = sys.GetCameras(cameras);          // Fetch all cameras
          if (VmbErrorSuccess == err) {
              num = 1; // Found camera
              QLOG_INFO() << "CameraAlliedVision::findCamera> opening camera 0";

              // Query all static details of all known cameras and print them out.
              // We don't have to open the cameras for that.
              camera = cameras[0];
              PrintCameraInfo(camera);
          }
          else {
              QLOG_INFO() << "Could not list cameras. Error code: " << err;
              sys.Shutdown();
              return -1;
          }
                
   }
   else {
       QLOG_INFO() << "Could not start system. Error code: " << err;
       sys.Shutdown();
       return -1;
   }
   
  QLOG_INFO() << "CameraAlliedVision::findCamera> opening camera 0";
  
  err = camera->Open(VmbAccessModeFull);

  if ( err != VmbErrorSuccess ) {
    QLOG_ERROR() << "Error opening ALLIEDVISION camera : error=" << QString(err);
    sys.Shutdown();
    return -1;
  }
  string name;
  err = camera->GetName(name);
  QLOG_INFO() << "CameraAlliedVision::findCamera> camera " << QString::fromStdString(name) << " open ";
  cameralist.push_back(&camera);
  vendorlist.push_back("AlliedVision");
  modelist.push_back("V4");
  return (0);
}
int 
CameraAlliedVision::connectCamera() {
  
  camera_err = -1;

  /* Init camera*/
  QLOG_INFO() << "CameraAlliedVision::connectCamera:: Start connecting camera...";
  
  /*-----------------------------------------------------------------------
   *  setup capture
   *-----------------------------------------------------------------------*/
  VmbCPP::FeaturePtr feature;
  VmbInt64_t width_d = 5496, height_d = 3672;
  camera->GetFeatureByName("Width",feature);
  feature->SetValue(width_d);
  feature->GetValue(width_d);
  camera->GetFeatureByName("Height",feature);
  feature->SetValue(height_d);
  feature->GetValue(height_d);
  width = (unsigned int) width_d;
  height = (unsigned int) height_d;
  QLOG_INFO() << "CameraAlliedVision::connectCamera>> Image width = " << QString::number(width);
  QLOG_INFO() << "CameraAlliedVision::connectCamera>> Image height = " << QString::number(height);
  
  buffer = (uchar*)malloc( sizeof(uchar) * width * height);
  snapshot = (uchar*)malloc( sizeof(uchar) * width * height);
  snapshot16 = (ushort*)malloc( sizeof(ushort) * width * height);
  buffer32 = (int*)malloc( sizeof(int) * width * height);
  snapshot32 = (int*)malloc( sizeof(int) * width * height);

  image = new QImage(buffer,width,height,width,QImage::Format_Indexed8);
  buffer = nullptr;
  image->setColorTable(*table);
  
  // Setup camera frames and start capture
  frameObs = new FrameObserver(camera);
  VmbErrorType err = camera->StartContinuousImageAcquisition(5,(VmbCPP::IFrameObserverPtr)(frameObs));
  if (err != VmbErrorSuccess)
     QLOG_ERROR() << "CameraAlliedVision::connectCamera> Could not start Acquisition; error = " << QString::number(err);
     
  VmbUint32_t nPayloadSize;
  err = camera->GetPayloadSize(nPayloadSize);
  QLOG_INFO() << "CameraAlliedVision::connectCamera> camera payload size = " << QString::number(nPayloadSize);
  if (err != VmbErrorSuccess)
     QLOG_ERROR() << "CameraAlliedVision::connectCamera> Could not get payload size; error = " << QString::number(err);
     
     
  err = camera->GetStreams(streams);
  if (err != VmbErrorSuccess)
     QLOG_ERROR() << "CameraAlliedVision::connectCamera> Could not get streams; error = " << QString::number(err);
   
  pixel_encoding = B8; // Use only this for the moment
  
  // Refresh properties
  getProps();
  // Refresh features
  getFeatures();

  return 0;
 

  return 0;
}

void  
CameraAlliedVision::cleanup_and_exit()
{
  QLOG_DEBUG() << "Cleanup camera AlliedVision connection";
  if (data) { free(data); data = nullptr;}
  if (buffer) { free(buffer); buffer = nullptr;}
  if (snapshot) { free(snapshot); snapshot = nullptr;}
  if (buffer32) { free(buffer32); buffer32 = nullptr;}
  if (snapshot32) { free(snapshot32); snapshot32 = nullptr;}
  if (image) delete image;

  return;
}

int 
CameraAlliedVision::acquireImage() {
   
  int acq_err = 1;
  QLOG_DEBUG () << " Cycle start ";
  double eTime = Utils::GetTimeMicroseconds();
  acquireMutex->lock();
  /*-----------------------------------------------------------------------
   * acquire frame
   *-----------------------------------------------------------------------*/
  // Lock the acquisition
  snapshotMutex->lock();
  usleep(10000);
  buffer = frameObs->GetImage();
  while ( buffer == nullptr) {
      QLOG_DEBUG() << "BUFFER IS nullptr...ACQUIRE ";
      usleep(10000);
      buffer = frameObs->GetImage();
  }
  QLOG_DEBUG() << "GETIMAGE DONE "; 
  
     // calculate min,max
     max = 0;
     min = 255;
     avg = 0;
     for (int i = 0; i < height*width; i++) {
       if (buffer[i] < min) {
          min = buffer[i];
       }
       else if (buffer[i] > max) {
          max = buffer[i];
       }
       avg+=buffer[i];
     }
     avg = (int) (avg / (width * height));
     QLOG_DEBUG() << "min = " << min;
     QLOG_DEBUG() << "max = " << max;
     QLOG_DEBUG() << "avg = " << avg;
     snapshotMutex->unlock();
  /*if (vflip) {
      buffer = reversebytes(buffer,height * width);
      buffer = fliphorizontal(buffer,height * width, width);
      buffer32 = reversebytes(buffer32,height * width);
      buffer32 = fliphorizontal(buffer32,height * width, width);
  }
  if (hflip) {
     buffer = fliphorizontal(buffer,height*width, width);
     buffer32 = fliphorizontal(buffer32,height*width, width);
  }TBCORRECTED
    */ 
  // Format video image
   QLOG_DEBUG() << " IMAGE SIZE = " << imageWidth << " x " << imageHeight;
   QImage imagescaled = image->scaled(imageWidth,imageHeight);
   QImage imagergb32 =  imagescaled.convertToFormat(QImage::Format_ARGB32_Premultiplied);
   emit getImage(imagergb32);
   emit updateMin(min);
   emit updateMax(max);
   emit updateAvg(avg);
 
   eTime = Utils::GetTimeMicroseconds() - eTime;
   QLOG_DEBUG () << " Process eTime " << QString::number(eTime);
   QLOG_DEBUG () << " Cycle end ";
   acquireMutex->unlock();
   return (acq_err);
}

void CameraAlliedVision::setImageSize(const int &_imageWidth, const int &_imageHeight){
  imageWidth = _imageWidth;
  imageHeight = _imageHeight;
  QLOG_DEBUG ( ) <<  "emit new image with size " << imageWidth  << "x" << imageHeight;
}

/**printing camera info for a camera.
        *\note this function is used with for_each and is called for each camera in range cameras.begin(), cameras.end()
        */
void CameraAlliedVision::PrintCameraInfo(const VmbCPP::CameraPtr& camera) {
            string strID;
            string strName;
            string strModelName;
            string strSerialNumber;
            string strInterfaceID;
            VmbCPP::TransportLayerPtr pTransportLayer;
            string strTransportLayerID;
            string strTransportLayerPath;

            ostringstream ErrorStream;

            VmbErrorType err = camera->GetID(strID);
            
            if (VmbErrorSuccess != err)
            {
                ErrorStream << "[Could not get camera ID. Error code: " << err << "]";
                strID = ErrorStream.str();
            }

            err = camera->GetName(strName);
            if (VmbErrorSuccess != err)
            {
                ErrorStream << "[Could not get camera name. Error code: " << err << "]";
                strName = ErrorStream.str();
            }

            err = camera->GetModel(strModelName);
            if (VmbErrorSuccess != err)
            {
                ErrorStream << "[Could not get camera mode name. Error code: " << err << "]";
                strModelName = ErrorStream.str();
            }

            err = camera->GetSerialNumber(strSerialNumber);
            if (VmbErrorSuccess != err)
            {
                ErrorStream << "[Could not get camera serial number. Error code: " << err << "]";
                strSerialNumber = ErrorStream.str();
            }

            err = camera->GetInterfaceID(strInterfaceID);
            if (VmbErrorSuccess != err)
            {
                ErrorStream << "[Could not get interface ID. Error code: " << err << "]";
                strInterfaceID = ErrorStream.str();
            }

            err = camera->GetTransportLayer(pTransportLayer);
            err = pTransportLayer->GetID(strTransportLayerID);
            if (VmbErrorSuccess != err)
            {
                ErrorStream << "[Could not get transport layer ID. Error code: " << err << "]";
                strTransportLayerID = ErrorStream.str();
            }
            err = pTransportLayer->GetPath(strTransportLayerPath);
            if (VmbErrorSuccess != err)
            {
                ErrorStream << "[Could not get transport layer path. Error code: " << err << "]";
                strTransportLayerPath = ErrorStream.str();
            }

            
            QLOG_INFO()     << " Camera Name         : " << QString::fromStdString(strName);
            QLOG_INFO()     << " Model Name          : " << QString::fromStdString(strModelName);
            QLOG_INFO()     << " Camera ID           : " << QString::fromStdString(strID);
            QLOG_INFO()     << " Serial Number       : " << QString::fromStdString(strSerialNumber);
            QLOG_INFO()     << " Interface ID        : " << QString::fromStdString(strInterfaceID);
            QLOG_INFO()     << " TransportLayer ID   : " << QString::fromStdString(strTransportLayerID);
            QLOG_INFO()     << " TransportLayer Path : " << QString::fromStdString(strTransportLayerPath);
        }

#endif
