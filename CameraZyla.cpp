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
#ifdef ZYLACAMERA
#include "CameraZyla.h"

#define NUMBER_OF_BUFFERS 1
#define GAIN_NUMBER 3
#define TRIGGER_NUMBER 7
#define ELECTRONICSHUTTERING_NUMBER 2
#define ENCODING_NUMBER 3
#define AOI_NUMBER 8
#define READOUTRATE_NUMBER 4
#define TIMEOUT_MS 2100

#define EXTRACTLOWPACKED(SourcePtr) ( (SourcePtr[0] << 4) + (SourcePtr[1] & 0xF) )
#define EXTRACTHIGHPACKED(SourcePtr) ( (SourcePtr[2] << 4) + (SourcePtr[1] >> 4) )

char *zyla_features[]  = {
                        (char*)"Exposure",
                        (char*)"ROI",
                        (char*)"Gain",
                        (char*)"Frame Rate",
                        (char*)"Readout Rate",
			(char*)"Trigger",
                        (char*)"Encoding",
			(char*)"Electronic Shuttering"
                        };
char *zyla_props[]  = {
                        (char*)"Temperature",
                        (char*)"Temperature Status",
                        (char*)"Image Size",
                        (char*)"AOIBinning",
                        (char*)"AOIStride",
                        (char*)"Gain Control",
                        (char*)"Pixel Encoding",
			(char*)"Exposure Time",
			(char*)"Readout Rate",
                        (char*)"Frame Rate",
			(char*)"PC Rate",
			(char*)"Trigger",
			(char*)"Electronic Shuttering"
                        };
char *zyla_encodings[]  = {
                           (char*)"Mono12", 
                           (char*)"Mono12Packed", 
                           (char*)"Mono16", 
                           (char*)"RGB8Packed", 
                           (char*)"Mono12Coded", 
                           (char*)"Mono12CodedPacked", 
                           (char*)"Mono22Parallel", 
                           (char*)"Mono22PackedParallel", 
                           (char*)"Mono8", 
                           (char*)"Mono32"
			};
char *zyla_gains[]  = {
                        (char*)"Gain1",
                        (char*)"Gain2",
                        (char*)"Gain3"
                        };
char *zyla_bin[]  = {
                        (char*)"1x1",
                        (char*)"2x2",
                        (char*)"3x3",
                        (char*)"4x4",
                        (char*)"8x8"
                        };

char *zyla_tempstatus[]  = {
                         (char*)"Cooler Off", 
                         (char*)"Stabilised", 
                         (char*)"Cooling", 
                         (char*)"Drift", 
                         (char*)"Not Stabilised", 
                         (char*)"Fault"
                        };

char *zyla_readout_rates[] = {
                        (char*)"10",
			(char*)"100",
			(char*)"200",
 			(char*)"280"
			};
char *zyla_trigger_modes[]  = {
                        (char*)"Internal",
                        (char*)"External Level Transition",
                        (char*)"External Start",
                        (char*)"External Exposure",
                        (char*)"Software",
                        (char*)"Advanced",
                        (char*)"External"
			};
char *zyla_electronicshuttering_modes[]  = {
                        (char*)"Rolling",
                        (char*)"Global"
                        };
int zyla_aoi_settings[][4] = {
		       	  {2544,25,2160,1},
			  {2064,265,2048,57},
			  {1776,409,1760,201},
			  {1920,337,1080,537},
			  {1392,601,1040,561},
			  {528,1033,512,825},
			  {240,1177,256,953},
 			  {144,1225,128,1017}};

int zyla_aoi_settings_packed[][4] = {
                          {2544,17,2160,1},
                          {2064,257,2048,57},
                          {1776,401,1760,201},
                          {1920,337,1080,537},
                          {1392,593,1040,561},
                          {528,1025,512,825},
                          {240,1169,256,953},
                          {144,1217,128,1017}};

#include <sys/time.h>
/*----------------------------------------------------------------------------*/
/*------------------------------------------------------------------- GetTime */
/*----------------------------------------------------------------------------*/
static double GetTime( void ) {
 struct timeval tp;
/*--------------------------------------------------------------------------*/
 gettimeofday( &tp, NULL );
/*--------------------------------------------------------------------------*/
 return( tp.tv_sec*1e6 + tp.tv_usec );
}

CameraZyla::CameraZyla()
  :Camera()
{
  vflip = 0;
  hflip = 0;
  image = NULL;
  buffer = NULL;
  snapshot = NULL;
  buffer32 = NULL;
  snapshot32 = NULL;
  suspend = true;
  has_started = false;
  mutex = new QMutex(QMutex::NonRecursive);
  snapshotMutex = new QMutex(QMutex::Recursive);
  acquireMutex = new QMutex(QMutex::Recursive);
  acqstart = new QWaitCondition();
  acqend = new QWaitCondition();
  id = 0;
  exposure = 0.01;
  frequency = 0.;
  modeCheckEnabled = false;
}

