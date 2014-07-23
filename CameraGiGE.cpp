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
#ifdef GIGECAMERA
#include "CameraGiGE.h"
#include <arvcamera.h>

#define GIGE_FRAME_NUMBER 50
#define FEATURES_NUMBER 19
#define PROPS_NUMBER 22
char *gige_features[]  = {
                          (char*)"ExposureTimeAbs",
                          (char*)"DigitalGain",
			  (char*)"PixelFormat",
                          (char*)"Width",
			  (char*)"Height",
			  (char*)"OffsetX",
                          (char*)"OffsetY",
                          (char*)"Correction_Mode",
			  (char*)"Convolver_3x3_0_Enable",
		       	  (char*)"Convolver_3x3_0_Scale",
        		  (char*)"Convolver_3x3_0_Offset",
       			  (char*)"Convolver_3x3_0_Coeff0",
     			  (char*)"Convolver_3x3_0_Coeff2",
     			  (char*)"Convolver_3x3_0_Coeff3",
       			  (char*)"Convolver_3x3_0_Coeff4",
       			  (char*)"Convolver_3x3_0_Coeff5",
      			  (char*)"Convolver_3x3_0_Coeff6",
       			  (char*)"Convolver_3x3_0_Coeff7",
      			  (char*)"Convolver_3x3_0_Coeff8"
                         };
char *gige_props[] = {
                      (char*)"TemperatureADC_Local",
 	              (char*)"ExposureTimeAbs",
   		      (char*)"DigitalGain",
                      (char*)"PixelFormat",
		      (char*)"Width",
                      (char*)"Height",
	              (char*)"OffsetX",
                      (char*)"OffsetY",
                      (char*)"Correction_Mode",
		      (char*)"Convolver_3x3_0_Enable",	
		      (char*)"Convolver_3x3_0_Scale",
                      (char*)"Convolver_3x3_0_Offset",
                      (char*)"Convolver_3x3_0_Coeff0",
                      (char*)"Convolver_3x3_0_Coeff1",
                      (char*)"Convolver_3x3_0_Coeff2",
                      (char*)"Convolver_3x3_0_Coeff3",
                      (char*)"Convolver_3x3_0_Coeff4",
                      (char*)"Convolver_3x3_0_Coeff5",
                      (char*)"Convolver_3x3_0_Coeff6",
                      (char*)"Convolver_3x3_0_Coeff7",
                      (char*)"Convolver_3x3_0_Coeff8",
                      (char*)"PC Rate"
                     };
char *gige_props_units[] = {
                            (char*)"C",
 			    (char*)"ms",
                            (char*)"",
                            (char*)"",
	                    (char*)"",
 			    (char*)"",
			    (char*)"",
                     	    (char*)"",
                            (char*)"",
 			    (char*)"",
			    (char*)"",
                            (char*)"",
                            (char*)"",
                            (char*)"",
                            (char*)"",
                            (char*)"",
                            (char*)"",
                            (char*)"",
                            (char*)"",
                            (char*)"",
                            (char*)"",
                            (char*)"Hz"
                           };
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

CameraGiGE::CameraGiGE()
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
  acqstart = new QWaitCondition();
  acqend = new QWaitCondition();
  modeCheckEnabled = false;
}

