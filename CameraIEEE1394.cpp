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
#ifdef IEEE1394CAMERA
#include "CameraIEEE1394.h"
 
#define VIDEO_MODES_OFFSET  64
#define FRAMERATES_OFFSET 32
#define IIDC_FRAME_NUMBER 15
 
char *pixel_encodings_str[] = {(char*)"MONO8",
			       (char*)"MONO10",
                               (char*)"MONO12",
                               (char*)"MONO12P",
                               (char*)"MONO16",
         	               (char*)"UNKNOWN"};

char *iidc_features[]  = {(char*)"BRIGHTNESS",
			  (char*)"EXPOSURE",
			  (char*)"SHARPNESS",
			  (char*)"WHITE_BALANCE",
			  (char*)"FEATURE_HUE",
			  (char*)"SATURATION",
			  (char*)"GAMMA",
			  (char*)"SHUTTER",
			  (char*)"GAIN",
			  (char*)"IRIS",
			  (char*)"FOCUS",
			  (char*)"TEMPERATURE",
			  (char*)"TRIGGER",
			  (char*)"TRIGGER_DELAY",
			  (char*)"WHITE_SHADING",
			  (char*)"FRAME_RATE",
			  (char*)"ZOOM",
			  (char*)"PAN",
			  (char*)"TILT",
			  (char*)"OPTICAL_FILTER",
			  (char*)"CAPTURE_SIZE",
			  (char*)"CAPTURE_QUALITY"};

char *iidc_props[] = {(char*)"Frame Rate",
                      (char*)"PC Rate",
 		      (char*)"Image Size",
                      (char*)"Pixel Encoding",
                      (char*)"Video Mode"};

char *iidc_framerates[] {(char*)"1.875",
			(char*)"3.75",
			(char*)"7.5",
			(char*)"15",
			(char*)"30",
			(char*)"60",
			(char*)"120",
			(char*)"240",
                        (char*)"---"};

char *iidc_video_modes[] = {(char*)"160x120_YUV444",
			    (char*)"320x240_YUV422",
			    (char*)"640x480_YUV411",
			    (char*)"640x480_YUV422",
			    (char*)"640x480_RGB8",
			    (char*)"640x480_MONO8",
			    (char*)"640x480_MONO16",
			    (char*)"800x600_YUV422",
			    (char*)"800x600_RGB8",
			    (char*)"800x600_MONO8",
			    (char*)"1024x768_YUV422",
			    (char*)"1024x768_RGB8",
			    (char*)"1024x768_MONO8",
			    (char*)"800x600_MONO16",
			    (char*)"1024x768_MONO16",
			    (char*)"1280x960_YUV422",
			    (char*)"1280x960_RGB8",
			    (char*)"1280x960_MONO8",
			    (char*)"1600x1200_YUV422",
			    (char*)"1600x1200_RGB8",
			    (char*)"1600x1200_MONO8",
			    (char*)"1280x960_MONO16",
			    (char*)"1600x1200_MONO16",
			    (char*)"EXIF",
			    (char*)"FORMAT7_0",
			    (char*)"FORMAT7_1",
			    (char*)"FORMAT7_2",
			    (char*)"FORMAT7_3",
			    (char*)"FORMAT7_4",
			    (char*)"FORMAT7_5",
			    (char*)"FORMAT7_6",
			    (char*)"FORMAT7_7"};

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

CameraIEEE1394::CameraIEEE1394()
  :Camera()
{
  hflip = 0;
  vflip = 0;
  image = NULL;
  buffer = NULL;
  snapshot = NULL;
  buffer32 = NULL;
  snapshot32 = NULL;
  d = NULL;
  suspend = true;
  has_started = false;
  mutex = new QMutex(QMutex::NonRecursive);
  snapshotMutex = new QMutex(QMutex::Recursive);
  acquireMutex = new QMutex(QMutex::Recursive);
  acqstart = new QWaitCondition();
  acqend = new QWaitCondition();
  modeCheckEnabled = true;
}