CameraZyla::~CameraZyla()
{
  QLOG_INFO() << "Deleting CameraZyla";
  stop();
  cleanup_and_exit();
  delete mutex;
  delete snapshotMutex;
  delete acquireMutex;
  delete acqstart;
  delete acqend;
}
void
CameraZyla::setCamera(void* _camera, int _id)
{
 QLOG_DEBUG () << " Getting camera pointer " << _camera;
 camera = (AT_H*)_camera;
 QLOG_DEBUG () << " Getting camera value " << *camera;
 vendor = "AndorZyla";
 AT_WC CameraModel[128];
 char szCamModel[128];
 i_err= AT_GetString(*camera, L"CameraModel", CameraModel, 128);
 if (i_err != AT_SUCCESS) {
   QLOG_ERROR() << "CameraZyla::setCamera> Error from AT_GetString('CameraModel') : " << i_err;
 }
 else {
   wcstombs(szCamModel, CameraModel, 64);
   QLOG_INFO() << "CameraZyla::setCamera> Camera model " << szCamModel;
 }
 model = QString(szCamModel);

 int featureCnt = -1;
 
 // Exposure feature
 double exp_min, exp_max;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(zyla_features[featureCnt]);
 i_err = AT_SetFloat(*camera, L"ExposureTime", exposure);
 errorOk(i_err, "AT_SetFloat 'ExposureTime'");
 i_err = AT_GetFloatMin(*camera, L"ExposureTime", &exp_min);
 i_err = AT_GetFloatMax(*camera, L"ExposureTime", &exp_max);
 exp_max = 2;
 featureMinList.push_back(exp_min);
 featureMaxList.push_back(exp_max);
 i_err = AT_GetFloat(*camera, L"ActualExposureTime", &exposure);
 errorOk(i_err, "AT_GetFloat 'ActualExposureTime'");
 featureValueList.push_back( exposure );
 featureAbsCapableList.push_back(true);
 featureAbsValueList.push_back(exposure);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraZyla::setCamera> get exposure feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraZyla::setCamera> value " << exposure << "(min "
             << exp_min << " max " << exp_max << ")";

 // AOI feature
 AT_64 aoiheight_min, aoiheight_max;
 AT_64 aoiwidth_min, aoiwidth_max;
 i_err = AT_GetIntMin(*camera, L"AOIHeight", &aoiheight_min);
 i_err = AT_GetIntMax(*camera, L"AOIHeight", &aoiheight_max);
 aoi_height = aoiheight_max;
 i_err = AT_GetIntMin(*camera, L"AOIWidth", &aoiwidth_min);
 i_err = AT_GetIntMax(*camera, L"AOIWidth", &aoiwidth_max);
 aoi_width = aoiwidth_max;
 
 i_err = AT_SetInt(*camera, L"AOIWidth", (AT_64)zyla_aoi_settings[0][0]);
 errorOk(i_err, "AT_SetInt 'AOIWidth'");
 i_err = AT_GetInt(*camera, L"AOIWidth", &aoi_width);
 QLOG_INFO() << "CameraZyla::setCamera> SET AOI WIDTH = " << aoi_width;
 
 if (pixel_encoding == B12P)
    i_err = AT_SetInt(*camera, L"AOILeft", (AT_64)zyla_aoi_settings_packed[0][1]);
 else
  i_err = AT_SetInt(*camera, L"AOILeft", (AT_64)zyla_aoi_settings[0][1]);
 errorOk(i_err, "AT_SetInt 'AOILeft'");
 i_err = AT_SetInt(*camera, L"AOIHeight", (AT_64)zyla_aoi_settings[0][2]);
 errorOk(i_err, "AT_SetInt 'AOIHeight'");
 i_err = AT_GetInt(*camera, L"AOIHeight", &aoi_height);
 QLOG_INFO() << "CameraZyla::setCamera> SET AOI HEIGHT = " << aoi_height;
 i_err = AT_SetInt(*camera, L"AOITop", (AT_64)zyla_aoi_settings[0][3]);
 errorOk(i_err, "AT_SetInt 'AOITop'");
 i_err = AT_GetInt(*camera, L"AOITop", &aoi_top);
 
 int aoi_num = 0;
 int aoi_min = 0, aoi_max = AOI_NUMBER - 1;
 if (aoi_top !=  1)
  QLOG_WARN () << "ameraZyla::setCamera> Wrong AOI type !";
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(zyla_features[featureCnt]);
 featureValueList.push_back(aoi_num);
 featureMinList.push_back(aoi_min);
 featureMaxList.push_back(aoi_max);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(aoi_num);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraZyla::setCamera> get aoi feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraZyla::setCamera> value " << aoi_num << "(min "
             << aoi_min << " max " << aoi_max  << ")";

 // Gain feature
 int gain_min = 0, gain_max = GAIN_NUMBER - 1; 
 i_err = AT_GetEnumIndex(*camera, L"SimplePreAmpGainControl", &gain_num);
 errorOk(i_err, "AT_GetEnumIndex 'SimplePreAmpGainControl'");
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(zyla_features[featureCnt]);
 featureValueList.push_back(gain_num);
 featureMinList.push_back(gain_min);
 featureMaxList.push_back(gain_max);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(gain_num);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraZyla::setCamera> get preamp gain feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraZyla::setCamera> value " << gain_num << "(min "
             << gain_min << " max " << gain_max  << ")";

 // Frame Rate feature
 double frate_min, frate_max;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(zyla_features[featureCnt]);
 int i_available = 0;
 AT_IsImplemented(*camera, L"FrameRate", &i_available);
 if (i_available) {
  i_err = AT_GetFloatMin(*camera, L"FrameRate", &frate_min);
  if (errorOk(i_err, "AT_GetFloatMin 'FrameRate'")) {
         QLOG_INFO() << "CameraZyla::setCamera> FrameRateMin = " << frate_min;
  }
  i_err = AT_GetFloatMax(*camera, L"FrameRate", &frate_max);
  if (errorOk(i_err, "AT_GetFloatMax 'FrameRate'")) {
         QLOG_INFO() << "CameraZyla::setCamera> FrameRateMax = " << frate_max;
  }
 }
 featureMinList.push_back(frate_min);
 featureMaxList.push_back(frate_max);
 i_err = AT_GetFloat(*camera, L"FrameRate", &frate);
 errorOk(i_err, "AT_GetFloat 'FrameRate'");
 featureValueList.push_back(frate);
 featureAbsCapableList.push_back(true);
 featureAbsValueList.push_back(frate);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraZyla::setCamera> get frame rate feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraZyla::setCamera> value " << (int) frate << "(min "
             << (int)frate_min << " max " << (int)frate_max << ")";

 // Readout Rate feature
 int rrate_min = 0, rrate_max = READOUTRATE_NUMBER - 1;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(zyla_features[featureCnt]);
 featureMinList.push_back(rrate_min);
 featureMaxList.push_back(rrate_max);
 i_err = AT_GetEnumIndex(*camera, L"PixelReadoutRate", &rrate);
 errorOk(i_err, "AT_GetEnumIndex 'PixelReadoutRate'");
 featureValueList.push_back(rrate);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(rrate);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraZyla::setCamera> get readout rate feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraZyla::setCamera> value " << zyla_readout_rates[rrate]; 

 // Trigger mode feature
 int trigger_min = 0, trigger_max = TRIGGER_NUMBER - 1;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(zyla_features[featureCnt]);
 featureMinList.push_back(trigger_min);
 featureMaxList.push_back(trigger_max);
 i_err = AT_GetEnumIndex(*camera, L"TriggerMode", &trigger_num);
 errorOk(i_err, "AT_GetEnumIndex 'TriggerMode'");
 featureValueList.push_back(trigger_num);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(trigger_num);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraZyla::setCamera> get trigger mode feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraZyla::setCamera> value " << zyla_trigger_modes[trigger_num];

 // Pixel encoding feature
 int encoding_min = 0, encoding_max = ENCODING_NUMBER - 1;
 i_err = AT_SetEnumString(*camera, L"PixelEncoding", L"Mono12" );
 errorOk(i_err, "AT_SetEnumString 'PixelEncoding'");
 i_err = AT_GetEnumIndex(*camera, L"PixelEncoding", &encoding_num );
 errorOk(i_err, "AT_GetEnumIndex 'PixelEncoding'");
   // Set Pixel encoding
   switch (encoding_num) {
    case 0:
    pixel_encoding = B12;
    break;
    case 1:
    pixel_encoding = B12P;
    break;
    case 2:
    pixel_encoding = B16;
    break;
    default:
    pixel_encoding = B8;
    break;
   }
 video_mode = pixel_encoding;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(zyla_features[featureCnt]);
 featureValueList.push_back(encoding_num);
 featureMinList.push_back(encoding_min);
 featureMaxList.push_back(encoding_max);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(encoding_num);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraZyla::setCamera> get encoding feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraZyla::setCamera> value " << (int) encoding_num << "(min "
             << encoding_min << " max " << encoding_max << ")";

 // ElectronicShuttering mode feature
 int acq_min = 0, acq_max = ELECTRONICSHUTTERING_NUMBER - 1;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(zyla_features[featureCnt]);
 featureMinList.push_back(acq_min);
 featureMaxList.push_back(acq_max);
 i_err = AT_GetEnumIndex(*camera, L"ElectronicShutteringMode", &acq_num);
 errorOk(i_err, "AT_GetEnumIndex 'ElectronicShutteringMode'");
 featureValueList.push_back(acq_num);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(acq_num);
 featureModeAutoList.push_back(false);
 QLOG_INFO() << "CameraNeo::setCamera> get electronic shutter mode feature "
             << featureNameList.at(featureCnt);
 QLOG_INFO() << "CameraNeo::setCamera> value " << zyla_electronicshuttering_modes[acq_num];

 i_err = AT_GetInt(*camera, L"ImageSizeBytes", &BufferSize);
 errorOk(i_err, "AT_GetInt 'ImageSizeBytes'");
 QLOG_INFO () << "CameraZyla::setCamera> size for image " 
              << BufferSize;
 // Properties 
 int propCnt = 0;
 // Temperature prop
 QString tempStr = zyla_props[propCnt];
 i_err = AT_GetFloat(*camera, L"SensorTemperature", &temp);
 errorOk(i_err, "AT_GetFloat 'SensorTemperature'");
 QLOG_INFO () << "CameraZyla::setCamera> sensor temperature "
              << QString::number(temp);
 tempStr.append(" : " + QString::number(temp));
 tempStr.append(" degrees");
 propList.push_back(tempStr);

 // Temperature Status prop
 int tempstatus_num;
 QString tempstatStr = zyla_props[++propCnt];
 i_err = AT_GetEnumIndex(*camera, L"TemperatureStatus", &tempstatus_num);
 errorOk(i_err, "AT_GetEnumIndex 'TemperatureStatus'");
 tempstatStr.append(" : " + QString(zyla_tempstatus[tempstatus_num]));
 propList.push_back(tempstatStr);

 // Image Size prop
 QString imgSizeStr = zyla_props[++propCnt];;
 i_err = AT_GetInt(*camera, L"AOIWidth", &aoi_width);
 errorOk(i_err, "AT_GetInt 'AOIWidth'");
 i_err = AT_GetInt(*camera, L"AOIHeight", &aoi_height);
 errorOk(i_err, "AT_GetInt 'AOIHeight'");
 imgSizeStr.append(" : " + QString::number(aoi_width) + "x" + QString::number(aoi_height));
 imgSizeStr.append(" pixels");
 propList.push_back(imgSizeStr);

 // AOIBinning prop
 QString binStr = zyla_props[++propCnt];
 i_err = AT_GetEnumIndex(*camera, L"AOIBinning", &bin_num);
 errorOk(i_err, "AT_GetEnumIndex 'AOIBinning'");
 binStr.append(" : " + QString(zyla_bin[bin_num]));
 propList.push_back(binStr);

 // AOIStride prop
 QString strideStr = zyla_props[++propCnt];
 i_err = AT_GetInt(*camera, L"AOIStride", &stride);
 errorOk(i_err, "AT_GetInt 'AOIStride'");
 strideStr.append(" : " + QString::number(stride));
 strideStr.append(" bytes");
 propList.push_back(strideStr);

 // Pre Amp Gain control
 QString gainStr = zyla_props[++propCnt];
 i_err = AT_GetEnumIndex(*camera, L"SimplePreAmpGainControl", &gain_num);
 errorOk(i_err, "AT_GetEnumIndex 'SimplePreAmpGainControl'");
 gainStr.append(" : " + QString(zyla_gains[gain_num]));
 propList.push_back(gainStr);

 // Encoding prop
 QString encStr = zyla_props[++propCnt];;
 i_err = AT_GetEnumIndex(*camera, L"PixelEncoding", &encoding_num);
 errorOk(i_err, "AT_GetEnumIndex 'PixelEncoding'");
 encStr.append(" : " + QString(zyla_encodings[encoding_num]));
 propList.push_back(encStr);

 // Exposure
 QString expStr = zyla_props[++propCnt];;
 i_err = AT_GetFloat(*camera, L"ActualExposureTime", &exposure);
 errorOk(i_err, "AT_GetFloat 'ActualExposureTime'");
 expStr.append(" : " + QString::number(exposure * 1000.0));
 expStr.append(" ms");
 propList.push_back(expStr);

 // Readout Rate
 QString rrateStr = zyla_props[++propCnt];;
 i_err = AT_GetEnumIndex(*camera, L"PixelReadoutRate", &rrate);
 errorOk(i_err, "AT_GetEnumIndex 'PixelReadoutRate'");
 rrateStr.append(" : " + QString(zyla_readout_rates[rrate]));
 rrateStr.append(" MHz");
 propList.push_back(rrateStr);

 // Frame Rate
 QString frateStr = zyla_props[++propCnt];;
 i_err = AT_GetFloat(*camera, L"FrameRate", &frate);
 errorOk(i_err, "AT_GetFloat 'FrameRate'");
 frateStr.append(" : " + QString::number((int)frate));
 frateStr.append(" Hz");
 propList.push_back(frateStr);

 // PC Rate
 QString freqStr = zyla_props[++propCnt];;
 freqStr.append(" : " + QString::number(frequency));
 freqStr.append(" Hz");
 propList.push_back(freqStr);

 // Trigger Mode
 QString triggerStr = zyla_props[++propCnt];;
 i_err = AT_GetEnumIndex(*camera, L"TriggerMode", &trigger_num);
 errorOk(i_err, "AT_GetEnumIndex 'TriggerMode'");
 triggerStr.append(" : " + QString(zyla_trigger_modes[trigger_num]));
 propList.push_back(triggerStr);

 // ElectronicShuttering Mode
 QString acqStr = zyla_props[++propCnt];;
 i_err = AT_GetEnumIndex(*camera, L"ElectronicShutteringMode", &acq_num);
 errorOk(i_err, "AT_GetEnumIndex 'ElectronicShutteringMode'");
 acqStr.append(" : " + QString(zyla_electronicshuttering_modes[acq_num]));
 propList.push_back(acqStr);

 id = _id;
 
 camera_err = connectCamera();

 QLOG_DEBUG() << "CameraZyla::setCamera " << vendor << " model : "
              << model << " - Err : " << camera_err;
}
uchar* 
CameraZyla::getSnapshot() {
  snapshotMutex->lock();
  QLOG_DEBUG() << "CameraZyla::getSnapshot> Image pixel size " << width * height;
  memcpy(snapshot,buffer, width * height * sizeof(uchar));
  snapShotMin = min;
  snapShotMax = max;
  snapshotMutex->unlock();
  return snapshot;
}
int*
CameraZyla::getSnapshot32() {
  snapshotMutex->lock();
  QLOG_DEBUG() << "CameraZyla::getSnapshot> Image pixel size " << width * height;
  memcpy(snapshot32,buffer32, width * height * sizeof(int));
  snapShotMin = min;
  snapShotMax = max;
  snapshotMutex->unlock();
  return snapshot32;
}
void 
CameraZyla::stop() {
  suspend = true;
  has_started = false;
  wait();
  exit();  
}

void 
CameraZyla::run() {
  int acq_err = 0;
  int acq_cnt = 0;
  if (camera_err == 0 && suspend == true ) {
    suspend = false;
    eTimeTotal = 0;
    while (suspend == false) {
      QLOG_DEBUG () << "CameraZyla::run> " << id << " : start new Acquisition";
      double eTime = GetTime();
      acqstart->wakeAll();
      acq_err = acquireImage();
      QLOG_DEBUG () << "CameraZyla::run> " << id << " : done";
      acqend->wakeAll();
      if (acq_err == 1) 
       eTimeTotal+= (GetTime() - eTime);
      acq_cnt++;
      if (acq_cnt == FREQUENCY_AVERAGE_COUNT) {
       eTimeTotal/=1e6;
       eTimeTotal/=FREQUENCY_AVERAGE_COUNT;
       if ( eTimeTotal > 0 )
        frequency = 1.0 / eTimeTotal;
       else
        frequency = 0;   
       QLOG_DEBUG() << "CameraZyla::run> Acquisition " 
                   << "freq " << (int) frequency << " Hz";
       eTimeTotal = 0;
       acq_cnt = 0;
       // Update props
       getProps();
      }
      has_started = true;
    }
    QLOG_DEBUG() << "CameraZyla thread exiting";
  }
}

void
CameraZyla::setFeature(int feature, double value) {
 
  // Stop Acquisition
  this->stop();
  //acquireMutex->lock();
  QLOG_INFO () << "CameraZyla::setFeature> AcquisitionStop...";
  i_err = AT_Command(*camera, L"AcquisitionStop");
  errorOk(i_err, "AT_Command 'AcquisitionStop'");
  i_err = AT_Flush(*camera);
  errorOk(i_err, "AT_Flush");
  FrameNumber = 0;

   switch ( feature ) {
   case 0:
   QLOG_INFO() << "CameraZyla::setFeature> Update feature " << QString(zyla_features[0])
	       << " value " << QString::number(value);
   i_err = AT_SetFloat(*camera, L"ExposureTime", value );
   errorOk(i_err, "AT_SetFloat 'ExposureTime'");
   i_err = AT_GetFloat(*camera, L"ActualExposureTime", &exposure);
   errorOk(i_err, "AT_GetFloat 'ActualExposureTime'");
   break;
   case 1:
   QLOG_INFO() << "CameraZyla::setFeature> Update feature " << QString(zyla_features[1])
               << " value " << zyla_aoi_settings[(int)value][0] << "x"
			    << zyla_aoi_settings[(int)value][2];
   i_err = AT_SetInt(*camera, L"AOIWidth", (AT_64)zyla_aoi_settings[(int)value][0]);
   errorOk(i_err, "AT_SetInt 'AOIWidth'");
   if (pixel_encoding == B12P)
    i_err = AT_SetInt(*camera, L"AOILeft", (AT_64)zyla_aoi_settings_packed[(int)value][1]);
   else
    i_err = AT_SetInt(*camera, L"AOILeft", (AT_64)zyla_aoi_settings[(int)value][1]);
   errorOk(i_err, "AT_SetInt 'AOILeft'");
   i_err = AT_SetInt(*camera, L"AOIHeight", (AT_64)zyla_aoi_settings[(int)value][2]);
   errorOk(i_err, "AT_SetInt 'AOIHeight'");
   i_err = AT_SetInt(*camera, L"AOITop", (AT_64)zyla_aoi_settings[(int)value][3]);
   errorOk(i_err, "AT_SetInt 'AOITop'");
   break;
   case 2:
   QLOG_INFO() << "CameraZyla::setFeature> Update feature " << QString(zyla_features[2])
               << " value " << QString(zyla_gains[(int)value]);
   i_err = AT_SetEnumIndex(*camera, L"SimplePreAmpGainControl", (int)value);
   errorOk(i_err, "AT_SetEnumIndex 'AOISimplePreAmpGainControl'");
   break;
   case 3:
   QLOG_INFO() << "CameraZyla::setFeature> Update feature " << QString(zyla_features[3])
               << " value " << value << " Hz";
   i_err = AT_SetFloat(*camera, L"FrameRate", value );
   errorOk(i_err, "AT_SetFloat 'FrameRate'");
   break;
   case 4:
   QLOG_INFO() << "CameraZyla::setFeature> Update feature " << QString(zyla_features[4])
               << " value " << QString(zyla_readout_rates[(int)value]);
   i_err = AT_SetEnumIndex(*camera, L"PixelReadoutRate", (int)value);
   errorOk(i_err, "AT_SetEnumIndex 'PixelReadoutRate'");
   i_err = AT_GetEnumIndex(*camera, L"PixelReadoutRate", &rrate);
   errorOk(i_err, "AT_GetEnumIndex 'PixelReadoutRate'");
   break;
   case 5:
   QLOG_INFO() << "CameraZyla::setFeature> Update feature " << QString(zyla_features[5])
               << " value " << QString(zyla_trigger_modes[(int)value]);
   i_err = AT_SetEnumIndex(*camera, L"TriggerMode", (int)value);
   errorOk(i_err, "AT_SetEnumIndex 'TriggerMode'");
   break;
   case 6:
   QLOG_INFO() << "CameraZyla::setFeature> Update feature " << QString(zyla_features[6])
               << " value " << QString(zyla_encodings[(int)value]);
   i_err = AT_SetEnumIndex(*camera, L"PixelEncoding", (int)value);
   errorOk(i_err, "AT_SetEnumIndex 'PixelEncoding'");
   i_err = AT_GetEnumIndex(*camera, L"PixelEncoding", &encoding_num );
   errorOk(i_err, "AT_GetEnumIndex 'PixelEncoding'");
   // Set Pixel encoding
   switch (encoding_num) {
    case 0:
    pixel_encoding = B12;
    break;
    case 1:
    pixel_encoding = B12P;
    break;
    case 2:
    pixel_encoding = B16;
    break;
    default:
    pixel_encoding = B8;
    break;
   }
   video_mode = pixel_encoding;
   break;
   case 7:
   QLOG_INFO() << "CameraNeo::setFeature> Update feature " << QString(zyla_features[7])
               << " value " << QString(zyla_electronicshuttering_modes[(int)value]);
   if ((int)value == 0)
     i_err = AT_SetEnumString(*camera, L"ElectronicShutteringMode", L"Rolling");
   else if ((int)value == 1) {
     i_err = AT_SetEnumString(*camera, L"ElectronicShutteringMode", L"Global");
     QLOG_INFO() << "CameraNeo::setFeature> Confirm feature " << QString(zyla_features[7])
               << " value " << QString(zyla_electronicshuttering_modes[(int)value]);
   }
   errorOk(i_err, "AT_SetEnumString 'ElectronicShutteringMode'");
   break;
   default:
   break;
 }
   QLOG_INFO () << "CameraZyla::setFeature> Reallocating the image buffer ";
   i_err = AT_GetInt(*camera, L"ImageSizeBytes", &BufferSize);
   errorOk(i_err, "AT_GetInt 'ImageSizeBytes'");
   i_err = AT_GetInt(*camera, L"AOIWidth", &aoi_width);
   errorOk(i_err, "AT_GetInt 'AOIWidth'");
   i_err = AT_GetInt(*camera, L"AOIHeight", &aoi_height);
   errorOk(i_err, "AT_SetInt 'AOIHeight'");
   height = static_cast<int>(aoi_height);
   width  = static_cast<int>(aoi_width);
   QLOG_INFO () << "CameraZyla::setFeature> Updated internal buffer " << BufferSize;
   QLOG_INFO () << "CameraZyla::setFeature> width " << width << " height " << height;
   if (buffer) { free(buffer); buffer = NULL;}
   if (snapshot) { free(snapshot); snapshot = NULL;}
   if (buffer32) { free(buffer32); buffer32 = NULL;}
   if (snapshot32) { free(snapshot32); snapshot32 = NULL;}
   buffer = (uchar*)malloc( sizeof(uchar) * width * height);
   snapshot = (uchar*)malloc( sizeof(uchar) * width * height);
   buffer32 = (int*)malloc( sizeof(int) * width * height);
   snapshot32 = (int*)malloc( sizeof(int) * width * height);
   delete image;
   image = new QImage(buffer,width,height,width,QImage::Format_Indexed8);
   QVector<QRgb> table;
   for (int i = 0; i < 256; i++) table.append(qRgb(i, i, i));
   image->setColorTable(table);
   for (int i = 0; i < NUMBER_OF_BUFFERS; i++) {
     delete AlignedBuffers[i];
     AlignedBuffers[i] = new ushort[BufferSize];
   }
   for(int i = 0; i < NUMBER_OF_BUFFERS; i++) {
    AT_QueueBuffer(*camera, reinterpret_cast<AT_U8*>(AlignedBuffers[i]), BufferSize);
   }
   i_err = AT_Command(*camera, L"AcquisitionStart");
   errorOk(i_err, "AT_Command 'AcquisitionStart'");
   QLOG_INFO () << "CameraZyla::setFeature> AcquisitionStart";
   //acquireMutex->unlock();
   this->start();
   getProps();
}
void
CameraZyla::setMode(int feature, bool value) {
  switch ( feature ) {
   default:
   break;
 }
}

void
CameraZyla::getProps() {
/*-----------------------------------------------------------------------
 *  report camera's properties
 *-----------------------------------------------------------------------*/
  // Properties
 int propCnt = 0;
 // Temperature prop
 QString tempStr = zyla_props[propCnt];
 i_err = AT_GetFloat(*camera, L"SensorTemperature", &temp);
 errorOk(i_err, "AT_GetFloat 'SensorTemperature'");
 tempStr.append(" : " + QString::number(temp));
 tempStr.append(" degrees");
 propList.replace(propCnt, tempStr);

 // Temperature Status prop
 QString tempstatStr = zyla_props[++propCnt];
 int tempstatus_num;
 i_err = AT_GetEnumIndex(*camera, L"TemperatureStatus", &tempstatus_num);
 errorOk(i_err, "AT_GetEnumIndex 'TemperatureStatus'");
 tempstatStr.append(" : " + QString(zyla_tempstatus[tempstatus_num]));
 propList.replace(propCnt,tempstatStr);

 // Image Size prop
 QString imgSizeStr = zyla_props[++propCnt];
 i_err = AT_GetInt(*camera, L"AOIWidth", &aoi_width);
 errorOk(i_err, "AT_GetInt 'AOIWidth'");
 i_err = AT_GetInt(*camera, L"AOIHeight", &aoi_height);
 errorOk(i_err, "AT_GetInt 'AOIHeight'");
 imgSizeStr.append(" : " + QString::number(aoi_width) + "x" + QString::number(aoi_height));
 imgSizeStr.append(" pixels");
 propList.replace(propCnt, imgSizeStr);

 // AOIBinning prop
 QString binStr = zyla_props[++propCnt];
 i_err = AT_GetEnumIndex(*camera, L"AOIBinning", &bin_num);
 errorOk(i_err, "AT_GetEnumIndex 'AOIBinning'");
 binStr.append(" : " + QString(zyla_bin[bin_num]));
 propList.replace(propCnt, binStr);

 // AOIStride prop
 QString strideStr = zyla_props[++propCnt];
 i_err = AT_GetInt(*camera, L"AOIStride", &stride);
 errorOk(i_err, "AT_GetInt 'AOIStride'");
 strideStr.append(" : " + QString::number(stride));
 strideStr.append(" bytes");
 propList.replace(propCnt, strideStr);
 
 // Pre Amp Gain control
 QString gainStr = zyla_props[++propCnt];;
 i_err = AT_GetEnumIndex(*camera, L"SimplePreAmpGainControl", &gain_num);
 errorOk(i_err, "AT_GetEnumIndex 'SimplePreAmpGainControl'");
 gainStr.append(" : " + QString(zyla_gains[gain_num]));
 propList.replace(propCnt, gainStr);

 // Encoding prop
 QString encStr = zyla_props[++propCnt];;
 i_err = AT_GetEnumIndex(*camera, L"PixelEncoding", &encoding_num);
 errorOk(i_err, "AT_GetEnumIndex 'PixelEncoding'");
 encStr.append(" : " + QString(zyla_encodings[encoding_num]));
 propList.replace(propCnt, encStr);

 // Exposure
 QString expStr = zyla_props[++propCnt];;
 i_err = AT_GetFloat(*camera, L"ActualExposureTime", &exposure);
 errorOk(i_err, "AT_GetFloat 'ActualExposureTime'");
 expStr.append(" : " + QString::number((int)(exposure * 1000.0)));
 expStr.append(" ms");
 propList.replace(propCnt, expStr);

 // Readout Rate
 QString rrateStr = zyla_props[++propCnt];;
 i_err = AT_GetEnumIndex(*camera, L"PixelReadoutRate", &rrate);
 errorOk(i_err, "AT_GetEnumIndex 'PixelReadoutRate'");
 rrateStr.append(" : " + QString(zyla_readout_rates[rrate]));
 rrateStr.append(" MHz");
 propList.replace(propCnt, rrateStr);

 // Frame Rate
 QString frateStr = zyla_props[++propCnt];;
 i_err = AT_GetFloat(*camera, L"FrameRate", &frate);
 errorOk(i_err, "AT_SetFloat 'FrameRate'");
 frateStr.append(" : " + QString::number((int)frate));
 frateStr.append(" Hz");
 propList.replace(propCnt, frateStr);

 // PC Rate 
 QString freqStr = zyla_props[++propCnt];;
 freqStr.append(" : " + QString::number((int)frequency));
 freqStr.append(" Hz");
 propList.replace(propCnt, freqStr);

 // Trigger Mode
 QString triggerStr = zyla_props[++propCnt];;
 i_err = AT_GetEnumIndex(*camera, L"TriggerMode", &trigger_num);
 errorOk(i_err, "AT_GetEnumIndex 'TriggerMode'");
 triggerStr.append(" : " + QString(zyla_trigger_modes[trigger_num]));
 propList.replace(propCnt, triggerStr);

 // ElectronicShuttering Mode
 QString acqStr = zyla_props[++propCnt];;
 i_err = AT_GetEnumIndex(*camera, L"ElectronicShutteringMode", &acq_num);
 errorOk(i_err, "AT_GetEnumIndex 'ElectronicShutteringMode'");
 acqStr.append(" : " + QString(zyla_electronicshuttering_modes[acq_num]));
 propList.replace(propCnt, acqStr);

 QLOG_DEBUG() << "CameraZyla::getProps> Properties updated";
 emit updateProps();
}

void
CameraZyla::getFeatures() {
  /*-----------------------------------------------------------------------
   *  report camera's features
   *-----------------------------------------------------------------------*/
  // Exposure Time feature
  int featureCnt = 0;
  i_err = AT_GetFloat(*camera, L"ActualExposureTime", &exposure);
  errorOk(i_err, "AT_GetFloat 'ActualExposureTime'");
  featureValueList.replace(featureCnt, exposure );
  featureAbsValueList.replace(featureCnt,exposure);
  double exp_min, exp_max;
  i_err = AT_GetFloatMin(*camera, L"ExposureTime", &exp_min);
  i_err = AT_GetFloatMax(*camera, L"ExposureTime", &exp_max);
  exp_max = 2.;
  featureMinList.replace(featureCnt, exp_min );
  featureMaxList.replace(featureCnt, exp_max );
  QLOG_INFO() << "CameraZyla::getFeatures> exp_min=" << QString::number(exp_min) 
              << " exp_max=" << QString::number(exp_max);

  // AOI feature
  featureCnt++;
  i_err = AT_GetInt(*camera, L"AOIWidth", &aoi_width);
  errorOk(i_err, "AT_GetInt 'AOIWidth'");
  i_err = AT_GetInt(*camera, L"AOILeft", &aoi_left);
  errorOk(i_err, "AT_SetInt 'AOILeft'");
  i_err = AT_GetInt(*camera, L"AOIHeight", &aoi_height);
  errorOk(i_err, "AT_SetInt 'AOIHeight'");
  i_err = AT_GetInt(*camera, L"AOITop", &aoi_top);
  errorOk(i_err, "AT_SetInt 'AOITop'");

  for (int i = 0 ; i < AOI_NUMBER; i++) {
      if (aoi_width == zyla_aoi_settings[i][0] &&
	  aoi_left == zyla_aoi_settings[i][1]  &&
	  aoi_height == zyla_aoi_settings[i][2] &&
	  aoi_top == zyla_aoi_settings[i][3]) {
          featureValueList.replace(featureCnt, i);
          break;
      }
  }
  
  // Gain feature
  featureCnt++;
  i_err = AT_GetEnumIndex(*camera, L"SimplePreAmpGainControl", &gain_num);
  if (errorOk(i_err, "AT_GetEnumIndex 'SimplePreAmpGainControl'")) 
    featureValueList.replace(featureCnt, gain_num); 

  // Frame Rate feature
  featureCnt++;
  double frate_min, frate_max;
  i_err = AT_GetFloat(*camera, L"FrameRate", &frate);
  errorOk(i_err, "AT_GetFloat 'FrameRate'");
  i_err = AT_GetFloatMin(*camera, L"FrameRate", &frate_min);
  if (errorOk(i_err, "AT_GetFloatMin 'FrameRate'"))
    QLOG_INFO() << "CameraZyla::getFeature> FrameRateMin = " << frate_min;
  i_err = AT_GetFloatMax(*camera, L"FrameRate", &frate_max);
  if (errorOk(i_err, "AT_GetFloatMax 'FrameRate'")) 
    QLOG_INFO() << "CameraZyla::getFeature> FrameRateMax = " << frate_max;
  featureMinList.replace(featureCnt, frate_min );
  featureMaxList.replace(featureCnt, frate_max );
  featureValueList.replace(featureCnt, frate );
  featureAbsValueList.replace(featureCnt,frate);
  QLOG_INFO() << "CameraZyla::getFeature> get frame rate feature "
             << featureNameList.at(featureCnt);
  QLOG_INFO() << "CameraZyla::getFeature> value " << (int)frate << "(min "
             << (int)frate_min << " max " << (int)frate_max << ")";
  
  // Readout Rate feature
  featureCnt++;
  i_err = AT_GetEnumIndex(*camera, L"PixelReadoutRate", &rrate);
  errorOk(i_err, "AT_GetEnumIndex 'PixelReadoutRate'");
  featureValueList.replace(featureCnt,rrate);
  QLOG_INFO() << "CameraZyla::getFeature> get readout rate feature "
              << featureNameList.at(featureCnt);
  QLOG_INFO() << "CameraZyla::getFeature> value " << zyla_readout_rates[rrate];

  // Trigger Mode feature
  featureCnt++;
  i_err = AT_GetEnumIndex(*camera, L"TriggerMode", &trigger_num);
  errorOk(i_err, "AT_GetEnumIndex 'TriggerMode'");
  featureValueList.replace(featureCnt,trigger_num);
  QLOG_INFO() << "CameraZyla::getFeature> get trigger mode feature "
              << featureNameList.at(featureCnt);
  QLOG_INFO() << "CameraZyla::getFeature> value " << zyla_trigger_modes[trigger_num];

  // Pixel Encoding feature
  featureCnt++;
  i_err = AT_GetEnumIndex(*camera, L"PixelEncoding", &encoding_num);
  if (errorOk(i_err, "AT_GetEnumIndex 'PixelEncoding'"))
  featureValueList.replace(featureCnt, encoding_num);
  QLOG_INFO() << "CameraZyla::getFeature> get pixel encoding feature "
              << featureNameList.at(featureCnt);
  QLOG_INFO() << "CameraZyla::getFeature> value " << zyla_encodings[encoding_num];
  // Update Pixel encoding
   switch (encoding_num) {
    case 0:
    pixel_encoding = B12;
    break;
    case 1:
    pixel_encoding = B12P;
    break;
    case 2:
    pixel_encoding = B16;
    break;
    default:
    pixel_encoding = B8;
    break;
   }
   video_mode = pixel_encoding;
 
  emit updateFeatures();
}

int 
CameraZyla::findCamera() {
  int i;
  QLOG_INFO() << "CameraZyla::findCamera> Init Zyla andor Library...";
  i_err = AT_InitialiseLibrary();
  if (!errorOk(i_err, "AT_InitialiseLibrary")) 
    return (-1);
  /* Find available Zyla cameras*/
  AT_64 i64_deviceCount = 0;
  i_err = AT_GetInt(AT_HANDLE_SYSTEM, L"DeviceCount", &i64_deviceCount);
  errorOk(i_err, "AT_GetInt 'DeviceCount'");
  QLOG_INFO() << "CameraZyla::findCamera> Found " << i64_deviceCount << "camera";
  QLOG_INFO() << "CameraZyla::findCamera> Init Zyla andor Library...Done";
  // Allocate camera list
  // Open Camera with id = 0;
  num = i64_deviceCount - 2; // remove the 2 fake cameras
  AT_H *camera;
  camera = (AT_H*)malloc(sizeof(AT_H));
  QLOG_INFO() << "CameraZyla::findCamera> opening camera " << id;
  i_err = AT_Open(id, camera);
  QLOG_INFO() << "CameraZyla::findCamera> camera " << id << " open";
  errorOk(i_err, "AT_Open");
  AT_WC CameraModel[128];
  char szCamModel[128];
  i_err= AT_GetString(*camera, L"CameraModel", CameraModel, 128);
  if (i_err != AT_SUCCESS) {
     QLOG_ERROR() << "CameraZyla::findCamera> Error from AT_GetString('CameraModel') : " << i_err;
  }
  else {
     wcstombs(szCamModel, CameraModel, 64);
     QLOG_INFO() << "CameraZyla::findCamera> Camera model " << szCamModel;
  }
  QLOG_DEBUG () << " Storing camera value " << *camera;
  QLOG_DEBUG () << " Storing camera address " << camera;

  cameralist.push_back(camera);
  vendorlist.push_back("AndorZyla");
  modelist.push_back(QString(szCamModel));
  return (0);
}
int 
CameraZyla::connectCamera() {
  
  camera_err = -1;

  /* Init camera*/
  QLOG_INFO() << "CameraZyla::connectCamera:: Start connecting camera...";
  height = static_cast<int>(aoi_height);
  width  = static_cast<int>(aoi_width);

  // Get Full AOI Control
  AT_BOOL fullaoicontrol;
  i_err = AT_GetBool(*camera, L"FullAOIControl", &fullaoicontrol);
  errorOk(i_err, "AT_GetBool 'FullAOIControl'");
  QLOG_INFO() << "CameraZyla::connectCamera:: FullAOICOntrol is " << fullaoicontrol;

  // Get Stride value
  i_err = AT_GetInt(*camera, L"AOIStride", &stride);
  errorOk(i_err, "AT_GetInt 'AOIStride'");
  QLOG_INFO() << "CameraZyla::connectCamera:: AOIStride is " << stride;

  // Get Binning value
  int bincnt;
  i_err = AT_GetEnumCount(*camera, L"AOIBinning", &bincnt);
  errorOk(i_err, "AT_GetEnumCount 'AOIBinning'");
  for (int i = 0 ; i < bincnt; i++) {
    AT_WC item[128];
    char citem[128];
    i_err = AT_GetEnumStringByIndex(*camera, L"AOIBinning", i, item, 128);
    wcstombs(citem, item, 64);
    QLOG_INFO () << "CameraZyla::connectCamera> AOIBinning " << i
                 << " " << QString(citem);
  }
  i_err = AT_GetEnumIndex(*camera, L"AOIBinning", &bin_num);
  errorOk(i_err, "AT_GetEnumIndex 'AOIBinning'");
  QLOG_INFO () << "CameraZyla::connectCamera> AOIBinning " << zyla_bin[bin_num];

  // Get Temperature Status mode
  int tempcnt;
  i_err = AT_GetEnumCount(*camera, L"TemperatureStatus", &tempcnt);
  errorOk(i_err, "AT_GetEnumCount 'TemperatureStatus'");
  for (int i = 0 ; i < tempcnt; i++) {
    AT_WC item[128];
    char citem[128];
    i_err = AT_GetEnumStringByIndex(*camera, L"TemperatureStatus", i, item, 128);
    wcstombs(citem, item, 64);
    QLOG_INFO () << "CameraZyla::connectCamera> TemperatureStatus " << i
                 << " " << QString(citem);
  }
  int tempstatus_num;
  i_err = AT_GetEnumIndex(*camera, L"TemperatureStatus", &tempstatus_num);
  errorOk(i_err, "AT_GetEnumIndex 'TemperatureStatus'");
  QLOG_INFO () << "CameraZyla::connectCamera> TemperatureStatus " << zyla_tempstatus[tempstatus_num];

  // Set continuous acquisition mode
  int cyclecnt;
  i_err = AT_GetEnumCount(*camera, L"CycleMode", &cyclecnt);
  errorOk(i_err, "AT_GetEnumCount 'CycleMode'");
  for (int i = 0 ; i < cyclecnt; i++) {
     AT_WC item[128];
     char citem[128];
     i_err = AT_GetEnumStringByIndex(*camera, L"CycleMode", i, item, 128);
     wcstombs(citem, item, 64);
     QLOG_INFO () << "CameraZyla::connectCamera> CycleMode " << i
                  << " " << QString(citem);
  }
  i_err = AT_SetEnumString(*camera, L"CycleMode", L"Continuous");
  errorOk(i_err, "AT_SetEnumString 'CycleMode'");
  QLOG_INFO () << "CameraZyla::connectCamera> Set Continuous Cycle Mode";

  // Set SensorCooling On
  AT_BOOL isCool;
  i_err = AT_GetBool(*camera, L"SensorCooling", &isCool);
  errorOk(i_err, "AT_GetBool 'SensorCooling'");
  QLOG_INFO () << "CameraZyla::connectCamera> SensorCooling is currently " << isCool;
  i_err = AT_SetBool(*camera, L"SensorCooling", AT_TRUE);
  errorOk(i_err, "AT_SetBool 'SensorCooling'");
  QLOG_INFO () << "CameraZyla::connectCamera> Set SensorCooling " << AT_TRUE;

  // Set FanSpeed On
  int fancnt;
  i_err = AT_GetEnumCount(*camera, L"FanSpeed", &fancnt);
  errorOk(i_err, "AT_GetEnumCount 'FanSpeed'");
  for (int i = 0 ; i < fancnt; i++) {
     AT_WC item[128];
     char citem[128];
     i_err = AT_GetEnumStringByIndex(*camera, L"FanSpeed", i, item, 128);
     wcstombs(citem, item, 64);
     QLOG_INFO () << "CameraZyla::connectCamera> FanSpeed " << i
                  << " " << QString(citem);
  }
  i_err = AT_SetEnumString(*camera, L"FanSpeed", L"On");
  errorOk(i_err, "AT_SetEnumIndex 'FanSpeed'");
  QLOG_INFO () << "CameraZyla::connectCamera> Set FanSpeed On";

  // Get Pixel encodings
  int pixelencodingcnt;
  i_err = AT_GetEnumCount(*camera, L"PixelEncoding", &pixelencodingcnt);
  errorOk(i_err, "AT_GetEnumCount 'PixelEncoding'");
  for (int i = 0 ; i < pixelencodingcnt; i++) {
     AT_WC item[128];
     char citem[128];
     i_err = AT_GetEnumStringByIndex(*camera, L"PixelEncoding", i, item, 128);
     wcstombs(citem, item, 64);
     QLOG_INFO () << "CameraZyla::connectCamera> PixelEncoding " << i
                  << " " << QString(citem);
  }

  // Set rolling shutter mode
  int shuttercnt;
  i_err = AT_GetEnumCount(*camera, L"ElectronicShutteringMode", &shuttercnt);
  errorOk(i_err, "AT_GetEnumCount 'ElectronicShutteringMode'");
  for (int i = 0 ; i < shuttercnt; i++) {
     AT_WC item[128];
     char citem[128];
     i_err = AT_GetEnumStringByIndex(*camera, L"ElectronicShutteringMode", i, item, 128);
     wcstombs(citem, item, 64);
     QLOG_INFO () << "CameraZyla::connectCamera> ElectronicShutteringMode " << i
                  << " " << QString(citem);
  }
  i_err = AT_SetEnumString(*camera, L"ElectronicShutteringMode", L"Rolling");
  errorOk(i_err, "AT_SetEnumIndex 'ElectronicShutteringMode'");
  QLOG_INFO () << "CameraZyla::connectCamera> Set Rolling Shutter Mode";

  // Set SimplePreAmpGainControl mode
  int gaincnt;
  i_err = AT_GetEnumCount(*camera, L"SimplePreAmpGainControl", &gaincnt);
  errorOk(i_err, "AT_GetEnumCount 'SimplePreAmpGainControl'");
  for (int i = 0 ; i < gaincnt; i++) {
     AT_WC item[128];
     char citem[128];
     i_err = AT_GetEnumStringByIndex(*camera, L"SimplePreAmpGainControl", i, item, 128);
     wcstombs(citem, item, 64);
     QLOG_INFO () << "CameraZyla::connectCamera> SimplePreAmpGainControl " << i
                  << " " << QString(citem);
  }

  // Get PixelEncoding
  int encodingcnt;
  i_err = AT_GetEnumCount(*camera, L"PixelEncoding", &encodingcnt);
  errorOk(i_err, "AT_GetEnumCount 'PixelEncoding'");
  for (int i = 0 ; i < encodingcnt; i++) {
     AT_WC item[128];
     char citem[128];
     i_err = AT_GetEnumStringByIndex(*camera, L"PixelEncoding", i, item, 128);
     wcstombs(citem, item, 64);
     QLOG_INFO () << "CameraZyla::connectCamera> Pixel Encoding " << i
                  << " " << QString(citem);
  }

  // Get Pixel Readout Rate mode
  int rratecnt;
  i_err = AT_GetEnumCount(*camera, L"PixelReadoutRate", &rratecnt);
  errorOk(i_err, "AT_GetEnumCount 'PixelReadoutRate'");
  for (int i = 0 ; i < rratecnt; i++) {
     AT_WC item[128];
     char citem[128];
     i_err = AT_GetEnumStringByIndex(*camera, L"PixelReadoutRate", i, item, 128);
     wcstombs(citem, item, 64);
     QLOG_INFO () << "CameraZyla::connectCamera> Pixel Readout Rate " << i
                  << " " << QString(citem);
  }

  // Set trigger mode Software
  int triggercnt;
  i_err = AT_GetEnumCount(*camera, L"TriggerMode", &triggercnt);
  errorOk(i_err, "AT_GetEnumCount 'TriggerMode'");
  for (int i = 0 ; i < triggercnt; i++) {
     AT_WC item[128];
     char citem[128];
     i_err = AT_GetEnumStringByIndex(*camera, L"TriggerMode", i, item, 128);
     wcstombs(citem, item, 64);
     QLOG_INFO () << "CameraZyla::connectCamera> TriggerMode " << i 
                  << " " << QString(citem);
  }
  i_err = AT_SetEnumString(*camera, L"TriggerMode", L"Software");
  errorOk(i_err, "AT_SetEnumString 'TriggerMode'");
  QLOG_INFO () << "CameraZyla::connectCamera> Set TriggerMode Internal";
  
  // Set spurious noise filter mode
  i_err = AT_SetBool(*camera, L"SpuriousNoiseFilter", AT_TRUE);
  errorOk(i_err, "AT_SetBool 'SpuriousNoiseFilter'");
  QLOG_INFO () << "CameraZyla::connectCamera> Set SpuriousNoiseFilter to TRUE";

  /*-----------------------------------------------------------------------
   *  setup capture
   *-----------------------------------------------------------------------*/
  buffer = (uchar*)malloc( sizeof(uchar) * width * height);
  snapshot = (uchar*)malloc( sizeof(uchar) * width * height);
  buffer32 = (int*)malloc( sizeof(int) * width * height);
  snapshot32 = (int*)malloc( sizeof(int) * width * height);

  image = new QImage(buffer,width,height,width,QImage::Format_Indexed8);
  QVector<QRgb> table;
  for (int i = 0; i < 256; i++) table.append(qRgb(i, i, i));
  image->setColorTable(table);

  //Allocate a number of memory buffers to store frames
  AlignedBuffers = new ushort*[NUMBER_OF_BUFFERS];
  for (int i = 0; i < NUMBER_OF_BUFFERS; i++) {
    AlignedBuffers[i] = new ushort[BufferSize];
  }
  /*-----------------------------------------------------------------------
   *  have the camera start sending data
   *-----------------------------------------------------------------------*/
  FrameNumber = 0;
   //Pass buffers to the SDK
  for(int i = 0; i < NUMBER_OF_BUFFERS; i++) {
     AT_QueueBuffer(*camera, reinterpret_cast<AT_U8*>(AlignedBuffers[i]), BufferSize);
  }
  i_err = AT_Command(*camera, L"AcquisitionStart");
  errorOk(i_err, "AT_Command 'AcquisitionStart'");

  // Refresh properties
  getProps();
  // Refresh features
  getFeatures();

  return 0;
}

void  
CameraZyla::cleanup_and_exit()
{
  QLOG_DEBUG() << "Cleanup camera Zyla connection";
  i_err = AT_Command(*camera, L"AcquisitionStop");
  errorOk(i_err, "AT_Command 'AcquisitionStop'");
  i_err = AT_Flush(*camera);
  errorOk(i_err, "AT_Flush");
  i_err = AT_Close(*camera);
  errorOk(i_err, "AT_Close");
  i_err = AT_FinaliseLibrary();
  errorOk(i_err, "AT_FinaliseLibrary");
  if (buffer) { free(buffer); buffer = NULL;}
  if (snapshot) { free(snapshot); snapshot = NULL;}
  if (buffer32) { free(buffer32); buffer32 = NULL;}
  if (snapshot32) { free(snapshot32); snapshot32 = NULL;}
  if (image) delete image;
  return;
}

int 
CameraZyla::acquireImage() {
   
  int acq_err = 1;
  QLOG_DEBUG () << " Cycle start ";
  acquireMutex->lock();
  /*-----------------------------------------------------------------------
   * acquire frame
   *-----------------------------------------------------------------------*/
  double eTime = GetTime();
  uchar* pBuf = NULL;
  BufSize = 0;
  QLOG_DEBUG() << "CameraZyla::acquireImage> Trigger " 
               << QString(zyla_trigger_modes[trigger_num]);
  if ( zyla_trigger_modes[trigger_num] == "Software" ) {
     QLOG_DEBUG() << "CameraZyla::acquireImage> Software trigger";
     i_err = AT_Command(*camera, L"SoftwareTrigger");
     errorOk(i_err, "AT_Command 'SoftwareTrigger'");
  }
  i_err = AT_WaitBuffer(*camera, &pBuf, &BufSize, TIMEOUT_MS);
  errorOk(i_err, "AT_Command 'AT_WaitBuffer'");
  if ( i_err == AT_SUCCESS && BufSize == BufferSize ) {
    // Lock the acquisition
    snapshotMutex->lock();
    // calculate min,max
    AT_64 i64_max = 0;
    AT_64 i64_min = 65535;
    ushort *tmpBuf;
    uchar *tmpBuf_c;
    tmpBuf_c = pBuf;
    tmpBuf = reinterpret_cast<ushort*>(pBuf);
    // Treat Mono12Packed case
    if (pixel_encoding == B12P) {
      for (AT_64 ii = 0; ii < BufferSize; ii+=3) {
        AT_64 ui_current = EXTRACTLOWPACKED(tmpBuf_c);
        AT_64 ui_currenth = EXTRACTHIGHPACKED(tmpBuf_c);
        if (ui_current < i64_min) {
          i64_min = ui_current;
        }
        else if (ui_current > i64_max) {
          i64_max = ui_current;
        }
        if (ui_currenth < i64_min) {
          i64_min = ui_currenth;
        }
        else if (ui_currenth > i64_max) {
          i64_max = ui_currenth;
        }
        tmpBuf_c+=3;
      }
    }
    // Treat Mono12 case
    else if (pixel_encoding == B12) {
     for (AT_64 ii = 0; ii < height * width; ii++) {
      ushort ui_current = *(tmpBuf++) & 0x07FF;
      if (ui_current < i64_min) {
        i64_min = ui_current;
      }
      else if (ui_current > i64_max) {
        i64_max = ui_current;
      }
     }
    }
    // Treat Mono16 case
    else if (pixel_encoding == B16) {
     for (AT_64 ii = 0; ii < height * width; ii++) {
      ushort ui_current = *(tmpBuf++);
      if (ui_current < i64_min) {
        i64_min = ui_current;
      }
      else if (ui_current > i64_max) {
        i64_max = ui_current;
      }
     }
    }
    // set the raw precision min and max
    min = (int) i64_min;
    max = (int) i64_max;

    // Convert current buffer to 8-bit buffer (0 - 255 range) & buffer32 (32-bit)
    tmpBuf_c = pBuf;
    tmpBuf = reinterpret_cast<ushort*>(pBuf);
    
    // Reset video image buffer (8-bit)
    memset(buffer,0,height * width * sizeof(uchar));
    // Reset 32-bit image buffer 
    memset(buffer32,0,height * width * sizeof(int));

    // Treat Mono12Packed case
    if (pixel_encoding == B12P) {
      int i = 0, j = 0;
      for (AT_64 ii = 0; ii < BufferSize; ii+=3) {
        AT_64 ui_current = EXTRACTLOWPACKED(tmpBuf_c);
        AT_64 ui_currenth = EXTRACTHIGHPACKED(tmpBuf_c);
        buffer32[i++] = ui_current;
        buffer32[i++] = ui_currenth;
        if ( (max - min) != 0 )
          buffer[j++] = (uchar) (( 255 * (ui_current - min) ) / (max - min));
        else
          buffer[j++] = 255;
        if ( (max - min) != 0 )
          buffer[j++] = (uchar) (( 255 * (ui_currenth - min) ) / (max - min));
        else
          buffer[j++] = 255;
        tmpBuf_c+=3;
      }
    }
    // Treat Mono12 case
    else if (pixel_encoding == B12) {
     for (AT_64 i = 0; i < height * width; i++) {
      ushort ui_current = *(tmpBuf++) & 0x07FF;
      buffer32[i] = ui_current;
      if ( (max - min) != 0 )
       buffer[i] = (uchar) (( 255 * (ui_current - min) ) / (max - min));
      else
       buffer[i] = 255;
     }
    }
    // Treat Mono16 case
    else if (pixel_encoding == B16) {
     for (AT_64 i = 0; i < height * width; i++) {
      ushort ui_current = *(tmpBuf++);
      buffer32[i] = ui_current;
      if ( (max - min) != 0 )
       buffer[i] = (uchar) (( 255 * (ui_current - min) ) / (max - min));
      else
       buffer[i] = 255;
     }
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
    image->loadFromData (buffer,width * height);
    QImage imagescaled = image->scaled(imageWidth,imageHeight);
    QImage imagergb32 =  imagescaled.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    emit getImage(imagergb32);
    emit updateMin(min);
    emit updateMax(max);
    /*-----------------------------------------------------------------------
    * release frame
    *-----------------------------------------------------------------------*/
    FrameNumber++;
    //Re-queue the buffers
    AT_QueueBuffer(*camera, reinterpret_cast<AT_U8*>(AlignedBuffers[FrameNumber%NUMBER_OF_BUFFERS]), 
 		   BufferSize);
    QLOG_DEBUG() << "CameraZyla::acquireImage> frame number " << FrameNumber 
                << " MIN " << min << " MAX " << max;
  }
  else {
   acq_err = 0;
   // Reinit Acquisition
   QLOG_WARN() << "CameraZyla::acquireImage>  BufSize=" << BufSize << " BufferSize=" << BufferSize
	       << "...Reinit Acquisition!";
   QLOG_INFO () << "CameraZyla::acquireImage> AcquisitionStop...";
   i_err = AT_Command(*camera, L"AcquisitionStop");
   errorOk(i_err, "AT_Command 'AcquisitionStop'");
   i_err = AT_Flush(*camera);
   errorOk(i_err, "AT_Flush");
   FrameNumber = 0;
   QLOG_INFO () << "CameraZyla::acquireImage> Reallocating the image buffer ";
   i_err = AT_GetInt(*camera, L"ImageSizeBytes", &BufferSize);
   errorOk(i_err, "AT_GetInt 'ImageSizeBytes'");
   i_err = AT_GetInt(*camera, L"AOIWidth", &aoi_width);
   errorOk(i_err, "AT_GetInt 'AOIWidth'");
   i_err = AT_GetInt(*camera, L"AOIHeight", &aoi_height);
   errorOk(i_err, "AT_SetInt 'AOIHeight'");
   height = static_cast<int>(aoi_height);
   width  = static_cast<int>(aoi_width);
   QLOG_INFO () << "CameraZyla::acquireImage> Updated internal buffer " << BufferSize;
   QLOG_INFO () << "CameraZyla::acquireImage> width " << width << " height " << height;
   if (buffer) { free(buffer); buffer = NULL;}
   if (snapshot) { free(snapshot); snapshot = NULL;}
   if (buffer32) { free(buffer32); buffer32 = NULL;}
   if (snapshot32) { free(snapshot32); snapshot32 = NULL;}
   buffer = (uchar*)malloc( sizeof(uchar) * width * height);
   snapshot = (uchar*)malloc( sizeof(uchar) * width * height);
   buffer32 = (int*)malloc( sizeof(int) * width * height);
   snapshot32 = (int*)malloc( sizeof(int) * width * height);

   delete image;
   image = new QImage(buffer,width,height,width,QImage::Format_Indexed8);
   QVector<QRgb> table;
   for (int i = 0; i < 256; i++) table.append(qRgb(i, i, i));
   image->setColorTable(table);
   for (int i = 0; i < NUMBER_OF_BUFFERS; i++) {
     delete AlignedBuffers[i];
     AlignedBuffers[i] = new ushort[BufferSize];
   }
   for(int i = 0; i < NUMBER_OF_BUFFERS; i++) {
    AT_QueueBuffer(*camera, reinterpret_cast<AT_U8*>(AlignedBuffers[i]), BufferSize);
   }
   i_err = AT_Command(*camera, L"AcquisitionStart");
   errorOk(i_err, "AT_Command 'AcquisitionStart'");
   QLOG_INFO () << "CameraZyla::acquireImage> AcquisitionStart";
  }
  eTime = GetTime() - eTime;
  QLOG_DEBUG () << " Process eTime " << QString::number(eTime);
  QLOG_DEBUG () << " Cycle end ";
  acquireMutex->unlock();
  return (acq_err);
}

void CameraZyla::setImageSize(const int &_imageWidth, const int &_imageHeight){
  
  imageWidth = _imageWidth;
  imageHeight = _imageHeight;
  QLOG_DEBUG ( ) <<  "emit new image with size " << imageWidth  << "x" << imageHeight;
}

const char * CameraZyla::sdkErrorString(int _i_errorCode)
{
        const char * sz_ret = NULL;
        switch (_i_errorCode) {
                case AT_SUCCESS:
                        sz_ret = "AT_SUCCESS";
                        break;
                case AT_ERR_NOTINITIALISED:
                        sz_ret = "AT_ERR_NOTINITIALISED";
                        break;
                case AT_ERR_NOTIMPLEMENTED:
                        sz_ret = "AT_ERR_NOTIMPLEMENTED";
                        break;
                case AT_ERR_READONLY:
                        sz_ret = "AT_ERR_READONLY";
                        break;
                case AT_ERR_NOTREADABLE:
                        sz_ret = "AT_ERR_NOTREADABLE";
                        break;
                case AT_ERR_NOTWRITABLE:
                        sz_ret = "AT_ERR_NOTWRITABLE";
                        break;
                case AT_ERR_OUTOFRANGE:
                        sz_ret = "AT_ERR_OUTOFRANGE";
                        break;
                case AT_ERR_INDEXNOTAVAILABLE:
                        sz_ret = "AT_ERR_INDEXNOTAVAILABLE";
                        break;
                case AT_ERR_INDEXNOTIMPLEMENTED:
                        sz_ret = "AT_ERR_INDEXNOTIMPLEMENTED";
                        break;
                case AT_ERR_EXCEEDEDMAXSTRINGLENGTH:
                        sz_ret = "AT_ERR_EXCEEDEDMAXSTRINGLENGTH";
                        break;
                case AT_ERR_CONNECTION:
                        sz_ret = "AT_ERR_CONNECTION";
                        break;
                case AT_ERR_NODATA:
                        sz_ret = "AT_ERR_NODATA";
                        break;
                case AT_ERR_INVALIDHANDLE:
                        sz_ret = "AT_ERR_INVALIDHANDLE";
                        break;
                case AT_ERR_TIMEDOUT:
                        sz_ret = "AT_ERR_TIMEDOUT";
                        break;
                default:
                        sz_ret = "Unknown code";
                        break;
        }
        return sz_ret;
}

bool CameraZyla::errorOk(int _i_err, const char * _sz_caller)
{
   bool b_ret = true;
   if (_i_err != AT_SUCCESS) {
      QLOG_ERROR () << "Error returned from "
                    <<  _sz_caller
                    << " " << _i_err
	            << " " <<  sdkErrorString(_i_err);
      b_ret = false;
   }
   return b_ret;
}
#endif
