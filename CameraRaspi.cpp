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
#ifdef RASPICAMERA
#include "CameraRaspi.h"
#include "Utils.h"

#define EXPOSURE_NUMBER 13
#define ENCODING_NUMBER 4
#define AOI_NUMBER 6
#define AWB_NUMBER 10

char *raspi_features[]  = {
                        (char*)"Exposure",
                        (char*)"ISO",
                        (char*)"AWB",
                        (char*)"Sharpness",
                        (char*)"Brightness",
                        (char*)"Saturation",
			(char*)"VideoStabilization",
                        (char*)"ExposureCompensation",
                        (char*)"ShutterSpeed",
                        (char*)"FrameRate",
                        (char*)"Resolution",
                        (char*)"Encoding"
                        };
char *raspi_props[]  = {
                        (char*)"Exposure",
                        (char*)"ISO",
                        (char*)"AWB",
                        (char*)"Sharpness",
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
char *raspi_encodings[]  = {
                           (char*)"YUV420", 
                           (char*)"GRAY", 
                           (char*)"BGR", 
                           (char*)"RGB" 
			};
char *raspi_exposure[]  = {
                        (char*)"OFF",
                        (char*)"AUTO",
                        (char*)"NIGHT",
                        (char*)"NIGHTPREVIEW",
                        (char*)"BACKLIGHT",
                        (char*)"SPOLIGHT",
                        (char*)"SPORTS",
                        (char*)"SNOW",
                        (char*)"BEACH",
                        (char*)"VERYLONG",
                        (char*)"FIXEDFPS",
                        (char*)"ANTISHAKE",
                        (char*)"FIREWORKS"
                        };
char *raspi_awb[]  = {
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

int raspi_aoi_settings[][2] = {
		       	  {480,320},
			  {960,640},
                          {1440,960},
			  {1920,1280},
			  {2400,1600},
			  {2880,1920}
                          };


CameraRaspi::CameraRaspi()
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

CameraRaspi::~CameraRaspi()
{
  QLOG_INFO() << "Deleting CameraRaspi";
  stop();
  cleanup_and_exit();
  delete mutex;
  delete snapshotMutex;
  delete acquireMutex;
  delete acqstart;
  delete acqend;
}
void
CameraRaspi::setCamera(void* _camera, int _id)
{
 QLOG_DEBUG () << " Getting camera pointer " << _camera;
 camera = (RaspiCam*)_camera;
 vendor = "Raspberry";
 model = QString("V2");

 int featureCnt = -1;
 
 // Exposure feature
 int exp_min = 0, exp_max = EXPOSURE_NUMBER -1;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raspi_features[featureCnt]);
 featureMinList.push_back(exp_min);
 featureMaxList.push_back(exp_max);
 exposure_num = camera->getExposure();
 featureValueList.push_back( exposure_num );
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(exposure_num);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraRaspi::setCamera> get exposure feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraRaspi::setCamera> value " << exposure_num << "(min "
             << exp_min << " max " << exp_max << ")";

 // ISO feature
 iso = camera->getISO();
 int iso_min = 100, iso_max = 800;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raspi_features[featureCnt]);
 featureValueList.push_back(iso);
 featureMinList.push_back(iso_min);
 featureMaxList.push_back(iso_max);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(iso);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraRaspi::setCamera> get iso feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraRaspi::setCamera> value " << iso << "(min "
             << iso_min << " max " << iso_max  << ")";

 // AWB feature
 awb_num = camera->getAWB();
 int awb_min = 0, awb_max = AWB_NUMBER - 1;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raspi_features[featureCnt]);
 featureValueList.push_back(awb_num);
 featureMinList.push_back(awb_min);
 featureMaxList.push_back(awb_max);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(awb_num);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraRaspi::setCamera> get awb feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraRaspi::setCamera> value " << awb_num << "(min "
             << awb_min << " max " << awb_max  << ")";
 
 // Sharpness feature
 sharpness = camera->getSharpness();
 int sharpness_min = -100, sharpness_max = 100;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raspi_features[featureCnt]);
 featureValueList.push_back(sharpness);
 featureMinList.push_back(sharpness_min);
 featureMaxList.push_back(sharpness_max);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(sharpness);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraRaspi::setCamera> get sharpness feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraRaspi::setCamera> value " << sharpness << "(min "
             << sharpness_min << " max " << sharpness_max  << ")";

 // Brightness feature
 camera->setBrightness(50);
 brightness = camera->getBrightness();
 int brightness_min = 0, brightness_max = 100;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raspi_features[featureCnt]);
 featureValueList.push_back(brightness);
 featureMinList.push_back(brightness_min);
 featureMaxList.push_back(brightness_max);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(brightness);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraRaspi::setCamera> get brightness feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraRaspi::setCamera> value " << brightness << "(min "
             << brightness_min << " max " << brightness_max  << ")";
 
 // Saturation feature
 saturation = camera->getSaturation();
 int saturation_min = -100, saturation_max = 100;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raspi_features[featureCnt]);
 featureValueList.push_back(saturation);
 featureMinList.push_back(saturation_min);
 featureMaxList.push_back(saturation_max);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(saturation);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraRaspi::setCamera> get saturation feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraRaspi::setCamera> value " << saturation << "(min "
             << saturation_min << " max " << saturation_max  << ")";

 // VideoStabilization feature
 camera->setVideoStabilization(true);
 videostabilization_num = 1;
 int videostabilization_min = 0, videostabilization_max = 1;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raspi_features[featureCnt]);
 featureValueList.push_back(videostabilization_num);
 featureMinList.push_back(videostabilization_min);
 featureMaxList.push_back(videostabilization_max);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(videostabilization_num);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraRaspi::setCamera> get videostabilization feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraRaspi::setCamera> value " << videostabilization_num << "(min "
             << videostabilization_min << " max " << videostabilization_max  << ")";
 

 // Exposure Compensation feature
 camera->setExposureCompensation(0);
 exposure_compensation = 0;
 int exposure_compensation_min = -10, exposure_compensation_max = 10;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raspi_features[featureCnt]);
 featureValueList.push_back(exposure_compensation);
 featureMinList.push_back(exposure_compensation_min);
 featureMaxList.push_back(exposure_compensation_max);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(exposure_compensation);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraRaspi::setCamera> get exposure_compensation feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraRaspi::setCamera> value " << exposure_compensation << "(min "
             << exposure_compensation_min << " max " << exposure_compensation_max  << ")";

 // Shutter Speed feature
 int shutterspeed_min = 0, shutterspeed_max = 330000;
 shutterspeed = 0;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raspi_features[featureCnt]);
 featureValueList.push_back(shutterspeed);
 featureMinList.push_back(shutterspeed_min);
 featureMaxList.push_back(shutterspeed_max);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back((int)shutterspeed);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraRaspi::setCamera> get shutterspeed feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraRaspi::setCamera> value " << shutterspeed << "(min "
             << shutterspeed_min << " max " << shutterspeed_max  << ")";


 // Frame Rate feature
 frate = camera->getFrameRate();
 int framerate_min = 0, framerate_max = 120;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raspi_features[featureCnt]);
 featureValueList.push_back(frate);
 featureMinList.push_back(framerate_min);
 featureMaxList.push_back(framerate_max);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(frate);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraRaspi::setCamera> get framerate feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraRaspi::setCamera> value " << frate << "(min "
             << framerate_min << " max " << framerate_max  << ")";

 // Resolution feature
 camera->setWidth(1440);
 camera->setHeight(960);
 width = camera->getWidth();
 height = camera->getHeight();
 resolution_num = 1;
 int resolution_min = 0, resolution_max = AOI_NUMBER - 1;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raspi_features[featureCnt]);
 featureValueList.push_back(resolution_num);
 featureMinList.push_back(resolution_min);
 featureMaxList.push_back(resolution_max);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(resolution_num);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraRaspi::setCamera> get resolution feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraRaspi::setCamera> value " << resolution_num << "(min "
             << resolution_min << " max " << resolution_max  << ")";

 // Encoding feature
 encoding_num = camera->getFormat();
 int encoding_min = 0, encoding_max = ENCODING_NUMBER - 1;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raspi_features[featureCnt]);
 featureValueList.push_back(encoding_num);
 featureMinList.push_back(encoding_min);
 featureMaxList.push_back(encoding_max);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(encoding_num);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraRaspi::setCamera> get encoding feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraRaspi::setCamera> value " << encoding_num << "(min "
             << encoding_min << " max " << encoding_max  << ")";

 // Properties 
 int propCnt = -1;

 // Exposure prop
 QString exposureStr = raspi_props[++propCnt];
 exposure_num = camera->getExposure();
 QLOG_INFO () << "CameraRaspi::setCamera> exposure "
              << raspi_exposure[exposure_num];
 exposureStr.append(" : " + QString(raspi_exposure[exposure_num]));
 propList.push_back(exposureStr);

 // ISO prop
 QString isoStr = raspi_props[++propCnt];
 iso = camera->getISO();
 isoStr.append(" : " + QString::number(iso));
 propList.push_back(isoStr);

 // AWB prop
 QString awbStr = raspi_props[++propCnt];
 awb_num = camera->getAWB();
 QLOG_INFO () << "CameraRaspi::setCamera> awb "
              << raspi_exposure[awb_num];
 awbStr.append(" : " + QString(raspi_awb[awb_num]));
 propList.push_back(awbStr);

 // Sharpness prop
 QString sharpnessStr = raspi_props[++propCnt];
 sharpness = camera->getSharpness();
 sharpnessStr.append(" : " + QString::number(sharpness));
 propList.push_back(sharpnessStr);

 // Brightness prop
 QString brightnessStr = raspi_props[++propCnt];
 brightness = camera->getBrightness();
 brightnessStr.append(" : " + QString::number(brightness));
 propList.push_back(brightnessStr);

 // Saturation prop
 QString saturationStr = raspi_props[++propCnt];
 saturation = camera->getSaturation();
 saturationStr.append(" : " + QString::number(saturation));
 propList.push_back(saturationStr);
  
 // VideoStabilization prop
 QString videostabilizationStr = raspi_props[++propCnt];
 saturationStr.append(" : " + QString::number(videostabilization_num));
 propList.push_back(videostabilizationStr);

 // Exposure Compensation prop
 QString exposure_compensationStr = raspi_props[++propCnt];
 exposure_compensationStr.append(" : " + QString::number(exposure_compensation));
 propList.push_back(exposure_compensationStr);

 // ShutterSpeed prop
 QString shutterspeedStr = raspi_props[++propCnt];
 shutterspeedStr.append(" : " + QString::number(shutterspeed));
 propList.push_back(shutterspeedStr);

 // Resolution prop
 QString resolutionStr = raspi_props[++propCnt];
 width = camera->getWidth();
 height = camera->getHeight();
 switch ((int)height) {
   case 320: resolution_num = 0;
    break;
   case 640: resolution_num = 1;
    break;
   case 960: resolution_num = 2;
    break;
   case 1280: resolution_num = 3;
    break;
   case 1600: resolution_num = 4;
    break;
   case 1920: resolution_num = 5;
    break;
 }
 resolutionStr.append(" : " + QString::number(raspi_aoi_settings[resolution_num][0]) + "x" + 
                              QString::number(raspi_aoi_settings[resolution_num][1]));
 propList.push_back(resolutionStr);

 // Encoding
 QString encodingStr = raspi_props[++propCnt];;
 encoding_num = camera->getFormat();
 encodingStr.append(" : " + QString(raspi_encodings[encoding_num]));
 propList.push_back(encodingStr);

 // Frame Rate
 QString frateStr = raspi_props[++propCnt];;
 frate = camera->getFrameRate();
 frateStr.append(" : " + QString::number((int)frate));
 frateStr.append(" Hz");
 propList.push_back(frateStr);

 // PC Rate
 QString freqStr = raspi_props[++propCnt];;
 freqStr.append(" : " + QString::number((int)frequency));
 freqStr.append(" Hz");
 propList.push_back(freqStr);

 id = _id;
 
 camera_err = connectCamera();

 QLOG_DEBUG() << "CameraRaspi::setCamera " << vendor << " model : "
              << model << " - Err : " << camera_err;
}
uchar* 
CameraRaspi::getSnapshot() {
  snapshotMutex->lock();
  QLOG_INFO() << "CameraRaspi::getSnapshot> Image pixel size " << width * height;
  memcpy(snapshot,buffer, width * height * sizeof(uchar));
  snapShotMin = min;
  snapShotMax = max;
  snapshotMutex->unlock();
  return snapshot;
}
ushort*
CameraRaspi::getSnapshot16() {
  snapshotMutex->lock();
  QLOG_DEBUG() << "CameraRaspi::getSnapshot> Image pixel size " << width * height;
  memcpy(snapshot16,buffer16, width * height * sizeof(ushort));
  snapShotMin = min;
  snapShotMax = max;
  snapshotMutex->unlock();
  return snapshot16;
}

int*
CameraRaspi::getSnapshot32() {
  snapshotMutex->lock();
  QLOG_DEBUG() << "CameraRaspi::getSnapshot> Image pixel size " << width * height;
  memcpy(snapshot32,buffer32, width * height * sizeof(int));
  snapShotMin = min;
  snapShotMax = max;
  snapshotMutex->unlock();
  return snapshot32;
}
void 
CameraRaspi::stop() {
  suspend = true;
  has_started = false;
  wait();
  exit();  
}

void 
CameraRaspi::run() {
  int acq_err = 0;
  int acq_cnt = 0;
  if (camera_err == 0 && suspend == true ) {
    suspend = false;
    eTimeTotal = 0;
    while (suspend == false) {
      QLOG_DEBUG () << "CameraRaspi::run> " << id << " : start new Acquisition";
      double eTime = Utils::GetTimeMicroseconds();
      acqstart->wakeAll();
      acq_err = acquireImage();
      QLOG_DEBUG () << "CameraRaspi::run> " << id << " : done";
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
       QLOG_DEBUG() << "CameraRaspi::run> Acquisition " 
                   << "freq " << (int) frequency << " Hz";
       eTimeTotal = 0;
       acq_cnt = 0;
       // Update props
       getProps();
      }
      has_started = true;
    }
    QLOG_DEBUG() << "CameraRaspi thread exiting";
  }
}

void
CameraRaspi::setFeature(int feature, double value) {
 
  QLOG_INFO() << " FEATURE NUMBER " << feature << " value requested " << (int)value;
  // Stop Acquisition
  this->stop();
  //acquireMutex->lock();
  QLOG_INFO () << "CameraRaspi::setFeature> AcquisitionStop...";
   switch ( feature ) {
   case 0:
   QLOG_INFO() << "CameraRaspi::setFeature> Update feature " << QString(raspi_features[0])
	       << " value " << QString::number(value);
   camera->setExposure((RASPICAM_EXPOSURE)value);
   break;
   case 1:
   QLOG_INFO() << "CameraRaspi::setFeature> Update feature " << QString(raspi_features[1])
               << "value " << (int)value;
   camera->setISO((int)value);
   break;
   case 2:
   QLOG_INFO() << "CameraRaspi::setFeature> Update feature " << QString(raspi_features[2])
               << " value " << (int)value;
   camera->setAWB((RASPICAM_AWB)value);
   break;
   case 3:
   QLOG_INFO() << "CameraRaspi::setFeature> Update feature " << QString(raspi_features[3])
               << " value " << (int)value;
   camera->setSharpness((int)value);
   break;
   case 4:
   QLOG_INFO() << "CameraRaspi::setFeature> Update feature " << QString(raspi_features[4])
               << " value " << (int)value;
   camera->setBrightness((int)value);
   break;
   case 5:
   QLOG_INFO() << "CameraRaspi::setFeature> Update feature " << QString(raspi_features[5])
               << " value " << (int)value;
   camera->setSaturation((int)value);
   break;
   case 6:
   QLOG_INFO() << "CameraRaspi::setFeature> Update feature " << QString(raspi_features[6])
               << " value " << (int)value;
   switch ((int)value) {
   case 0: camera->setVideoStabilization(false);
           videostabilization_num = 0;
     break;
   case 1: camera->setVideoStabilization(true);
           videostabilization_num = 1;
     break;
   }
   break;
   case 7:
   QLOG_INFO() << "CameraRaspi::setFeature> Update feature " << QString(raspi_features[7])
               << " value " << (int)value;
   camera->setExposureCompensation((int)value);
   exposure_compensation = (int) value;
   break;
   case 8:
   QLOG_INFO() << "CameraRaspi::setFeature> Update feature " << QString(raspi_features[8])
               << " value " << (int)value;
   camera->setShutterSpeed((unsigned int)(value));
   shutterspeed = value;
   break;
   case 9:
   QLOG_INFO() << "CameraRaspi::setFeature> Update feature " << QString(raspi_features[9])
               << " value " << (int)value;
   camera->setFrameRate((int)value);
   break;
   case 10:
   QLOG_INFO() << "CameraRaspi::setFeature> Update feature " << QString(raspi_features[10])
               << " value " << (int)value;
   camera->release();
   camera->setWidth(raspi_aoi_settings[(int)value][0]);
   camera->setHeight(raspi_aoi_settings[(int)value][1]);
   width = camera->getWidth();
   height = camera->getHeight(); 
   QLOG_INFO() << " NEW WIDTH = " << width;
   QLOG_INFO() << " NEW HEIGHT = " << height;
   // Allocation data buffer
   if (buffer) { free(buffer); buffer = nullptr;}
   if (snapshot) { free(snapshot); snapshot = nullptr;}
   if (buffer32) { free(buffer32); buffer32 = nullptr;}
   if (snapshot32) { free(snapshot32); snapshot32 = nullptr;}
   if (data) { free(data); data = nullptr;}
   if (image) delete image;
    // Init Format to B8 pixel depth
   camera->setFormat(RASPICAM_FORMAT_GRAY);
   pixel_encoding = B8;
   buffer = (uchar*)malloc( sizeof(uchar) * width * height);
   snapshot = (uchar*)malloc( sizeof(uchar) * width * height);
   buffer32 = (int*)malloc( sizeof(int) * width * height);
   snapshot32 = (int*)malloc( sizeof(int) * width * height);
   image = new QImage(buffer,width,height,width,QImage::Format_Indexed8);
   image->setColorTable(*table);
   data = new unsigned char[camera->getImageBufferSize( )];
   camera->open();
   break;
   case 11:
   QLOG_INFO() << "CameraRaspi::setFeature> Update feature " << QString(raspi_features[11])
               << " value " << (int)value;
   /*camera->release();
   camera->setFormat((RASPICAM_FORMAT)value);
   camera->open();*/
   break;
   }
   //acquireMutex->unlock();
   this->start();
   getFeatures();
   getProps();
}
void
CameraRaspi::setMode(int feature, bool value) {
  switch ( feature ) {
   default:
   break;
 }
}

void
CameraRaspi::getProps() {
/*-----------------------------------------------------------------------
 *  report camera's properties
 *-----------------------------------------------------------------------*/
  // Properties
 int propCnt = -1;

 // Exposure prop
 QString exposureStr = raspi_props[++propCnt];
 exposure_num = camera->getExposure();
 exposureStr.append(" : " + QString(raspi_exposure[exposure_num]));
 propList.replace(propCnt, exposureStr);

 // ISO prop
 QString isoStr = raspi_props[++propCnt];
 iso = camera->getISO();
 isoStr.append(" : " + QString::number(iso));
 propList.replace(propCnt, isoStr);

 // AWB prop
 QString awbStr = raspi_props[++propCnt];
 awb_num = camera->getAWB();
 awbStr.append(" : " + QString(raspi_awb[awb_num]));
 propList.replace(propCnt, awbStr);

 // Sharpness prop
 QString sharpnessStr = raspi_props[++propCnt];
 sharpness = camera->getSharpness();
 sharpnessStr.append(" : " + QString::number(sharpness));
 propList.replace(propCnt, sharpnessStr);

 // Brightness prop
 QString brightnessStr = raspi_props[++propCnt];
 brightness = camera->getBrightness();
 brightnessStr.append(" : " + QString::number(brightness));
 propList.replace(propCnt, brightnessStr);

 // Saturation prop
 QString saturationStr = raspi_props[++propCnt];
 saturation = camera->getSaturation();
 saturationStr.append(" : " + QString::number(saturation));
 propList.replace(propCnt, saturationStr);

 // VideoStabilization prop
 QString videostabilizationStr = raspi_props[++propCnt];
 videostabilizationStr.append(" : " + QString::number(videostabilization_num));
 propList.replace(propCnt, videostabilizationStr);

 // Exposure Compensation prop
 QString exposure_compensationStr = raspi_props[++propCnt];
 exposure_compensationStr.append(" : " + QString::number(exposure_compensation));
 propList.replace(propCnt, exposure_compensationStr);

 // ShutterSpeed prop
 QString shutterspeedStr = raspi_props[++propCnt];
 shutterspeedStr.append(" : " + QString::number(shutterspeed));
 propList.replace(propCnt, shutterspeedStr);

 // Resolution prop
 QString resolutionStr = raspi_props[++propCnt];
 width = camera->getWidth();
 height = camera->getHeight();
 switch ((int)height) {
   case 320: resolution_num = 0;
    break;
   case 640: resolution_num = 1;
    break;
   case 960: resolution_num = 2;
    break;
   case 1280: resolution_num = 3;
    break;
   case 1600: resolution_num = 4;
    break;
   case 1920: resolution_num = 5;
    break;
 }
 resolutionStr.append(" : " + QString::number(raspi_aoi_settings[resolution_num][0]) + "x" +
                              QString::number(raspi_aoi_settings[resolution_num][1]));
 propList.replace(propCnt, resolutionStr);

 // Encoding
 QString encodingStr = raspi_props[++propCnt];
 encoding_num = camera->getFormat();
 encodingStr.append(" : " + QString(raspi_encodings[encoding_num]));
 propList.replace(propCnt, encodingStr);

 // Frame Rate
 QString frateStr = raspi_props[++propCnt];
 frate = camera->getFrameRate();
 frateStr.append(" : " + QString::number((int)frate));
 frateStr.append(" Hz");
 propList.replace(propCnt, frateStr);

 // PC Rate
 QString freqStr = raspi_props[++propCnt];
 //QLOG_INFO() << QString::number((int)frequency);
 freqStr.append(" : " + QString::number((int)frequency));
 freqStr.append(" Hz");
 propList.replace(propCnt, freqStr);
 
 QLOG_DEBUG() << "CameraRaspi::getProps> Properties updated";
 emit updateProps();
}

void
CameraRaspi::getFeatures() {
  /*-----------------------------------------------------------------------
   *  report camera's features
   *-----------------------------------------------------------------------*/
  // Exposure feature
  int featureCnt = -1;
  exposure_num = camera->getExposure();
  featureValueList.replace(++featureCnt, exposure_num);
  featureAbsValueList.replace(featureCnt, exposure_num);

  // ISO feature
  iso = camera->getISO();
  featureValueList.replace(++featureCnt, iso);
  featureAbsValueList.replace(featureCnt, iso);

  // AWB feature
  awb_num = camera->getAWB();
  featureValueList.replace(++featureCnt, awb_num);
  featureAbsValueList.replace(featureCnt, awb_num);

  // Sharpness feature
  sharpness = camera->getSharpness();
  featureValueList.replace(++featureCnt, sharpness);
  featureAbsValueList.replace(featureCnt, sharpness);

  // Brightness feature
  brightness = camera->getBrightness();
  featureValueList.replace(++featureCnt, brightness);
  featureAbsValueList.replace(featureCnt, brightness);
  
  // Saturation feature
  saturation = camera->getSaturation();
  featureValueList.replace(++featureCnt, saturation);
  featureAbsValueList.replace(featureCnt, saturation);

  // Video Stabilization feature
  featureValueList.replace(++featureCnt, videostabilization_num);
  featureAbsValueList.replace(featureCnt, videostabilization_num);

  // Exposure Compensation feature
  featureValueList.replace(++featureCnt, exposure_compensation);
  featureAbsValueList.replace(featureCnt, exposure_compensation);

  // Shutter Speed feature
  featureValueList.replace(++featureCnt, shutterspeed);
  featureAbsValueList.replace(featureCnt, shutterspeed);

  // Frame Rate feature
  frate = camera->getFrameRate();
  featureValueList.replace(++featureCnt, frate);
  featureAbsValueList.replace(featureCnt, frate);

  // Resolution feature
  width = camera->getWidth();
  height = camera->getHeight();
  switch ((int)height) {
    case 320: resolution_num = 0;
      break;
    case 640: resolution_num = 1;
      break;
    case 960: resolution_num = 2;
      break;
    case 1280: resolution_num = 3;
      break;
    case 1600: resolution_num = 4;
      break;
    case 1920: resolution_num = 5;
      break;
    }
    featureValueList.replace(++featureCnt, resolution_num);
    featureAbsValueList.replace(featureCnt, resolution_num);

  // Encoding feature
  encoding_num = camera->getFormat();
  featureValueList.replace(++featureCnt, encoding_num);
  featureAbsValueList.replace(featureCnt, encoding_num);

  emit updateFeatures();
}

int 
CameraRaspi::findCamera() {
  int i;
  /* Find available Raspi cameras*/
  // Allocate camera list
  // Open Camera with id = 0;
  num = 1; // set num = 1
  RaspiCam *camera = new RaspiCam();
  id = 1;
  QLOG_INFO() << "CameraRaspi::findCamera> opening camera " << id;

  if ( !camera->open() ) {
    QLOG_ERROR() << "Error opening RASPI camera";
    return -1;
  }
  camera->release();
  QLOG_INFO() << "CameraRaspi::findCamera> camera " << QString::fromStdString(camera->getId()) << " open";
  cameralist.push_back(camera);
  vendorlist.push_back("Raspberry");
  modelist.push_back("V2");
  return (0);
}
int 
CameraRaspi::connectCamera() {
  
  camera_err = -1;

  /* Init camera*/
  QLOG_INFO() << "CameraRaspi::connectCamera:: Start connecting camera...";

  /*-----------------------------------------------------------------------
   *  setup capture
   *-----------------------------------------------------------------------*/
  height = camera->getHeight();
  width  = camera->getWidth();
  buffer = (uchar*)malloc( sizeof(uchar) * width * height);
  snapshot = (uchar*)malloc( sizeof(uchar) * width * height);
  buffer32 = (int*)malloc( sizeof(int) * width * height);
  snapshot32 = (int*)malloc( sizeof(int) * width * height);

  image = new QImage(buffer,width,height,width,QImage::Format_Indexed8);
  image->setColorTable(*table);

  // Init Format to B8 pixel depth
  camera->setFormat(RASPICAM_FORMAT_GRAY);
  pixel_encoding = B8;
  // Allocation data buffer
  data=new unsigned char[camera->getImageBufferSize( )];
  camera->open();

  // Refresh properties
  getProps();
  // Refresh features
  getFeatures();

  return 0;
}

void  
CameraRaspi::cleanup_and_exit()
{
  QLOG_DEBUG() << "Cleanup camera Raspi connection";
  camera->release();
  if (data) { free(data); data = nullptr;}
  if (buffer) { free(buffer); buffer = nullptr;}
  if (snapshot) { free(snapshot); snapshot = nullptr;}
  if (buffer32) { free(buffer32); buffer32 = nullptr;}
  if (snapshot32) { free(snapshot32); snapshot32 = nullptr;}
  if (image) delete image;
  return;
}

int 
CameraRaspi::acquireImage() {
   
  int acq_err = 1;
  QLOG_DEBUG () << " Cycle start ";
  acquireMutex->lock();
  /*-----------------------------------------------------------------------
   * acquire frame
   *-----------------------------------------------------------------------*/
  double eTime = Utils::GetTimeMicroseconds();
  camera->grab();
  camera->retrieve ( data );
  if (data == nullptr) {
    QLOG_ERROR() << "ACQUISITION ERROR";
    return (-1);
  }
  // Lock the acquisition
  snapshotMutex->lock();
  //QLOG_INFO() << width << " " << height;
  //QLOG_INFO() << "buffersize=" << camera->getImageBufferSize();
    //for (int i = 0 ; i < 100; i++) 
    //  QLOG_INFO() << "data: " << data[i];
    // calculate min,max
    max = 0;
    min = 65535;
    //QLOG_INFO() << "pixel encoding = " << pixel_encoding;
    if ( pixel_encoding == B8 ) {
      for (unsigned int i = 0 ; i < width * height; i++) { 
        if (data[i] > max)
          max = data[i];
        if (data[i] < min)
          min = data[i];
      }
    }
    //QLOG_INFO() << "min = " << min << " max = " << max;
    // copy image to buffer (8-bit : 0 - 255 range) and to buffer32 (32-bit)
    // Reset buffer (8-bit) and buffer32 (32-bit)
    memset(buffer,0,height * width * sizeof(uchar));
    memset(buffer32,0,height * width * sizeof(int));

    if ( pixel_encoding == B8 ) {
      memcpy(buffer,data,width * height * sizeof(uchar));
      for (unsigned int i = 0 ; i < width * height; i++)
        buffer32[i] = buffer[i];
    }

    if (vflip) {
      buffer = reversebytes(buffer,height * width);
      buffer = fliphorizontal(buffer,height * width, width);
      buffer32 = reversebytes(buffer32,height * width);
      buffer32 = fliphorizontal(buffer32,height * width, width);
    }
    if (!hflip) {
      buffer = fliphorizontal(buffer,height*width, width);
      buffer32 = fliphorizontal(buffer32,height*width, width);
    }
    snapshotMutex->unlock();
    // Format video image
    QLOG_DEBUG() << "size IMAGE = " << imageWidth << " " << imageHeight;
    QImage imagescaled = image->scaled(imageWidth,imageHeight);
    QImage imagergb32 =  imagescaled.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    emit getImage(imagergb32);
    emit updateMin(min);
    emit updateMax(max);
    eTime = Utils::GetTimeMicroseconds() - eTime;
    QLOG_DEBUG () << " Process eTime " << QString::number(eTime);
    QLOG_DEBUG () << " Cycle end ";
    acquireMutex->unlock();
  return (acq_err);
}

void CameraRaspi::setImageSize(const int &_imageWidth, const int &_imageHeight){
  imageWidth = _imageWidth;
  imageHeight = _imageHeight;
  QLOG_DEBUG ( ) <<  "emit new image with size " << imageWidth  << "x" << imageHeight;
}

#endif