CameraGiGE::~CameraGiGE()
{
  QLOG_INFO() << "Deleting CameraGiGE";
  stop();
  cleanup_and_exit();
  delete mutex;
  delete snapshotMutex;
  delete acqstart;
  delete acqend;
}
void
CameraGiGE::setCamera(void* _camera, int _id)
{
 camera = (ArvCamera*)_camera;
 vendor = arv_camera_get_vendor_name (camera);
 model = arv_camera_get_model_name (camera);

 device = arv_camera_get_device(camera);
 double feature_min_d, feature_max_d;
 gint64 feature_min_i, feature_max_i;
 int featureCnt = 0;
 for ( int i = 0; i < FEATURES_NUMBER; i++) {
   ArvGcNode *feature = NULL;
   feature = arv_device_get_feature (device, gige_features[i]);
   if (feature == NULL) {
     QLOG_WARN() << "CameraGiGE::setCamera> Feature "
                 << gige_features[featureCnt] 
               	 <<  " not exists !";
     continue;
   }
   if ( ARV_IS_GC_FLOAT (feature) || 
        ARV_IS_GC_ENUMERATION (feature) ||
        ARV_IS_GC_BOOLEAN (feature) ||
	ARV_IS_GC_INTEGER (feature) 
	) {
      featureIdList.push_back(featureCnt);
      featureNameList.push_back(gige_features[i]);
      featureModeAutoList.push_back(false);
   }
   if ( ARV_IS_GC_FLOAT (feature) ) {
     QVector<QString> feature_string_choice;
     arv_device_get_float_feature_bounds (device, gige_features[i], &feature_min_d, &feature_max_d);
     QLOG_INFO () << "Feature Min " << QString::number(feature_min_d) << " Max " << QString::number(feature_max_d);
     QString feature_string_value;
     feature_string_value = QString(arv_gc_node_get_value_as_string (feature));
     QLOG_INFO() << "CameraGiGE::setCamera> Feature "
                     << featureNameList.at(featureCnt) << " value " << feature_string_value;
     featureValueList.push_back( feature_string_value.toDouble() );
     featureMinList.push_back(feature_min_d );
     featureMaxList.push_back(feature_max_d );
     featureChoiceList.push_back(feature_string_choice);
     featureAbsCapableList.push_back(true);
     featureAbsValueList.push_back(feature_string_value.toDouble());
   }
   else if ( ARV_IS_GC_INTEGER (feature) ) {
     QVector<QString> feature_string_choice;
     arv_device_get_integer_feature_bounds (device, gige_features[i], &feature_min_i, &feature_max_i);
     QLOG_INFO () << "Feature Min " << QString::number(feature_min_i) << " Max " << QString::number(feature_max_i);
     QString feature_string_value;
     feature_string_value = QString(arv_gc_node_get_value_as_string (feature));
     QLOG_INFO() << "CameraGiGE::setCamera> Feature "
                     << featureNameList.at(featureCnt) << " value " << feature_string_value;
     featureValueList.push_back( feature_string_value.toInt());
     featureMinList.push_back(feature_min_i);
     featureMaxList.push_back(feature_max_i);
     featureChoiceList.push_back(feature_string_choice);
     featureAbsCapableList.push_back(false);
     featureAbsValueList.push_back(feature_string_value.toDouble());
   }
   else if ( ARV_IS_GC_BOOLEAN (feature) ) {
     QVector<QString> feature_string_choice;
     QString feature_string_value;
     feature_string_value = QString(arv_gc_node_get_value_as_string (feature));
     QLOG_INFO() << "CameraGiGE::setCamera> Feature "
                 << featureNameList.at(featureCnt) << " value " << feature_string_value;
     feature_min_i = 0;
     feature_max_i = 1;
     feature_string_choice.push_back("false");
     feature_string_choice.push_back("true");
     if (feature_string_value == "true")
       featureValueList.push_back( feature_max_i );
     else if (feature_string_value == "false")
       featureValueList.push_back( feature_min_i);
     featureMinList.push_back(feature_min_i);
     featureMaxList.push_back(feature_max_i);
     featureChoiceList.push_back(feature_string_choice);
     featureAbsCapableList.push_back(false);
     featureAbsValueList.push_back(feature_max_i);
   }
   else if ( ARV_IS_GC_ENUMERATION (feature) ) {
     QVector<QString> feature_string_choice;
     const GSList *childs;
     const GSList *iter;
     childs = arv_gc_node_get_childs (feature);
     int feature_choicecnt = 0;
     for (iter = childs; iter != NULL; iter = iter->next) {
       QLOG_INFO() << arv_gc_node_get_node_name ((ArvGcNode *)iter->data)
                   << " " << arv_gc_node_get_name ((ArvGcNode *)iter->data);
       feature_string_choice.push_back(arv_gc_node_get_name ((ArvGcNode *)iter->data));
       QString feature_string_value = QString(arv_gc_node_get_value_as_string (feature));
       if ( arv_gc_node_get_name ((ArvGcNode *)iter->data) == feature_string_value) {
         QLOG_INFO() << "CameraGiGE::setCamera> Feature "
                     << featureNameList.at(featureCnt) << " value " << feature_string_value;
         featureValueList.push_back(feature_choicecnt);
         // Particular Case Pixel Encoding
         if (featureNameList.at(featureCnt) == "PixelFormat") {
           if (feature_string_value == "Mono8")
             pixel_encoding = B8;
           else if (feature_string_value == "Mono10")
             pixel_encoding = B10;
           else if (feature_string_value == "Mono12")
             pixel_encoding = B12;
         }
       }
       feature_choicecnt++;
     }
     featureChoiceList.push_back(feature_string_choice);
     feature_min_i = 0;
     feature_max_i = feature_string_choice.size() -1;
     featureMinList.push_back(feature_min_i);
     featureMaxList.push_back(feature_max_i);
     featureAbsCapableList.push_back(false);
     featureAbsValueList.push_back(feature_choicecnt);
   }
   featureCnt++;
  }
  int propCnt = 0;
  for ( int i = 0; i < PROPS_NUMBER; i++ ) {
    ArvGcNode *feature = NULL;
    feature = arv_device_get_feature (device, gige_props[i]);
    if ( feature == NULL && gige_props[i] != "PC Rate" ) continue;
    QString propStr = gige_props[i];
    QLOG_INFO () << "CameraGiGE::setCamera> " << propStr << " property added ";
    propList.push_back(propStr);
    propCnt++;
  }
  
  id = _id;
 
  camera_err = connectCamera();

  QLOG_DEBUG() << "CameraGiGE::setCamera " << vendor << " model : "
               << model << " - Err : " << camera_err;
}
uchar* 
CameraGiGE::getSnapshot() {
  snapshotMutex->lock();
  memcpy(snapshot,buffer,width * height * sizeof(uchar));
  snapShotMin = min;
  snapShotMax = max;
  snapshotMutex->unlock();
  return snapshot;
}