CameraIEEE1394::~CameraIEEE1394()
{
  QLOG_INFO() << "Deleting CameraIEEE1394";
  stop();
  cleanup_and_exit();
  delete mutex;
  delete snapshotMutex;
  delete acquireMutex;
  delete acqstart;
  delete acqend;
}
void
CameraIEEE1394::setCamera(void* _camera, int _id)
{
  camera = (dc1394camera_t*)_camera;
  QLOG_DEBUG() << "CameraIEEE1394::setCamera> camera pointer " <<  camera;
  vendor = camera->vendor;
  model = camera->model;

   /* Init camera*/
  err = dc1394_reset_bus (camera);
  /*DC1394_ERR_CLN_RTN(err,cleanup_and_exit(),"reset bus reported error");
*/
  err = dc1394_feature_get_all(camera,&features);
  if (err != DC1394_SUCCESS) {
    showWarning("Could not get feature set");
    return;
  }
  else {
    QLOG_DEBUG() << dc1394_feature_print_all(&features, stdout);
  }
  int featureCnt = 0;
  for (int i = 0 ; i < DC1394_FEATURE_NUM; i++) {
    if (features.feature[i].available == DC1394_TRUE && 
	features.feature[i].id != DC1394_FEATURE_TRIGGER) {      
      featureNameList.push_back(iidc_features[i]);
      featureIdList.push_back(i);
      featureMinList.push_back(features.feature[i].min);
      featureMaxList.push_back(features.feature[i].max);
      featureValueList.push_back(features.feature[i].value);
      featureAbsCapableList.push_back(features.feature[i].absolute_capable);
      featureAbsValueList.push_back(features.feature[i].abs_value);
      if (features.feature[i].current_mode == DC1394_FEATURE_MODE_AUTO)
	featureModeAutoList.push_back(true);
      else
	featureModeAutoList.push_back(false);
      QLOG_INFO() << "CameraIEEE1394::setCamera Found feature " << 
	featureNameList.at(featureCnt++);
    }
  }
  // Video mode feature
  featureNameList.push_back("VIDEO MODE");
  featureIdList.push_back(DC1394_FEATURE_NUM + 1);
  video_mode_feature = featureCnt++;
  // get video modes:
  err = dc1394_video_get_supported_modes(camera,&video_modes);
  if (err != DC1394_SUCCESS) {
    showWarning("Can't get video modes");
    return;
  }
  featureMinList.push_back(0);
  featureMaxList.push_back(video_modes.num - 1);
  featureValueList.push_back(0);
  featureAbsCapableList.push_back(false);
  featureAbsValueList.push_back(0);
  featureModeAutoList.push_back(false);

  /* select highest video mode*/
  unsigned int tmpwidth = 0, tmpheight = 0;
  dc1394video_mode_t tmpvideo_mode;
  dc1394color_coding_t tmpencoding_num;
  for ( int i = 0 ; i < video_modes.num ; i++) {
    tmpvideo_mode = video_modes.modes[i];
    dc1394_get_image_size_from_video_mode(camera, tmpvideo_mode,
                                        &tmpwidth, &tmpheight);
    dc1394_get_color_coding_from_video_mode(camera,
                                      tmpvideo_mode,
                                      &tmpencoding_num);
    // check for highest resolution
    QLOG_INFO () << "CameraIEEE1394::setCamera> Available IIDC video mode "
                 << tmpvideo_mode
                 << " color coding " << tmpencoding_num
                 << " width " << tmpwidth << " height " << tmpheight;
    if ( width * height < tmpwidth * tmpheight ) {
        width = tmpwidth;
        height = tmpheight;
        encoding_num = tmpencoding_num;
        video_mode = tmpvideo_mode;
        featureValueList.replace(video_mode_feature,i);
    }
  }
  QLOG_INFO() << "CameraIEEE1394::setCamera> Found feature " 
              << featureNameList.at(video_mode_feature) << " set to " 
              << video_modes.modes[(int)featureValueList.at(video_mode_feature)];
  uint32_t data_depth;
  dc1394_get_color_coding_data_depth(encoding_num,&data_depth);
  QLOG_INFO() << "CameraIEEE1394::setCamera> Pixel data depth " << data_depth;
   /* select highest framerate */
   if ( !dc1394_is_video_mode_scalable(video_mode) ) {
   err = dc1394_get_color_coding_from_video_mode(camera,
                                                   video_mode,
                                                   &encoding_num);
   dc1394framerates_t framerates;
   err = dc1394_video_get_supported_framerates(camera,
                                              video_mode,
                                              &framerates);
   framerate = (dc1394framerate_t)0;
   for (int i = 0 ; i < DC1394_FRAMERATE_NUM; i++) {
    QLOG_INFO () << "CameraIEEE1394::setCamera>> supported framerate "
              << framerates.framerates[i];
    if (framerate < framerates.framerates[i] && framerates.framerates[i] < DC1394_FRAMERATE_MAX) {
     QLOG_INFO () << "CameraIEEE1394::setCamera>> selected highest framerate "
              << framerate;
     framerate = framerates.framerates[i];
     }
    }
    QLOG_INFO () << "CameraIEEE1394::setCamera>> selected highest framerate "
              << framerate;
    err = dc1394_video_set_framerate(camera, framerate);
    }
   else {
     framerate = (dc1394framerate_t)40;
     dc1394_format7_get_color_coding(camera, video_mode, &encoding_num);
    }
  dc1394_get_color_coding_data_depth(encoding_num,&data_depth);
  QLOG_INFO() << "CameraIEEE1394::setCamera> Pixel data depth " << data_depth;

  switch (encoding_num) {
      case DC1394_COLOR_CODING_MONO8:
      pixel_encoding = B8;
      break;
      case DC1394_COLOR_CODING_MONO16:
      pixel_encoding = B16;
      break;
      default:
      QLOG_INFO() << "ENCODING NUMBER " << encoding_num;
      pixel_encoding = UNKNOWN;
      break;
    }

  // Color Coding feature
  featureNameList.push_back("COLOR CODING");
  featureIdList.push_back(DC1394_FEATURE_NUM + 2);
  color_coding_feature = featureCnt++;
  
  featureMinList.push_back(0);
  featureMaxList.push_back(1);
  
  switch (encoding_num) {
      case DC1394_COLOR_CODING_MONO8:
      featureValueList.push_back(0);
      break;
      case DC1394_COLOR_CODING_MONO16:
      featureValueList.push_back(1);
      break;
      default:
      featureValueList.push_back(0);
      break;
    }
  featureAbsCapableList.push_back(false);
  featureAbsValueList.push_back(0);
  featureModeAutoList.push_back(false);

  // Properties
  int propCnt = 0;

  // Frame Rate prop
  QString frateStr = iidc_props[propCnt];
  QLOG_INFO () << "CameraIEEE1394::setCamera> Frame Rate property added ";
  propList.push_back(frateStr);

  // PC Rate prop
  QString pcrateStr = iidc_props[++propCnt];
  QLOG_INFO () << "CameraIEEE1394::setCamera> PC Rate property added ";
  propList.push_back(pcrateStr);

  // Image Size prop
  QString imgsizeStr = iidc_props[++propCnt];
  QLOG_INFO () << "CameraIEEE1394::setCamera> Image Size property added ";
  propList.push_back(imgsizeStr);

  // Pixel Encoding prop
  QString encodingStr = iidc_props[++propCnt];
  QLOG_INFO () << "CameraIEEE1394::setCamera> Pixel Encoding property added ";
  propList.push_back(encodingStr);

  // Video Mode prop
  QString videomodeStr = iidc_props[++propCnt];
  QLOG_INFO () << "CameraIEEE1394::setCamera> Video Mode property added ";
  propList.push_back(videomodeStr);
  
  id = _id;

  camera_err = connectCamera();

  QLOG_INFO() << "CameraIEEE1394::setCamera " << vendor << " model : " 
	      << model << " - Err : " << camera_err;
}
uchar* 
CameraIEEE1394::getSnapshot() {
  snapshotMutex->lock();
  memcpy(snapshot,buffer,width * height * sizeof(uchar));
  snapShotMin = min;
  snapShotMax = max;
  snapshotMutex->unlock();
  return snapshot;
}

