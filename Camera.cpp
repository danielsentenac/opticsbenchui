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

#include "Camera.h"
				       
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

int VIDEO_MODES_OFFSET = 64;

Camera::Camera(QObject* parent)
  :QThread(parent)
{
  image = NULL;
  buffer = NULL;
  d = NULL;
  suspend = true;
  has_started = false;
  mutex = new QMutex(QMutex::NonRecursive);
  snapshotMutex = new QMutex(QMutex::Recursive);
  acqstart = new QWaitCondition();
  acqend = new QWaitCondition();
}

Camera::~Camera()
{
  QLOG_INFO() << "Deleting Camera";
  stop();
  cleanup_and_exit();
  delete mutex;
  delete snapshotMutex;
  delete acqstart;
  delete acqend;
}
void
Camera::setCamera(dc1394camera_t* _camera, int _id)
{
  camera = _camera;
  id = _id;
  camera_err = connectCamera();
  QLOG_DEBUG() << "Camera::setCamera " << camera_err;
  getFeatures();
}
uchar* 
Camera::getSnapshot() {
  snapshotMutex->lock();
  memcpy(snapshot,buffer,width * height);
  snapShotMin = min;
  snapShotMax = max;
  snapshotMutex->unlock();
  return snapshot;
}
void 
Camera::stop() {
  suspend = true;
  has_started = false;
  wait();
  exit();  
}

void 
Camera::run() {
  if (camera_err == 0 && suspend == true ) {
    suspend = false;
    while (suspend == false) {
      QLOG_DEBUG () << " Camera " << id << " : start new Acquisition";
      acqstart->wakeAll();
      acquireImage();
      QLOG_DEBUG () << " Camera " << id << " : done";
      acqend->wakeAll();
      has_started = true;
    }
    QLOG_DEBUG() << "Camera thread exiting";
  }
}