int*
CameraGiGE::getSnapshot32() {
  snapshotMutex->lock();
  memcpy(snapshot32,buffer32, width * height * sizeof(int));
  snapShotMin = min;
  snapShotMax = max;
  snapshotMutex->unlock();
  return snapshot32;
}

void 
CameraGiGE::stop() {
  suspend = true;
  has_started = false;
  wait();
  exit();  
}

void 
CameraGiGE::run() {
  int acq_err = 0;
  int acq_cnt = 0;
  QLOG_INFO() << "CameraGiGE::run> start thread";
  if (camera_err == 0 && suspend == true ) {
    QLOG_INFO() << "CameraGiGE::run> start thread in";
    suspend = false;
    eTimeTotal = 0;
    while (suspend == false) {
      QLOG_DEBUG () << "CameraGiGE::run> " << id << " : start new Acquisition";
      double eTime = GetTime();
      acqstart->wakeAll();
      acq_err = acquireImage();
      QLOG_DEBUG () << "CameraGiGE::run> " << id << " : done";
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
       QLOG_DEBUG() << "CameraGiGE::run> Acquisition "
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
CameraGiGE::setFeature(int feature, double value) {

   device = arv_camera_get_device(camera);

   ArvGcNode *node = NULL;
   node = arv_device_get_feature (device, featureNameList.at(feature).toStdString().c_str());
   QLOG_INFO() << "CameraGiGE::setFeature> Updating feature " 
	       << featureNameList.at(feature)
               << " New Value " << QString::number(value);
   if ( node == NULL ) {
       QLOG_ERROR() << "CameraGiGE::setFeature> node NULL ";
       return;
   }
   if ( ARV_IS_GC_FLOAT (node) ) {
     QLOG_INFO() << "CameraGiGE::setFeature> feature has FLOAT type ";
     QString feature_string_value = QString::number( value );
     QLOG_INFO() << "CameraGiGE::setFeature> set feature "
               << featureNameList.at(feature)
               << " New value " << feature_string_value;
     QLOG_INFO() << "CameraGiGE::setFeature> feature " << featureNameList.at(feature)
                 << "has INT type ";
     arv_gc_node_set_value_from_string (node, feature_string_value.toStdString().c_str());
   }
   else if ( ARV_IS_GC_INTEGER (node) ) {
     QString feature_string_value = QString::number((int)value);
     // Particular Case Image Size
     if (featureNameList.at(feature) == "Width" ||
         featureNameList.at(feature) == "Height") {
       // Stop acquisition
       this->stop();
       arv_camera_stop_acquisition (camera);
       g_object_unref (stream);
       
       // Set & Update new Size
       arv_gc_node_set_value_from_string (node, feature_string_value.toStdString().c_str());
       arv_camera_get_region (camera, &x, &y, &gwidth, &gheight);
       QLOG_INFO() << "arv_camera_get_region " << x << " " << y 
                   << " " << gwidth << " " << gheight;
       width = gwidth;
       height = gheight;
       if (buffer) { free(buffer); buffer = NULL;}
       if (buffer32) { free(buffer32); buffer32 = NULL;}
       if (snapshot) { free(snapshot); snapshot = NULL;}
       if (snapshot32) { free(snapshot32); snapshot32 = NULL;}
       buffer = (uchar*)malloc( sizeof(uchar) * width * height);
       snapshot = (uchar*)malloc( sizeof(uchar) * width * height);
       buffer32 = (int*)malloc( sizeof(int) * width * height);
       snapshot32 = (int*)malloc( sizeof(int) * width * height);
       delete image;
       image = new QImage(buffer,width,height,width,QImage::Format_Indexed8);
       image->setColorTable(*table);

       // Start acquisition
       payload = arv_camera_get_payload (camera);
       QLOG_INFO() << "CameraGiGE::setFeature> set feature "
                   << featureNameList.at(feature)
                   << " New value " << feature_string_value
		   << " Updated Width " << width << " Height " << height
		   << " New payload " << payload;

       stream = arv_camera_create_stream (camera, NULL, NULL);
       if (stream != NULL) {
         if (ARV_IS_GV_STREAM (stream)) {
            g_object_set (stream,
                      "socket-buffer", ARV_GV_STREAM_SOCKET_BUFFER_AUTO,
                      "socket-buffer-size", 0,
                      NULL);
            g_object_set (stream,
                      "packet-resend", ARV_GV_STREAM_PACKET_RESEND_NEVER,
                      NULL);
            g_object_set (stream,
                      "packet-timeout", 20 * 1000,
                      "frame-retention", 100 * 1000,
                      NULL);
        }
       }
       else
         QLOG_ERROR() << "CameraGiGE::setFeature> cannot create new stream !";
       for (int i = 0; i < GIGE_FRAME_NUMBER; i++)
         arv_stream_push_buffer (stream, arv_buffer_new (payload, NULL));
       arv_camera_set_acquisition_mode (camera, ARV_ACQUISITION_MODE_CONTINUOUS);
       arv_camera_start_acquisition (camera);
       this->start();
     }
     else {
       QString feature_string_value = QString::number((int)value);
       arv_gc_node_set_value_from_string (node, feature_string_value.toStdString().c_str());
       QLOG_INFO() << "CameraGiGE::setFeature> set feature "
                 << featureNameList.at(feature) 
                 << " New value " << feature_string_value;
     }
   }
   else if ( ARV_IS_GC_BOOLEAN (node) ) {
     QVector<QString> feature_string_choice = featureChoiceList.at(feature);
     QString feature_string_value = feature_string_choice.at((int)value);
      arv_gc_node_set_value_from_string (node, feature_string_value.toStdString().c_str());
       QLOG_INFO() << "CameraGiGE::setFeature> set feature "
                 << featureNameList.at(feature)
                 << " New value " << feature_string_value;
   }
   else if ( ARV_IS_GC_ENUMERATION (node) ) {
     QVector<QString> feature_string_choice = featureChoiceList.at(feature);
     QString feature_string_value = feature_string_choice.at((int)value);
     // Particular Case Pixel Encoding
     if (featureNameList.at(feature) == "PixelFormat") {
          if (feature_string_value == "Mono8")
            pixel_encoding = B8;
          else if (feature_string_value == "Mono10")
            pixel_encoding = B10;
          else if (feature_string_value == "Mono12")
            pixel_encoding = B12;
          
          // Stop acquisition
          this->stop();
          arv_camera_stop_acquisition (camera);
          g_object_unref (stream);

          // Set new Pixelformat
          arv_gc_node_set_value_from_string (node, feature_string_value.toStdString().c_str());
 
          // Start acquisition
          payload = arv_camera_get_payload (camera);
          QLOG_INFO() << "CameraGiGE::setFeature> set feature "
                      << featureNameList.at(feature) 
                      << " New value " << feature_string_choice.at(value)
		      << " New payload " << payload;
          stream = arv_camera_create_stream (camera, NULL, NULL);
          if (stream != NULL) {
            if (ARV_IS_GV_STREAM (stream)) {
               g_object_set (stream,
                      "socket-buffer", ARV_GV_STREAM_SOCKET_BUFFER_AUTO,
                      "socket-buffer-size", 0,
                      NULL);
               g_object_set (stream,
                      "packet-resend", ARV_GV_STREAM_PACKET_RESEND_NEVER,
                      NULL);
               g_object_set (stream,
                      "packet-timeout", 20 * 1000,
                      "frame-retention", 100 * 1000,
                      NULL);
           }
          }
          else 
            QLOG_ERROR() << "CameraGiGE::setFeature> cannot create new stream !";
          for (int i = 0; i < GIGE_FRAME_NUMBER; i++)
            arv_stream_push_buffer (stream, arv_buffer_new (payload, NULL));
          arv_camera_set_acquisition_mode (camera, ARV_ACQUISITION_MODE_CONTINUOUS);
          arv_camera_start_acquisition (camera);
          this->start();
     }
     else {
        arv_gc_node_set_value_from_string (node, feature_string_value.toStdString().c_str());
        QLOG_INFO() << "CameraGiGE::setFeature> set feature "
                    << featureNameList.at(feature) << " New value " << feature_string_choice.at(value);
     }
  }
}
void
CameraGiGE::setMode(int feature, bool value) {
  switch ( feature ) {
   default:
   break;
 }

}
void
CameraGiGE::getFeatures() {
  
  device = arv_camera_get_device(camera);
  double feature_min_d, feature_max_d;
  gint64 feature_min_i, feature_max_i;
  int featureCnt = 0;
  for ( int i = 0; i < FEATURES_NUMBER; i++) {
   ArvGcNode *feature = NULL;
   feature = arv_device_get_feature (device, gige_features[i]);
   if ( feature == NULL ) continue;
   if ( ARV_IS_GC_FLOAT (feature) ) {
     QLOG_INFO() << "CameraGiGE::getFeature> get feature "
                 << gige_features[featureCnt];
     arv_device_get_float_feature_bounds (device, gige_features[i], &feature_min_d, &feature_max_d);
     //QLOG_INFO () << "Feature Min " << QString::number(feature_min_d) << " Max " << QString::number(feature_max_d);
     QString feature_string_value;
     feature_string_value = QString(arv_gc_node_get_value_as_string (feature));
     QLOG_INFO() << "CameraGiGE::getFeature> get feature "
                     << featureNameList.at(featureCnt) << " value " << feature_string_value;
     featureValueList.replace(featureCnt, feature_string_value.toDouble() );
     featureAbsValueList.replace(featureCnt, feature_string_value.toDouble() );
     featureMinList.replace(featureCnt,feature_min_d );
     featureMaxList.replace(featureCnt,feature_max_d );
   }
   else if ( ARV_IS_GC_INTEGER (feature) ) {
     QLOG_INFO() << "CameraGiGE::getFeature> get feature "
                 << gige_features[i];
     arv_device_get_integer_feature_bounds (device, gige_features[i], &feature_min_i, &feature_max_i);
     QString feature_string_value;
     feature_string_value = QString(arv_gc_node_get_value_as_string (feature));
     QLOG_INFO() << "CameraGiGE::getFeature> get feature "
                     << featureNameList.at(featureCnt) << " value " << feature_string_value;
     featureValueList.replace(featureCnt, feature_string_value.toInt());
     featureMinList.replace(featureCnt,feature_min_i);
     featureMaxList.replace(featureCnt,feature_max_i);
   }
   else if ( ARV_IS_GC_BOOLEAN (feature) ) {
     QLOG_INFO() << "CameraGiGE::getFeature> get feature "
                 << gige_features[i];
     QString feature_string_value;
     feature_string_value = QString(arv_gc_node_get_value_as_string (feature));
     QLOG_INFO() << "CameraGiGE::getFeature> get feature "
                     << featureNameList.at(featureCnt) << " value " << feature_string_value;
     if ( feature_string_value == "true" )
       featureValueList.replace(featureCnt, 1);
     else if ( feature_string_value == "false" )
       featureValueList.replace(featureCnt, 0);
   }
   else if ( ARV_IS_GC_ENUMERATION (feature) ) {
     const GSList *childs;
     const GSList *iter;
     childs = arv_gc_node_get_childs (feature);
     int feature_choicecnt = 0;
     for (iter = childs; iter != NULL; iter = iter->next) {
       if ( arv_gc_node_get_name ((ArvGcNode *)iter->data) == arv_gc_node_get_value_as_string (feature)) {
         QLOG_INFO() << "CameraGiGE::setCamera> get feature "
                     << featureNameList.at(featureCnt) << " value " << QString(arv_gc_node_get_value_as_string (feature));
         featureValueList.replace(featureCnt,feature_choicecnt);
         break;
       }
       feature_choicecnt++;
     }
   }
   featureCnt++;
  }
  emit updateFeatures();
}
void 
CameraGiGE::getProps()  {

  device = arv_camera_get_device(camera);
  double feature_min, feature_max;
  int propCnt = 0, i = 0;
  QString propStr;
  
  for ( i = 0; i < PROPS_NUMBER - 1; i++) {
   ArvGcNode *feature = NULL;
   feature = arv_device_get_feature (device, gige_props[i]);
   if ( feature == NULL ) continue;
   QString feature_string_value;
   feature_string_value = QString(arv_gc_node_get_value_as_string (feature));
   propStr = gige_props[i];
   QLOG_DEBUG () << "CameraGiGE::getProps> " << propStr << " updated "
                << feature_string_value;
   if ( ARV_IS_GC_FLOAT (feature) ) {
     double value = feature_string_value.toDouble();
     propStr.append(" : " + QString::number(value,'f',3) 
                          + " " + gige_props_units[i]);
     propList.replace(propCnt++,propStr);
   }
   else if ( ARV_IS_GC_INTEGER (feature) ) {
     int value = feature_string_value.toInt();
     propStr.append(" : " + QString::number(value)
                          + " " + gige_props_units[i]);
     propList.replace(propCnt++,propStr);
   }
   else if ( ARV_IS_GC_BOOLEAN (feature) ) {
      propStr.append(" : " + feature_string_value
                           + " " + gige_props_units[i]);
     propList.replace(propCnt++,propStr);
   }
   else if ( ARV_IS_GC_ENUMERATION (feature) ) {
     propStr.append(" : " + feature_string_value 
                          + " " + gige_props_units[i]);
     propList.replace(propCnt++,propStr);
   }
  }
  // PC Rate prop
  propStr = gige_props[i];
  propStr.append(" : " + QString::number((int)frequency) + " " + gige_props_units[i]);
  QLOG_DEBUG () << "CameraGiGE::getProps> " << propStr << " updated ";
  propList.replace(propCnt,propStr);
  emit updateProps();
}

int 
CameraGiGE::findCamera() {
   int i;
   /* Find available gigevision cameras*/
   arv_update_device_list ();
   num = arv_get_n_devices ();
   // Allocate camera list
   for (i = 1 ; i < num; i++) {
     camera =  arv_camera_new (arv_get_device_id (i));
     if (!camera) {
      QLOG_ERROR() << "CameraGiGE::findCamera> Failed to initialize camera with guid " << arv_get_device_id (i);
      return -1;
     }
     cameralist.push_back((ArvCamera*)camera);
     vendorlist.push_back(arv_camera_get_vendor_name (camera));
     modelist.push_back(arv_camera_get_model_name (camera));
   }
   num--; // Remove Aravis Fake camera
   return (0);
}
int 
CameraGiGE::connectCamera() {
  
  camera_err = -1;
  
  /* Init camera*/
  arv_camera_get_region (camera, &x, &y, &gwidth, &gheight);
  width = gwidth;
  height = gheight;
  arv_camera_get_binning (camera, &dx, &dy);
  payload = arv_camera_get_payload (camera);
   
  stream = arv_camera_create_stream (camera, NULL, NULL);
  if (stream != NULL) {
    if (ARV_IS_GV_STREAM (stream)) {
       	g_object_set (stream,
          	      "socket-buffer", ARV_GV_STREAM_SOCKET_BUFFER_AUTO,
		      "socket-buffer-size", 0,
		      NULL);
	g_object_set (stream,
		      "packet-resend", ARV_GV_STREAM_PACKET_RESEND_NEVER,
		      NULL);
	g_object_set (stream,
	              "packet-timeout", 20 * 1000,
	              "frame-retention", 100 * 1000,
	              NULL);
    
   }
   else  {
	g_object_unref (camera);
	camera = NULL;
	return camera_err;
   } 
   for (int i = 0; i < GIGE_FRAME_NUMBER; i++)
      arv_stream_push_buffer (stream, arv_buffer_new (payload, NULL));

  /*-----------------------------------------------------------------------
   *  setup capture
   *-----------------------------------------------------------------------*/
 
  buffer = (uchar*)malloc( sizeof(uchar) * width * height);
  snapshot = (uchar*)malloc( sizeof(uchar) * width * height);
  buffer32 = (int*)malloc( sizeof(int) * width * height);
  snapshot32 = (int*)malloc( sizeof(int) * width * height);

  image = new QImage(buffer,width,height,width,QImage::Format_Indexed8);
  image->setColorTable(*table);

  /*-----------------------------------------------------------------------
   *  have the camera start sending data
   *-----------------------------------------------------------------------*/
   arv_camera_set_acquisition_mode (camera, ARV_ACQUISITION_MODE_CONTINUOUS);
   arv_camera_start_acquisition (camera);

  // Refresh properties
  getProps();
  // Refresh features
  getFeatures();

  }
  return 0;
}

void  
CameraGiGE::cleanup_and_exit()
{
  QLOG_DEBUG() << "Cleanup camera GiGE connection";
  if (camera)
   arv_camera_stop_acquisition (camera);
  g_object_unref (stream);
  g_object_unref (camera); 
  arv_shutdown ();
  if (buffer) { free(buffer); buffer = NULL;}
  if (snapshot) { free(snapshot); snapshot = NULL;}
  if (buffer32) { free(buffer32); buffer32 = NULL;}
  if (snapshot32) { free(snapshot32); snapshot32 = NULL;}
  if (image) delete image;
  return;
}

int 
CameraGiGE::acquireImage() {
   
  /*-----------------------------------------------------------------------
   * acquire frame
   *-----------------------------------------------------------------------*/
  uchar *pBuf;
  arvbuffer = arv_stream_pop_buffer (stream);
  while (arvbuffer == NULL) 
   arvbuffer = arv_stream_pop_buffer (stream);
  QLOG_DEBUG () << "Buffer data size " << arvbuffer->size;
  QLOG_DEBUG () << "Pixel Encoding " << pixel_encoding;
  if (arvbuffer->status != ARV_BUFFER_STATUS_SIZE_MISMATCH) { 
  /* copy captured image */
  snapshotMutex->lock();
  ushort *tmpBuf;
  pBuf = reinterpret_cast<uchar*>(arvbuffer->data);
  // calculate min,max
    max = 0;
    min = 65535;
    if ( pixel_encoding == B8 ) {
      for (unsigned int i = 0 ; i < width * height; i++) {
        if (pBuf[i] > max)
          max = pBuf[i];
        if (pBuf[i] < min)
          min = pBuf[i];
      }
    }
    if ( pixel_encoding == B10 ) {
      tmpBuf = reinterpret_cast<ushort*>(pBuf);
      for (unsigned int i = 0 ; i < width * height; i++) {
        ushort ui_current = *(tmpBuf++) & 0x03FF;
        if (ui_current > max)
          max = ui_current;
        if (ui_current < min)
          min = ui_current;
      }
    }
    if ( pixel_encoding == B12 ) {
      tmpBuf = reinterpret_cast<ushort*>(pBuf);
      for (unsigned int i = 0 ; i < width * height; i++) {
        ushort ui_current = *(tmpBuf++) & 0x07FF;
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
      memcpy(buffer,pBuf,width * height * sizeof(uchar));
      for (unsigned int i = 0 ; i < width * height; i++)
        buffer32[i] = buffer[i];
    }
    if ( pixel_encoding == B10 ) {
      tmpBuf = reinterpret_cast<ushort*>(pBuf);
      for (unsigned int i = 0 ; i < width * height; i++) {
        ushort ui_current = *(tmpBuf++) & 0x03FF;
        buffer32[i] = ui_current;
        if ( (max - min) != 0 )
          buffer[i] = (uchar) (( 255 * (ui_current - min) ) / (max - min));
        else
          buffer[i] = 255;
      }
    }
    if ( pixel_encoding == B12 ) {
      tmpBuf = reinterpret_cast<ushort*>(pBuf);
      for (unsigned int i = 0 ; i < width * height; i++) {
        ushort ui_current = *(tmpBuf++) & 0x07FF;
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
    if (hflip) {
     buffer = fliphorizontal(buffer,height*width, width);
     buffer32 = fliphorizontal(buffer32,height*width, width);
    }
  snapshotMutex->unlock();
  image->loadFromData (buffer,width * height);
  
  // emit visualisation image
  QImage imagescaled = image->scaled(imageWidth,imageHeight);
  QImage imagergb32 =  imagescaled.convertToFormat(QImage::Format_ARGB32_Premultiplied);
  
  emit getImage(imagergb32);
  emit updateMin(min);
  emit updateMax(max);
  }
  else
   QLOG_ERROR() << " Arv buffer Error " << arvbuffer->status;
  /*-----------------------------------------------------------------------
   * release frame
   *-----------------------------------------------------------------------*/
  arv_stream_push_buffer (stream, arvbuffer);
 
  return (1);
}

void CameraGiGE::setImageSize(const int &_imageWidth, const int &_imageHeight){
  
  imageWidth = _imageWidth;
  imageHeight = _imageHeight;
  QLOG_DEBUG ( ) <<  "emit new image with size " << imageWidth  << "x" << imageHeight;
}
#endif