int*
CameraIEEE1394::getSnapshot32() {
  snapshotMutex->lock();
  memcpy(snapshot32,buffer32, width * height * sizeof(int));
  snapShotMin = min;
  snapShotMax = max;
  snapshotMutex->unlock();
  return snapshot32;
}
void 
CameraIEEE1394::stop() {
  suspend = true;
  has_started = false;
  wait();
  exit();  
}

void 
CameraIEEE1394::run() {
  int acq_err = 0;
  int acq_cnt = 0;
  if (camera_err == 0 && suspend == true ) {
    suspend = false;
    eTimeTotal = 0;
    while (suspend == false) {
      QLOG_DEBUG () << "CameraIEEE1394::run> " << id << " : start new Acquisition";
      double eTime = GetTime();
      acqstart->wakeAll();
      acq_err = acquireImage();
      QLOG_DEBUG () << "CameraIEEE1394::run> " << id << " : done";
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
       QLOG_DEBUG() << "CameraIEEE1394::run> Acquisition "
                   << "freq " << (int) frequency << " Hz";
       eTimeTotal = 0;
       acq_cnt = 0;
       // Update props
       getProps();
      }
     has_started = true;
    }
    QLOG_DEBUG() << "CameraNeo thread exiting";
  }
}

void
CameraIEEE1394::setFeature(int feature, double value) {

  for (int i = 0 ; i < DC1394_FEATURE_NUM ; i++) {
    if ( feature == i ) {
      /*Set IIDC camera feature*/
      QLOG_DEBUG() << "setFeature> Change " << iidc_features[feature] << " to " << QString::number(value);
      err = dc1394_feature_set_value(camera,
				     features.feature[i].id,
				     value);
      break;
    }
  }
  if ( feature == DC1394_FEATURE_NUM + 1 ) {
    /*Set IIDC Video Mode*/
    QLOG_DEBUG() << "setFeature> Stop Acquisition";
    stop();
    dc1394_video_set_transmission(camera, DC1394_OFF);
    dc1394_capture_stop(camera);
    video_mode = video_modes.modes[(int)value];
    QLOG_INFO() << "setFeature> Change Video Mode to " << video_mode;
    err = dc1394_video_set_mode(camera, video_mode);
    if (err != DC1394_SUCCESS)
      QLOG_WARN() << "setFeature> Could not set video mode set to " << video_mode;
    err = dc1394_video_get_mode(camera, &video_mode); 
    QLOG_INFO() << "setFeature> New Video Mode " << video_mode;
    dc1394_feature_get_all(camera, &features);
    if ( !dc1394_is_video_mode_scalable(video_mode) ) {
      err = dc1394_get_color_coding_from_video_mode(camera,
                                                   video_mode,
                                                   &encoding_num);
      /* select highest framerate */
      dc1394framerates_t framerates;
      err = dc1394_video_get_supported_framerates(camera,
                                              video_mode,
                                              &framerates);
      framerate = (dc1394framerate_t)0;
      for (int i = 0 ; i < DC1394_FRAMERATE_NUM; i++) {
        QLOG_INFO () << "CameraIEEE1394::setFeature>> supported framerate "
                << framerates.framerates[i];
        if (framerate < framerates.framerates[i] && framerates.framerates[i] < DC1394_FRAMERATE_MAX) {
         framerate = framerates.framerates[i];
         QLOG_INFO () << "CameraIEEE1394::setFeature>> selected highest framerate "
                << framerate;
       }
      }
      QLOG_INFO () << "CameraIEEE1394::setFeature>> selected highest framerate "
                << framerate;
      err = dc1394_video_set_framerate(camera, framerate);
    }
    else {
      framerate = (dc1394framerate_t)40;
      dc1394_format7_get_color_coding(camera, video_mode, &encoding_num);
    } 
    QLOG_INFO() << "setFeature> New Color Encoding " << encoding_num;
    uint32_t data_depth;
    dc1394_get_color_coding_data_depth(encoding_num,&data_depth);
    QLOG_INFO() << "CameraIEEE1394::setCamera> Pixel data depth " << data_depth;

    switch (encoding_num) {
      case DC1394_COLOR_CODING_MONO8:
      pixel_encoding = B8;
      break;
      case DC1394_COLOR_CODING_MONO16:
      pixel_encoding = B16;
      break;
      default:
      QLOG_INFO() << "ENCODING NUMBER " << encoding_num;
      pixel_encoding = UNKNOWN;
      break;
    }
    dc1394_get_image_size_from_video_mode(camera, video_mode,
                                        &width, &height);
    QLOG_INFO () << "CameraNeo::setFeature> Reallocating the image buffer "
                 << " width " << width << " height " << height;
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
    sleep(1);
    err = dc1394_video_set_iso_channel(camera,id);
    err = dc1394_video_set_iso_speed(camera,DC1394_ISO_SPEED_400);
    err = dc1394_capture_setup(camera, IIDC_FRAME_NUMBER, 0);
    err = dc1394_video_set_transmission(camera, DC1394_ON);
    start();
    QLOG_INFO() << "setFeature> Release the mutex";
  }
  if ( feature == DC1394_FEATURE_NUM + 2 ) {
    if ( dc1394_is_video_mode_scalable(video_mode) ) {
    /*Set IIDC Video Mode*/
    QLOG_DEBUG() << "setFeature> Stop Acquisition";
    stop();
    dc1394_video_set_transmission(camera, DC1394_OFF);
    dc1394_capture_stop(camera);
    QLOG_INFO() << "setFeature> Change Color Coding to " << QString::number(value);
     switch ((int)value) {
      case 0:
      dc1394_format7_set_color_coding(camera, video_mode, DC1394_COLOR_CODING_MONO8);
      break;
      case 1:
      dc1394_format7_set_color_coding(camera, video_mode, DC1394_COLOR_CODING_MONO16);
      break;
      default:
      dc1394_format7_set_color_coding(camera, video_mode, DC1394_COLOR_CODING_MONO8);
      break;
     }
    framerate = (dc1394framerate_t)40;
    dc1394_format7_get_color_coding(camera, video_mode, &encoding_num);
    QLOG_INFO() << "setFeature> New Color Encoding " << encoding_num;
    uint32_t data_depth;
    dc1394_get_color_coding_data_depth(encoding_num,&data_depth);
    QLOG_INFO() << "CameraIEEE1394::setCamera> Pixel data depth " << data_depth;

    switch (encoding_num) {
      case DC1394_COLOR_CODING_MONO8:
      pixel_encoding = B8;
      break;
      case DC1394_COLOR_CODING_MONO16:
      pixel_encoding = B16;
      break;
      default:
      QLOG_INFO() << "ENCODING NUMBER " << encoding_num;
      pixel_encoding = UNKNOWN;
      break;
     }
    }
    dc1394_get_image_size_from_video_mode(camera, video_mode,
                                        &width, &height);
    QLOG_INFO () << "CameraNeo::setFeature> Reallocating the image buffer "
                 << " width " << width << " height " << height;
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
    sleep(1);
    err = dc1394_video_set_iso_channel(camera,id);
    err = dc1394_video_set_iso_speed(camera,DC1394_ISO_SPEED_400);
    err = dc1394_capture_setup(camera, IIDC_FRAME_NUMBER, 0);
    err = dc1394_video_set_transmission(camera, DC1394_ON);
    start();
    QLOG_INFO() << "setFeature> Restart Acquisition";
  }
}
void  
CameraIEEE1394::setMode(int feature, bool value) {
  for (int i = 0 ; i < DC1394_FEATURE_NUM ; i++) {
    if (feature == i) {
      /*Set camera feature mode*/
      QLOG_INFO() << "SetMode> Change feature mode" << iidc_features[feature] << " to " << value;
      if (value == true ) 
	err = dc1394_feature_set_mode(camera,
				      features.feature[i].id,
				      DC1394_FEATURE_MODE_MANUAL);
      
      else
	err = dc1394_feature_set_mode(camera,
				      features.feature[i].id,
				      DC1394_FEATURE_MODE_AUTO);
      break;
    }
  }
}
void
CameraIEEE1394::getFeatures() {
  /*-----------------------------------------------------------------------
   *  report camera's features
   *-----------------------------------------------------------------------*/
  err = dc1394_feature_get_all(camera,&features);
  if (err != DC1394_SUCCESS) {
    showWarning("Could not get feature set");
  }
  else {
    QLOG_DEBUG() << dc1394_feature_print_all(&features, stdout);
  }
  for (int i = 0 ; i < DC1394_FEATURE_NUM; i++) {
    if (features.feature[i].available == DC1394_TRUE && 
	features.feature[i].id != DC1394_FEATURE_TRIGGER) {
      for (int j = 0 ; j < featureNameList.size(); j++) {
	if (featureNameList[j] == iidc_features[i]) {
	  featureValueList.replace(j,features.feature[i].value);
	  featureAbsValueList.replace(j,features.feature[i].abs_value);
	  if (features.feature[i].current_mode == DC1394_FEATURE_MODE_AUTO)
	    featureModeAutoList.replace(j,true);
	  else
	    featureModeAutoList.replace(j,false);
	  break;
	}
      }
    }
  }
  // get Video Mode feature
  err = dc1394_video_get_mode(camera, &video_mode);
  for ( int i = 0 ; i < video_modes.num ; i++) {
    if (video_modes.modes[i] == video_mode) {
      featureValueList.replace(video_mode_feature, i);
      err = dc1394_get_color_coding_from_video_mode(camera,
                                                    video_mode,
                                                    &encoding_num);
      // Set Pixel encoding
      switch (encoding_num) {
      case DC1394_COLOR_CODING_MONO8:
      pixel_encoding = B8;
      break;
      case DC1394_COLOR_CODING_MONO16:
      pixel_encoding = B16;
      break;
      default:
      QLOG_INFO() << "ENCODING NUMBER " << encoding_num;
      pixel_encoding = UNKNOWN;
      break;
      }
      dc1394_get_image_size_from_video_mode(camera, video_mode,
                                        &width, &height);
      break;
    }
  }
  // Color Coding feature
  switch (encoding_num) {
      case DC1394_COLOR_CODING_MONO8:
      featureValueList.replace(color_coding_feature, 0);
      break;
      case DC1394_COLOR_CODING_MONO16:
      featureValueList.replace(color_coding_feature,1);
      break;
      default:
      featureValueList.replace(color_coding_feature,0);
      break;
    }

  emit updateFeatures();
}
void 
CameraIEEE1394::getProps()  {

  int propCnt = 0;

  // Frame Rate prop
  QString frateStr = iidc_props[propCnt];
  //err = dc1394_video_get_framerate(camera, &framerate);
  QLOG_DEBUG () << "CameraIEEE1394::getProps> Frame Rate updated " 
               << iidc_framerates[framerate - FRAMERATES_OFFSET];
  frateStr.append(" : " + QString(iidc_framerates[framerate - FRAMERATES_OFFSET]) + " Hz");
  propList.replace(propCnt,frateStr);

  // PC Rate prop
  QString pcrateStr = iidc_props[++propCnt];
  QLOG_DEBUG () << "CameraIEEE1394::getProps> PC Rate updated "
               << frequency;
  pcrateStr.append(" : " + QString::number((int)frequency) + " Hz");
  propList.replace(propCnt,pcrateStr);

  // Image Size prop
  QString imgsizeStr = iidc_props[++propCnt];
  QLOG_DEBUG () << "CameraIEEE1394::getProps> Image Size updated "
               << width * height;
  imgsizeStr.append(" : " + QString::number(width) + "x" + QString::number(height));
  propList.replace(propCnt,imgsizeStr);

  // Encoding prop
  QString encodingStr = iidc_props[++propCnt];
  QLOG_DEBUG () << "CameraIEEE1394::getProps> Encoding updated "
               << pixel_encodings_str[pixel_encoding];
  encodingStr.append(" : " + QString(pixel_encodings_str[pixel_encoding]));
  propList.replace(propCnt,encodingStr);

  // Video Mode prop
  QString videomodeStr = iidc_props[++propCnt];
  QLOG_DEBUG() << "CameraIEEE1394::getProps> Video Mode updated "
               << iidc_video_modes[video_mode - VIDEO_MODES_OFFSET];
  videomodeStr.append(" : " + QString(iidc_video_modes[video_mode - VIDEO_MODES_OFFSET]));
  propList.replace(propCnt,videomodeStr);
  
  emit updateProps();
}
int 
CameraIEEE1394::findCamera() {
   int i;
   /* Find available firewire cameras*/
   d = dc1394_new ();
   if (!d)
     return -1;
   err = dc1394_camera_enumerate (d, &list);
   DC1394_ERR_RTN(err,"Failed to enumerate cameras");
   
   num = list->num;
   if (num == 0) {
     QLOG_DEBUG() << "CameraIEEE1394::findCamera> No cameras found";
     return -1;
   }
   // Allocate camera list
   for (i = 0 ; i < num; i++) {
     dc1394camera_t *camera = dc1394_camera_new (d,list->ids[i].guid);
     if (!camera) {
       QLOG_ERROR() << "CameraIEEE1394::findCamera> Failed to initialize camera with guid " 
                    << list->ids[i].guid;
       return -1;
     }
     cameralist.push_back((dc1394camera_t *)camera);
     vendorlist.push_back(camera->vendor);
     modelist.push_back(camera->model);
   }
   dc1394_camera_free_list (list);
   return (err);
}
int 
CameraIEEE1394::connectCamera() {

  camera_err = -1;

  err = dc1394_video_set_mode(camera, video_mode);

  // Refresh properties
  getProps();
  // Refresh features
  getFeatures();

  /*-----------------------------------------------------------------------
   *  setup capture
   *-----------------------------------------------------------------------*/
  err = dc1394_video_set_iso_channel(camera,id);
  DC1394_ERR_CLN_RTN(err,cleanup_and_exit(),
		     "Could not set iso channel");
  err = dc1394_video_set_iso_speed(camera,DC1394_ISO_SPEED_400);
  DC1394_ERR_CLN_RTN(err,cleanup_and_exit(),
		     "Could not set iso speed");
  err = dc1394_capture_setup(camera, IIDC_FRAME_NUMBER, 0);
  DC1394_ERR_CLN_RTN(err,cleanup_and_exit(),
		     "\nCould not setup camera\n"
		     "make sure that the video mode and framerate are\n"
		     "supported by your camera");
  uint32_t iso_channel;
  err = dc1394_video_get_iso_channel(camera,&iso_channel);
  QLOG_DEBUG() << "CameraIEEE1394 " << id << ": ISO Channel " << iso_channel;
  
  dc1394_get_image_size_from_video_mode(camera, video_mode, 
					&width, &height);
  QLOG_INFO() << "CameraIEEE1394::connectCamera> image resolution used width " 
              << width << " height " << height;
  QLOG_DEBUG() << "CameraIEEE1394::connectCamera> Allocating buffer for camera pointer " <<  camera;
  
  buffer = (uchar*)malloc( sizeof(uchar) * width * height);
  snapshot = (uchar*)malloc( sizeof(uchar) * width * height);
  buffer32 = (int*)malloc( sizeof(int) * width * height);
  snapshot32 = (int*)malloc( sizeof(int) * width * height);

  image = new QImage(buffer,width,height,width,QImage::Format_Indexed8);
  QVector<QRgb> table;
  for (int i = 0; i < 256; i++)
    table.append(qRgb(i, i, i));
  image->setColorTable(table);

  /*-----------------------------------------------------------------------
   *  have the camera start sending data
   *-----------------------------------------------------------------------*/
  err = dc1394_video_set_transmission(camera, DC1394_ON);
  DC1394_ERR_CLN_RTN(err,cleanup_and_exit(),
		     "connectCamera> Could not start camera iso transmission");

  return 0;

}
void  
CameraIEEE1394::cleanup_and_exit()
{
  QLOG_DEBUG() << "Cleanup camera firewire connection";
  dc1394_video_set_transmission(camera, DC1394_OFF);
  dc1394_capture_stop(camera);
  dc1394_camera_free(camera);
  camera = NULL;
  if (d) dc1394_free (d);
  if (buffer) { free(buffer); buffer = NULL;}
  if (snapshot) { free(snapshot); snapshot = NULL;}
  if (buffer32) { free(buffer32); buffer32 = NULL;}
  if (snapshot32) { free(snapshot32); snapshot32 = NULL;}
  if (image) delete image;
  return;
}