void
Camera::setFeature(int feature, int value) {
  for (int i = 0 ; i < DC1394_FEATURE_NUM ; i++) {
    if (feature == i) {
      /*Set camera feature*/
      QLOG_DEBUG() << "SetFeature> Change " << iidc_features[feature] << " to " << value;
      err = dc1394_feature_set_value(camera,
				     features.feature[i].id,
				     value);
      break;
    }
  }
}
void
Camera::setMode(int feature, bool value) {
  for (int i = 0 ; i < DC1394_FEATURE_NUM ; i++) {
    if (feature == i) {
      /*Set camera feature mode*/
      QLOG_DEBUG() << "SetMode> Change feature mode" << iidc_features[feature] << " to " << value;
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
Camera::getFeatures() {
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
  emit updateFeatures();
}

int 
Camera::findCamera() {
   int i;
   /* Find available firewire cameras*/
   d = dc1394_new ();
   if (!d)
     return -1;
   err = dc1394_camera_enumerate (d, &list);
   DC1394_ERR_RTN(err,"Failed to enumerate cameras");
   
   num = list->num;
   if (num == 0) {
     QLOG_ERROR() << "No cameras found";
     return -1;
   }
   // Allocate camera list
   cameralist = (dc1394camera_t**)malloc(sizeof(dc1394camera_t*) * num);
   for (i = 0 ; i < num; i++) {
     cameralist[i] = dc1394_camera_new (d,list->ids[i].guid);
     if (!cameralist[i]) {
       QLOG_ERROR() << "Failed to initialize camera with guid " << list->ids[i].guid;
       return -1;
     }
   }
   dc1394_camera_free_list (list);
   return (err);
}
int 
Camera::connectCamera() {

  int i;
  camera_err = -1;
   /* Init camera*/
  err = dc1394_reset_bus (camera);
  DC1394_ERR_CLN_RTN(err,cleanup_and_exit(),"reset bus reported error");
  
  // get video modes:
  err = dc1394_video_get_supported_modes(camera,&video_modes);
  DC1394_ERR_CLN_RTN(err,cleanup_and_exit(),
		     "Can't get video modes");
  
  /* select highest res mode*/
  for ( i = video_modes.num - 1 ; i >= 0 ; i--) {
    if (!dc1394_is_video_mode_scalable(video_modes.modes[i])) {
      dc1394_get_color_coding_from_video_mode(camera,
					      video_modes.modes[i], 
					      &coding);
      if (coding == DC1394_COLOR_CODING_MONO8) {
	video_mode = video_modes.modes[i];
        QLOG_INFO () << " Highest mode selected : " << video_mode;
	break;
      }
    }
  }
  if (i < 0) {
    QLOG_ERROR() << "Could not get a valid MONO8 mode";
    cleanup_and_exit();
  }
  
  err = dc1394_get_color_coding_from_video_mode(camera, 
						video_mode,
						&coding);
  DC1394_ERR_CLN_RTN(err,cleanup_and_exit(),
		     "Could not get color coding");
  
  // get highest framerate
  err = dc1394_video_get_supported_framerates(camera,
					      video_mode,
					      &framerates);
  DC1394_ERR_CLN_RTN(err,cleanup_and_exit(),
		     "Could not get framerates");
  framerate = framerates.framerates[framerates.num-1];
  
  /*-----------------------------------------------------------------------
   *  setup capture
   *-----------------------------------------------------------------------*/
  err = dc1394_video_set_iso_channel(camera,id);
  DC1394_ERR_CLN_RTN(err,cleanup_and_exit(),
		     "Could not set iso channel");
  err = dc1394_video_set_iso_speed(camera,DC1394_ISO_SPEED_400);
  DC1394_ERR_CLN_RTN(err,cleanup_and_exit(),
		     "Could not set iso speed");
  
  err=dc1394_video_set_mode(camera, video_mode);
  DC1394_ERR_CLN_RTN(err,cleanup_and_exit(),
		     "Could not set video mode");
  
  err = dc1394_video_set_framerate(camera, framerate);
  DC1394_ERR_CLN_RTN(err,cleanup_and_exit(),
		     "Could not set framerate");
  
  err = dc1394_capture_setup(camera,frequency,0);
  DC1394_ERR_CLN_RTN(err,cleanup_and_exit(),
		     "\nCould not setup camera\n"
		     "make sure that the video mode and framerate are\n"
		     "supported by your camera");
  uint32_t iso_channel;
  err = dc1394_video_get_iso_channel(camera,&iso_channel);
  QLOG_INFO() << "Camera " << id << ": ISO Channel " << iso_channel;
  dc1394_get_image_size_from_video_mode(camera, video_mode, 
					&width, &height);
  QLOG_INFO() << "Image width " << width << " height " << height;
  
  buffer = (uchar*)malloc( sizeof(uchar) * width * height);
  snapshot = (uchar*)malloc( sizeof(uchar) * width * height);

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
		     "acquireImage> Could not start camera iso transmission");

  return 0;

}
void  
Camera::cleanup_and_exit()
{
  QLOG_DEBUG() << "Cleanup camera firewire connection";
  dc1394_video_set_transmission(camera, DC1394_OFF);
  dc1394_capture_stop(camera);
  dc1394_camera_free(camera);
  if (d) dc1394_free (d);
  if (buffer) { free(buffer); buffer = NULL;}
  if (snapshot) { free(snapshot); snapshot = NULL;}
  if (image) delete image;
  return;
}

int 
Camera::acquireImage() {
   
     /*Init frame to NULL*/
    frame = NULL;
    
    /*-----------------------------------------------------------------------
     *  capture one frame
     *-----------------------------------------------------------------------*/
    err = dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, 
					&frame);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(),
		       "acquireImage> Could not capture a frame"); 
        
    /* copy captured image */
    snapshotMutex->lock();
    memcpy(buffer,frame->image,width * height);
    // calculate min,max
    max = 0;
    for (unsigned int i = 0 ; i < width * height; i++) {
      if (buffer[i] > max)
	max = buffer[i];
    }
    min = 255;
    for (unsigned int i = 0 ; i < width * height; i++) {
      if (buffer[i] < min)
	min = buffer[i];
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
    
 
  return (1);
}

void Camera::setImageSize(const int &_imageWidth, const int &_imageHeight){
  
  imageWidth = _imageWidth;
  imageHeight = _imageHeight;
  QLOG_DEBUG ( ) <<  "emit new image with size " << imageWidth  << "x" << imageHeight;
}