int 
CameraIEEE1394::acquireImage() {
   
     acquireMutex->lock();
     /*Init frame to NULL*/
     frame = NULL;
    /*-----------------------------------------------------------------------
     *  capture one frame
     *-----------------------------------------------------------------------*/
    err = dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, 
					&frame);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(),
		       "acquireImage> Could not capture a frame"); 
    QLOG_DEBUG() << "CameraIEEE1394::acquireImage> Frame image size " << frame->image_bytes;
    /* copy captured image */
    snapshotMutex->lock();
    ushort *tmpBuf;
    // calculate min,max
    max = 0;
    min = 65535;
    if ( pixel_encoding == B8 ) {
      for (unsigned int i = width ; i < width * height - width; i++) { // avoid first/last line to calculate min/max
        if (frame->image[i] > max)
          max = frame->image[i];
        if (frame->image[i] < min)
          min = frame->image[i];
      }
    }
    if ( pixel_encoding == B16 ) {
      tmpBuf = reinterpret_cast<ushort*>(frame->image); 
      for (unsigned int i = width ; i < width * height - width; i++) { // avoid first line to calculate min/max
        ushort ui_current = *(tmpBuf++);
        if (ui_current > max)
          max = ui_current;
        if (ui_current < min)
          min = ui_current;
      }
    }
    // copy image to buffer (8-bit : 0 - 255 range) and to buffer32 (32-bit)
    // Reset buffer (8-bit) and buffer32 (32-bit)
    memset(buffer,0,height * width * sizeof(uchar));
    memset(buffer32,0,height * width * sizeof(int));  
    if ( pixel_encoding == B8 ) {
      memcpy(buffer,frame->image,width * height * sizeof(uchar));
      for (unsigned int i = 0 ; i < width * height; i++)
        buffer32[i] = buffer[i];
    }
    if ( pixel_encoding == B16 ) {
      tmpBuf = reinterpret_cast<ushort*>(frame->image);
      for (unsigned int i = 0 ; i < width * height; i++) {
        ushort ui_current = *(tmpBuf++);
        buffer32[i] = ui_current;
        if ( (max - min) != 0 )
          buffer[i] = (uchar) (( 255 * (ui_current - min) ) / (max - min));
        else
          buffer[i] = 255;
      }
    }
    if (vflip) {
      buffer = transpose(buffer,height * width);
      buffer = rotate(buffer,height * width, width);
      buffer32 = transpose(buffer32,height * width);
      buffer32 = rotate(buffer32,height * width, width);
    }
    else if (hflip) {
     buffer = rotate(buffer,height*width, width);
     buffer32 = rotate(buffer32,height*width, width);
    }
    snapshotMutex->unlock();
    image->loadFromData (buffer,width * height);
    // emit visualisation image
    QImage imagescaled = image->scaled(imageWidth,imageHeight);
    QImage imagergb32 =  imagescaled.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    emit getImage(imagergb32);
    emit updateMin(min);
    emit updateMax(max);
    /*-----------------------------------------------------------------------
     * release frame
     *-----------------------------------------------------------------------*/
    err = dc1394_capture_enqueue(camera, frame);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(),
		       "acquireImage> Could not enqueue a frame");
    acquireMutex->unlock();
    return (1);
}

void CameraIEEE1394::setImageSize(const int &_imageWidth, const int &_imageHeight){
  
  imageWidth = _imageWidth;
  imageHeight = _imageHeight;
  QLOG_DEBUG ( ) <<  "emit new image with size " << imageWidth  << "x" << imageHeight;
}

#endif
