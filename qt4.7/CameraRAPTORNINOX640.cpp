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
#ifdef RAPTORNINOX640CAMERA
#include "CameraRAPTORNINOX640.h"
#include <sys/time.h>

char *raptorninox640_features[]  = {
                        (char*)"Frame Rate (Hz)",
                        (char*)"Exposure (millisecs)",
                        (char*)"Analog Gain",
                        (char*)"Digital Gain (dB)",
                        (char*)"NUC"
                        
                        };
char *raptorninox640_props[]  = {
                        (char*)"PCB Temp.",
			(char*)"CMOS Temp.",
                        (char*)"Image Size",
                        (char*)"Analog Gain",
                        (char*)"Digital Gain",
                        (char*)"NUC",
                        (char*)"Exposure",
                        (char*)"PC Rate",
                        (char*)"Frame Rate"
                        };

char *raptorninox640_gains[]  = {
                        (char*)"Low",
                        (char*)"High"
                        };

char *raptorninox640_nuc[]  = {
                        (char*)"Normal (Raw)",
                        (char*)"Offset+Bad Pixel",
                        (char*)"Offset+Gain+Bad Pixel",
                        (char*)"Offset+Gain+Dark+Bad Pixel",
                        };

int raptorninox640_aoi_settings[][4] = {
                       {0,640,0,512}
                       };

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

CameraRAPTORNINOX640::CameraRAPTORNINOX640()
  :Camera()
{
  hflip = 0;
  vflip = 0;
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
  modeCheckEnabled = true;
  binning_changed = false;
  aoi_changed = false;
  adcCal0 = 1240;
  adcCal40 = 789;
  dacCal0 = 1622;
  dacCal40 = 2526;
}

CameraRAPTORNINOX640::~CameraRAPTORNINOX640()
{
  QLOG_DEBUG() << "Deleting CameraRAPTORNINOX640";
  stop();
  cleanup_and_exit();
  delete mutex;
  delete snapshotMutex;
  delete acquireMutex;
  delete acqstart;
  delete acqend;
}
void
CameraRAPTORNINOX640::setCamera(void* _camera, int _id)
{
  /* Init camera*/
  vendor = "RaptorNinox640x640";
  model = "Raptor Ninox640";
  camera_err = connectCamera();

  QLOG_INFO() << "CameraRAPTORNINOX640::setCamera " << vendor << " model : "
              << model << " - Err : " << camera_err;
 
 int featureCnt = 0;
 // Frame Rate feature
 int frate_min, frate_max;
 featureIdList.push_back(featureCnt);
 featureNameList.push_back(raptorninox640_features[featureCnt]);
 frate_min = 1;
 frate_max = 120;
 featureMinList.push_back(frate_min);
 featureMaxList.push_back(frate_max);
 featureValueList.push_back( (int)frate );
 featureAbsCapableList.push_back(true);
 featureAbsValueList.push_back((int)frate);
 featureModeAutoList.push_back(false);
// Exposure feature
 int exp_min, exp_max;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raptorninox640_features[featureCnt]);
 exp_min = 1;
 exp_max = 1000;
 featureMinList.push_back(exp_min);
 featureMaxList.push_back(exp_max);
 int exposure = (int)getExposure();
 featureValueList.push_back( exposure );
 featureAbsCapableList.push_back(true);
 featureAbsValueList.push_back( exposure );
 featureModeAutoList.push_back(false);
 // Analog Gain feature
 int g_min, g_max;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raptorninox640_features[featureCnt]);
 g_min = 0;
 g_max = 1;
 featureMinList.push_back(g_min);
 featureMaxList.push_back(g_max);
 int gain = (int)getGain();
 featureValueList.push_back( gain );
 featureAbsCapableList.push_back(true);
 featureAbsValueList.push_back( gain );
 featureModeAutoList.push_back(false);
 // Digital Gain feature
 int dg_min, dg_max;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raptorninox640_features[featureCnt]);
 dg_min = 0;
 dg_max = 48;
 featureMinList.push_back(dg_min);
 featureMaxList.push_back(dg_max);
 int dgain = (int)getDGain();
 featureValueList.push_back( dgain );
 featureAbsCapableList.push_back(true);
 featureAbsValueList.push_back( dgain );
 featureModeAutoList.push_back(false);
 // NUC feature
 int nuc_min, nuc_max;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raptorninox640_features[featureCnt]);
 nuc_min = 0;
 nuc_max = 3;
 featureMinList.push_back(nuc_min);
 featureMaxList.push_back(nuc_max);
 int nuc = (int)getNUC();
 featureValueList.push_back( nuc );
 featureAbsCapableList.push_back(true);
 featureAbsValueList.push_back( nuc );
 featureModeAutoList.push_back(false);


 // Properties
 int propCnt = 0;

 // PCB Temperature prop
  QString pcbtempStr = raptorninox640_props[propCnt];
  pcbtempStr.append(" : " + QString::number(getPCBtemperature()));
  pcbtempStr.append(" C");
  propList.push_back(pcbtempStr);
  
  // CCD Temperature prop
  QString ccdtempStr = raptorninox640_props[++propCnt];
  ccdtempStr.append(" : " + QString::number(getCCDtemperature()));
  ccdtempStr.append(" C");
  propList.push_back(ccdtempStr);

  // Image Size prop
  QString imgSizeStr = raptorninox640_props[++propCnt];;
  imgSizeStr.append(" : " + QString::number(pxd_imageXdim()) + "x" + QString::number(pxd_imageYdim()));
  imgSizeStr.append(" pixels");
  propList.push_back(imgSizeStr);

  // Analog Gain prop
  QString gainStr = raptorninox640_props[++propCnt];
  gainStr.append(" : " + QString(raptorninox640_gains[gain]));
  propList.push_back(gainStr);

  // Digital Gain prop
  QString dgainStr = raptorninox640_props[++propCnt];
  dgainStr.append(" : " + QString::number(dgain));
  dgainStr.append(" dB");
  propList.push_back(dgainStr);

  // NUC prop
  QString nucStr = raptorninox640_props[++propCnt];
  nucStr.append(" : " + QString(raptorninox640_nuc[nuc]));
  propList.push_back(nucStr);

  // Exposure prop
  QString exposureStr = raptorninox640_props[++propCnt];
  exposureStr.append(" : " + QString::number(exposure));
  exposureStr.append(" ms");
  propList.push_back(exposureStr);

  // PC Rate prop
  QString freqStr = raptorninox640_props[++propCnt];;
  freqStr.append(" : " + QString::number((int)frequency));
  freqStr.append(" Hz");
  propList.push_back(freqStr);

  // Frame Rate prop
  QString frateStr = raptorninox640_props[++propCnt];;
  frateStr.append(" : " + QString::number((int)frate));
  frateStr.append(" Hz");
  propList.push_back(frateStr);

  pixel_encoding = B14;
}

uchar* 
CameraRAPTORNINOX640::getSnapshot() {
  snapshotMutex->lock();
  memcpy(snapshot,buffer,width * height * sizeof(uchar));
  snapShotMin = min;
  snapShotMax = max;
  snapshotMutex->unlock();
  return snapshot;
}

int*
CameraRAPTORNINOX640::getSnapshot32() {
  snapshotMutex->lock();
  QLOG_INFO() << "CameraRAPTORNINOX640::getSnapshot32()> width " << width << "height " << height;
  memcpy(snapshot32,buffer32, width * height * sizeof(int));
  snapShotMin = min;
  snapShotMax = max;
  snapshotMutex->unlock();
  return snapshot32;
}
void 
CameraRAPTORNINOX640::stop() {
  suspend = true;
  has_started = false;
  wait();
  exit();  
}

void 
CameraRAPTORNINOX640::run() {
  int acq_err = 0;
  int acq_cnt = 0;
  if (camera_err == 0 && suspend == true ) {
    suspend = false;
    eTimeTotal = 0;
    frameTotal = pxd_videoFieldCount(1);
    while (suspend == false) {
      QLOG_DEBUG () << "CameraRAPTORNINOX640::run> " << id << " : start new Acquisition";
      double eTime = GetTime();
      acqstart->wakeAll();
      acq_err = acquireImage();
      QLOG_DEBUG () << "CameraRAPTORNINOX640::run> " << id << " : done";
      acqend->wakeAll();
      if (acq_err == 1)
       eTimeTotal+= (GetTime() - eTime);
      acq_cnt++;
      if (acq_cnt == FREQUENCY_AVERAGE_COUNT) {
       eTimeTotal/=1e6;
       if ( eTimeTotal > 0 ) {
        frequency = FREQUENCY_AVERAGE_COUNT / eTimeTotal;
        frameTotal = pxd_videoFieldCount(1) - frameTotal;
        frate = frameTotal  / eTimeTotal;
       }
       else {
        frequency = 0;
        frameTotal = 0;
       }
       QLOG_DEBUG() << "CameraRAPTORNINOX640::run> Acquisition "
                   << "freq " << (int) frequency << " Hz";
       eTimeTotal = 0;
       frameTotal = pxd_videoFieldCount(1);
       acq_cnt = 0;
       // Update props
       //getProps();
     }
     has_started = true;
    }
    QLOG_DEBUG() << "CameraRAPTORNINOX640 thread exiting";
  }
}

void
CameraRAPTORNINOX640::setFeature(int feature, double value) {

  QLOG_INFO() << "CameraRAPTORNINOX640::setFeature> Update feature " << QString(raptorninox640_features[feature])
               << " value " << QString::number(value);
  int err = 0;
  int bin_width,bin_height;
  
  switch ( feature ) {
   case 0:
     setFramerate(value);
     break;
   case 1:
     setExposure(value);
     break;
   case 2:
     setGain(value);
     break;
   case 3:
     setDGain(value);
     break;
   case 4:
     setNUC(value);
     break;
   default:
     break;
  }
  getProps();
}
void  
CameraRAPTORNINOX640::setMode(int feature, bool value) {
}
void
CameraRAPTORNINOX640::getFeatures() {
  /*-----------------------------------------------------------------------
   *  report camera's features
   *-----------------------------------------------------------------------*/
 
 int featureCnt = 0;
 // Frame rate feature
 featureAbsValueList.replace(featureCnt, (int)frate );
 // Exposure feature
 featureAbsValueList.replace(++featureCnt, (int)getExposure() );
 // Analog Gain feature
 featureAbsValueList.replace(++featureCnt, (int)getGain() );
 // Digital Gain feature
 featureAbsValueList.replace(++featureCnt, (int)getDGain() );
 // NUC feature
 featureAbsValueList.replace(++featureCnt, (int)getNUC() );

 emit updateFeatures();
}
void 
CameraRAPTORNINOX640::getProps()  {

// Properties
  int propCnt = 0;

  // PCB Temperature prop
  QString pcbtempStr = raptorninox640_props[propCnt];
  pcbtempStr.append(" : " + QString::number(getPCBtemperature()));
  pcbtempStr.append(" C");
  propList.replace(propCnt,pcbtempStr);
 
  // CCD Temperature prop
  QString ccdtempStr = raptorninox640_props[++propCnt];
  ccdtempStr.append(" : " + QString::number(getCCDtemperature()));
  ccdtempStr.append(" C");
  propList.replace(propCnt,ccdtempStr);
  
  // Image Size prop
  QString imgSizeStr = raptorninox640_props[++propCnt];;
  imgSizeStr.append(" : " + QString::number(pxd_imageXdim()) + "x" + QString::number(pxd_imageYdim()));
  imgSizeStr.append(" pixels");
  propList.replace(propCnt,imgSizeStr);

  // Analog Gain prop
  QString gainStr = raptorninox640_props[++propCnt];
  gainStr.append(" : " + QString(raptorninox640_gains[getGain()]));
  propList.replace(propCnt,gainStr);

  // Digital Gain prop
  QString dgainStr = raptorninox640_props[++propCnt];
  dgainStr.append(" : " + QString::number(getDGain()));
  dgainStr.append(" dB");
  propList.replace(propCnt,dgainStr);

  // NUC prop
  QString nucStr = raptorninox640_props[++propCnt];
  nucStr.append(" : " + QString(raptorninox640_nuc[getNUC()]));
  propList.replace(propCnt,nucStr);

  // Exposure prop
  QString exposureStr = raptorninox640_props[++propCnt];
  exposureStr.append(" : " + QString::number(getExposure()));
  exposureStr.append(" ms");
  propList.replace(propCnt,exposureStr);

  // PC Rate
  QString freqStr = raptorninox640_props[++propCnt];;
  freqStr.append(" : " + QString::number((int)frequency));
  freqStr.append(" Hz");
  propList.replace(propCnt, freqStr);

  // Frame Rate prop
  QString frateStr = raptorninox640_props[++propCnt];;
  frateStr.append(" : " + QString::number((int)frate));
  frateStr.append(" Hz");
  propList.replace(propCnt,frateStr);

  QLOG_DEBUG() << "CameraRAPTORNINOX640::getProps> Properties updated";
  emit updateProps();

}
int 
CameraRAPTORNINOX640::findCamera() {

  int err = -1;
  QLOG_INFO() << "Open Raptor Ninox640";
  err = pxd_PIXCIopen("", "", "/usr/local/xcap/data/nino640.fmt");
  QLOG_INFO() << "Open Raptor Ninox640" << err;
  pxd_mesgFault(1);
  if (err < 0) {
    num = 0;
    pxd_PIXCIclose();
    return 0;
  }
  int camera = 1; // Camera internal ID
  cameralist.push_back((int*)&camera);
  vendorlist.push_back("Ninox640");
  modelist.push_back("Raptor");
  num = 1; // Camera number

  return 0;
}
int 
CameraRAPTORNINOX640::connectCamera() {

  int err;

  QLOG_INFO() << "Connect serial to Raptor";
  err = pxd_serialConfigure(0x1, 0, 115200, 8, 0, 1, 0, 0, 0);
  QLOG_INFO() << "Connect serial to Raptor" << err;

  char readadrs_v[]   = { 0x56, 0x50 };
  uchar resp_v[] = { 0x0, 0x0 };
  readFeature (readadrs_v, 0, readadrs_v, 2, resp_v, 2);
  QLOG_INFO() << "Found Raptor camera Micro Version V" << resp_v[0] << "." << resp_v[1];

  //readManufacturingInfo();

  /*-----------------------------------------------------------------------
   *  setup capture
   *-----------------------------------------------------------------------*/

  QLOG_INFO() << "CameraRAPTORNINOX640::connectCamera()> Xdim=" << pxd_imageXdim();
  QLOG_INFO() << "CameraRAPTORNINOX640::connectCamera()> Ydim=" << pxd_imageYdim();
  QLOG_INFO() << "CameraRAPTORNINOX640::connectCamera()> Cdim=" << pxd_imageCdim();
  QLOG_INFO() << "CameraRAPTORNINOX640::connectCamera()> Bdim=" << pxd_imageBdim();

  // Init AOI and buffers
  width = pxd_imageXdim();
  height =  pxd_imageYdim();
  QLOG_INFO() << "CameraRAPTORNINOX640::connectCamera> Updated width " << width;
  QLOG_INFO() << "CameraRAPTORNINOX640::connectCamera> Updated height " << height;
  buffer = (uchar*)malloc( sizeof(uchar) * width * height);
  snapshot = (uchar*)malloc( sizeof(uchar) * width * height);
  buffer32 = (int*)malloc( sizeof(int) * width * height);
  snapshot32 = (int*)malloc( sizeof(int) * width * height);
  image16 = (ushort*)malloc( sizeof(ushort) * width * height);
  image = new QImage(buffer,width,height,width,QImage::Format_Indexed8);
  image->setColorTable(*table);

  /*-----------------------------------------------------------------------
   *  have the camera start sending data
   *-----------------------------------------------------------------------*/
  // err = pxd_goLive(1, framenum);
  // if (err < 0)
  //    QLOG_WARN() << "pxd_goLive:" <<  pxd_mesgErrorCode(err) ;

  return 0;

}
void  
CameraRAPTORNINOX640::cleanup_and_exit()
{
  int err;
  err = pxd_PIXCIclose();
  QLOG_DEBUG() << "Close Raptor" << err;
  if (buffer) { free(buffer); buffer = NULL;}
  if (snapshot) { free(snapshot); snapshot = NULL;}
  if (buffer32) { free(buffer32); buffer32 = NULL;}
  if (snapshot32) { free(snapshot32); snapshot32 = NULL;}
  if (image16) { free(image16); image16 = NULL;}
  if (image) delete image;

  return;
}

int 
CameraRAPTORNINOX640::acquireImage() {

    // Capture a new frame
    // Capture a new frame
    while (acquireMutex->tryLock() == false) {
       usleep(100);
    }
      
    int err = pxd_doSnap(1,1,2000);
    if (err < 0)
      QLOG_WARN() << "pxd_doSnap:" <<  pxd_mesgErrorCode(err) ;
    err = pxd_readushort(1, 1, 0, 0, width, height, image16, (size_t)(width * height), "Grey");
    if (err < 0)
      QLOG_WARN() << "pxd_readushort:" <<  pxd_mesgErrorCode(err) ;
    else if ( err != pxd_imageXdim() * pxd_imageYdim())
      QLOG_DEBUG() << "pxd_readushort  missing pixels" << err << "!= " << pxd_imageXdim() * pxd_imageYdim();
    // Copy image buffer
    snapshotMutex->lock();
    // calculate min,max
    max = 0;
    min = 65535;
    // Treat Mono16 case
    for (int i = 0; i < height * width; i++) {
     // QLOG_DEBUG() << "CameraRAPTORNINOX640::acquireImage()> image16[" << i << "]=" << image16[i];
      if (image16[i] < min) {
        min = image16[i];
      }
      else if (image16[i] > max) {
        max = image16[i];
      }
    }
    // Treat Mono16 case
    for (int i = 0; i < height * width; i++) {
      buffer32[i] = image16[i];
      if ( (max - min) != 0 )
       buffer[i] = (uchar) (( 255 * (image16[i] - min) ) / (max - min));
      else
       buffer[i] = 255;
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
    // Format video image
    image->loadFromData (buffer,width * height);
    QImage imagescaled = image->scaled(imageWidth,imageHeight);
    QImage imagergb32 =  imagescaled.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    emit getImage(imagergb32);
    emit updateMin(min);
    emit updateMax(max);
    acquireMutex->unlock();
    return (1);
}

void CameraRAPTORNINOX640::setImageSize(const int &_imageWidth, const int &_imageHeight){
  imageWidth = _imageWidth;
  imageHeight = _imageHeight;
  QLOG_DEBUG ( ) <<  "emit new image with size " << imageWidth  << "x" << imageHeight;
}
int CameraRAPTORNINOX640::readFeature(char* sendreg, int size_sreg, char* readreg, int size_rreg, uchar* data, int size_data) {
  int error = 0;
  uchar tmp[] = { 0x0 };

  usleep(5000);
  while (pxd_serialRead(0x1, 0, (char*)tmp, 1) == 1) {
    QLOG_DEBUG() << "CameraRAPTORNINOX640::readFeature> tmp=" << tmp[0];
  }
  usleep(5000);
  error = pxd_serialWrite(0x1, 0, (char*)sendreg, size_sreg);
  if ( error < 0 ) {
   QLOG_WARN() << "CameraRAPTORNINOX640::readFeature> error=" << error;
   return error;
  }
  usleep(5000);
  while (pxd_serialRead(0x1, 0, (char*)tmp, 1) == 1) {
      QLOG_DEBUG() << "CameraRAPTORNINOX640::readFeature> tmp=" << tmp[0];
  }
  usleep(5000);
  error = pxd_serialWrite(0x1, 0, (char*)readreg, size_rreg);
  if ( error < 0 ) {
     QLOG_WARN() << "CameraRAPTORNINOX640::readFeature> error=" << error;
     return error;
  }
  usleep(5000);
  error = pxd_serialRead(0x1, 0, (char*)data, size_data);
  if ( error < 0 ) {
   QLOG_WARN() << "CameraRAPTORNINOX640::readFeature> pxd_serialRead error=" << error;
   return error;
  }
  for (int i= 0; i < size_data; i++)
    QLOG_DEBUG() << "CameraRAPTORNINOX640::readFeature> reads: " << " data[" << i << "]=" << data[i];

  while (pxd_serialRead(0x1, 0, (char*)tmp, 1) == 1) {
      QLOG_DEBUG() << "CameraRAPTORNINOX640::readFeature> tmp=" << tmp[0];
  }
  return error;
}
int CameraRAPTORNINOX640::writeFeature(char* sendreg, int size_sreg) {
  int error;
  uchar tmp[] = { 0x0 };

  usleep(5000);
  while (pxd_serialRead(0x1, 0, (char*)tmp, 1) == 1) {
    QLOG_DEBUG() << "CameraRAPTORNINOX640::readFeature> tmp=" << tmp[0];
  }
  usleep(5000);
  error = pxd_serialWrite(0x1, 0, sendreg, size_sreg);
  if ( error < 0 ) {
   QLOG_WARN() << "CameraRAPTORNINOX640::writeFeature> error=" << error;
   return error;
  }
  usleep(5000);
  while (pxd_serialRead(0x1, 0, (char*)tmp, 1) == 1) {
    QLOG_DEBUG() << "CameraRAPTORNINOX640::readFeature> tmp=" << tmp[0];
  }
 return error;
}

double CameraRAPTORNINOX640::getPCBtemperature() {
  QLOG_DEBUG() << "Get Raptor PCB temperature";
  char setadrs1[]   = { 0x53, 0xE0, 0x01, 0x70, 0x50 };
  char setadrs2[]   = { 0x53, 0xE0, 0x01, 0x71, 0x50 };
  char readadrs[]  = { 0x53, 0xE1, 0x01, 0x50 };
  uchar resp1[] = { 0x0 };
  uchar resp2[] = { 0x0 };
  readFeature (setadrs1, 5, readadrs, 4, resp1, 1);
  readFeature (setadrs2, 5, readadrs, 4, resp2, 1);
  int  t1 = ((resp1[0]&0x0F)<<8) | (resp2[0]&0xFF);
  double t2 = t1/16.0;
  QLOG_DEBUG() << "pcbtemp =" <<  t2;
  return t2;
}
double CameraRAPTORNINOX640::getDGain() {
  QLOG_DEBUG() << "Get Raptor Digital gain";
  char setadrs1[]   = { 0x53, 0xE0, 0x01, 0xC6, 0x50 };
  char setadrs2[]   = { 0x53, 0xE0, 0x01, 0xC7, 0x50 };
  char readadrs[]   = { 0x53, 0xE1, 0x01, 0x50 };
  uchar resp1[] = { 0x0 };
  uchar resp2[] = { 0x0 };
  readFeature (setadrs1, 5, readadrs, 4, resp1, 1);
  readFeature (setadrs2, 5, readadrs, 4, resp2, 1);
  int  g = ((resp1[0])<<8) | (resp2[0]&0xFF);
  QLOG_INFO() << "CameraRAPTORNINOX640::getDGain()> receives Digital gain =" <<  g;
  double dbgain = round((log10f(g)-log10f(256.0))*20.);
  QLOG_INFO() << "CameraRAPTORNINOX640::getDGain()> Digital gain =" <<  dbgain;
  return dbgain;
}
void CameraRAPTORNINOX640::setDGain(int g) {
  QLOG_DEBUG() << "Set Raptor Digital gain";
  char setadrs1[]   = { 0x53, 0xE0, 0x02, 0xC6, 0, 0x50 };
  char setadrs2[]   = { 0x53, 0xE0, 0x02, 0xC7, 0, 0x50 };
  int dn_gain = round(pow10f((double)(g)/20.0)*256.0);
  setadrs1[4] = (dn_gain>>8);
  setadrs2[4] = (dn_gain&0xFF);
  writeFeature (setadrs1, 6);
  writeFeature (setadrs2, 6);
  QLOG_INFO() << "CameraRAPTORNINOX640::setDGain()> Sends Digital gain =" << dn_gain ;
}
void CameraRAPTORNINOX640::setAOI(int left, int width, int top, int height) {
  QLOG_DEBUG() << "CameraRAPTORNINOX640::setAOI> Set Raptor AOI left " << left 
      << " width " << width << " top " << top << " height " << height;
  char setadrs1[]   = { 0x53, 0xE0, 0x02, 0xDD, 0, 0x50 };
  char setadrs2[]   = { 0x53, 0xE0, 0x02, 0xDE, 0, 0x50 };
  char setadrs3[]   = { 0x53, 0xE0, 0x02, 0xDF, 0, 0x50 };
  char setadrs4[]   = { 0x53, 0xE0, 0x02, 0xE0, 0, 0x50 };
  char setadrs5[]   = { 0x53, 0xE0, 0x02, 0xD9, 0, 0x50 };
  char setadrs6[]   = { 0x53, 0xE0, 0x02, 0xDA, 0, 0x50 };
  char setadrs7[]   = { 0x53, 0xE0, 0x02, 0xDB, 0, 0x50 };
  char setadrs8[]   = { 0x53, 0xE0, 0x02, 0xDC, 0, 0x50 };
  setadrs1[4] = (left>>0);
  setadrs2[4] = (left>>8);
  setadrs3[4] = (width>>0);
  setadrs4[4] = (width>>8);
  setadrs5[4] = (top>>0);
  setadrs6[4] = (top>>8);
  setadrs7[4] = (height>>0);
  setadrs8[4] = (height>>8);
  writeFeature (setadrs1, 6);
  writeFeature (setadrs2, 6);
  writeFeature (setadrs3, 6);
  writeFeature (setadrs4, 6);
  writeFeature (setadrs5, 6);
  writeFeature (setadrs6, 6);
  writeFeature (setadrs7, 6);
  writeFeature (setadrs8, 6);
}
double CameraRAPTORNINOX640::getExposure() {
  QLOG_DEBUG() << "Get Raptor Ninox640 exposure";
  char setadrs1[]   = { 0x53, 0xE0, 0x01, 0xEE, 0x50 };
  char setadrs2[]   = { 0x53, 0xE0, 0x01, 0xEF, 0x50 };
  char setadrs3[]   = { 0x53, 0xE0, 0x01, 0xF0, 0x50 };
  char setadrs4[]   = { 0x53, 0xE0, 0x01, 0xF1, 0x50 };
  char readadrs[]   = { 0x53, 0xE1, 0x01, 0x50 };
  uchar resp1[] = { 0x0 };
  uchar resp2[] = { 0x0 };
  uchar resp3[] = { 0x0 };
  uchar resp4[] = { 0x0 }; 
  readFeature (setadrs1, 5, readadrs, 4, resp1, 1);
  readFeature (setadrs2, 5, readadrs, 4, resp2, 1);
  readFeature (setadrs3, 5, readadrs, 4, resp3, 1);
  readFeature (setadrs4, 5, readadrs, 4, resp4, 1);
  double e = ((resp1[0]&0x0F)<<24) | ((resp2[0]&0xFF)<<16) | ((resp3[0]&0xFF)<<8) | (resp4[0]&0xFF) ;
  QLOG_INFO() << "CameraRAPTORNINOX640::getExposure()> exposure =" <<  e / 40000.;
  return e / 40000.;
}
void CameraRAPTORNINOX640::setExposure(double e) {
  QLOG_INFO() << "CameraRAPTORNINOX640::setExposure> Set Raptor  Ninox640 exposure " << e;

  char setadrs1[]   = { 0x53, 0xE0, 0x02, 0xEE, 0, 0x50 };
  char setadrs2[]   = { 0x53, 0xE0, 0x02, 0xEF, 0, 0x50 };
  char setadrs3[]   = { 0x53, 0xE0, 0x02, 0xF0, 0, 0x50 };
  char setadrs4[]   = { 0x53, 0xE0, 0x02, 0xF1, 0, 0x50 };
  int value = (int)(e * 40.0E3);
  QLOG_INFO() << "CameraRAPTORNINOX640::setExposure> Writing exposure  =" <<  value;
  setadrs1[4] = (value>>24);
  setadrs2[4] = (value>>16);
  setadrs3[4] = (value>>8);
  setadrs4[4] = (value>>0);
  writeFeature (setadrs1, 6);
  writeFeature (setadrs2, 6);
  writeFeature (setadrs3, 6);
  writeFeature (setadrs4, 6);
  
  double esend = ((setadrs1[4]&0x0F)<<24) | ((setadrs2[4]&0xFF)<<16) | ((setadrs3[4]&0xFF)<<8) | (setadrs4[4]&0xFF) ;
  QLOG_INFO() << "CameraRAPTORNINOX640::setExposure> Sent exposure =" <<  esend / 40.0E3 << " milliseconds";
 

}
double CameraRAPTORNINOX640::getFramerate() { // Does not work..Miss conversion factor
  QLOG_DEBUG() << "Get Raptor Ninox640 Frame rate";
  char setadrs1[]   = { 0x53, 0xE0, 0x02, 0xDD, 0x50 };
  char setadrs2[]   = { 0x53, 0xE0, 0x02, 0xDE, 0x50 };
  char setadrs3[]   = { 0x53, 0xE0, 0x02, 0xDF, 0x50 };
  char setadrs4[]   = { 0x53, 0xE0, 0x02, 0xE0, 0x50 };
  char readadrs[]   = { 0x53, 0xE1, 0x01, 0x50 };
  uchar resp1[] = { 0x0 };
  uchar resp2[] = { 0x0 };
  uchar resp3[] = { 0x0 };
  uchar resp4[] = { 0x0 }; 
  readFeature (setadrs1, 5, readadrs, 4, resp1, 1);
  readFeature (setadrs2, 5, readadrs, 4, resp2, 1);
  readFeature (setadrs3, 5, readadrs, 4, resp3, 1);
  readFeature (setadrs4, 5, readadrs, 4, resp4, 1);
  double f = ((resp1[0]&0x0F)<<24) | ((resp2[0]&0xFF)<<16) | ((resp3[0]&0xFF)<<8) | (resp4[0]&0xFF) ;
  QLOG_INFO() << "CameraRAPTORNINOX640::getFramerate()> frame rate =" << 40.0E6 / f << " Hz";;
  return 40.0E6 / f;
}
void CameraRAPTORNINOX640::setFramerate(double f) {
  QLOG_INFO() << "CameraRAPTORNINOX640::setFramerate> Set Raptor frame rate (Hz) " << f;

  char setadrs1[]   = { 0x53, 0xE0, 0x02, 0xDD, 0, 0x50 };
  char setadrs2[]   = { 0x53, 0xE0, 0x02, 0xDE, 0, 0x50 };
  char setadrs3[]   = { 0x53, 0xE0, 0x02, 0xDF, 0, 0x50 };
  char setadrs4[]   = { 0x53, 0xE0, 0x02, 0xE0, 0, 0x50 };
  int value = (int)(40.0E6/f);
  QLOG_INFO() << "CameraRAPTORNINOX640::setFramerate> Writing frame period =" <<  value;
  setadrs1[4] = (value>>24);
  setadrs2[4] = (value>>16);
  setadrs3[4] = (value>>8);
  setadrs4[4] = (value>>0);
  writeFeature (setadrs1, 6);
  writeFeature (setadrs2, 6);
  writeFeature (setadrs3, 6);
  writeFeature (setadrs4, 6);
 
  double esend = ((setadrs1[4]&0x0F)<<24) | ((setadrs2[4]&0xFF)<<16) | ((setadrs3[4]&0xFF)<<8) | (setadrs4[4]&0xFF) ;
  QLOG_INFO() << "CameraRAPTORNINOX640::setFramerate> Sent frame rate =" <<  40.0E6 / esend << " Hz";
 
}
int CameraRAPTORNINOX640::getGain() {
  QLOG_DEBUG() << "Get Raptor Ninox640 gain (high/low)";
  char setadrs[]   = { 0x53, 0xE0, 0x01, 0xF2, 0x50 };
  char readadrs[]   = { 0x53, 0xE1, 0x01, 0x50 };
  uchar resp[] = { 0x0 };
  readFeature (setadrs, 5, readadrs, 4, resp, 1);
  QLOG_INFO() << "CameraRAPTORNINOX640::getGain()> gain =" <<  resp[0];
  int gain = 0;
  if ( resp[0] == 0x06 )
   gain = 1;
  else if ( resp[0] == 0x00 )
   gain = 0;
  return gain;
}
void CameraRAPTORNINOX640::setGain(int g) {
  QLOG_INFO() << "CameraRAPTORNINOX640::setExposure> Set Raptor  Ninox640 gain " << g;

  char setadrs[]   = { 0x53, 0xE0, 0x02, 0xF2, 0, 0x50 };
  if ( g == 0 )
    setadrs[4] = 0;
  else if ( g == 1 )
    setadrs[4] = 0x06;
  writeFeature (setadrs, 6);

}

int CameraRAPTORNINOX640::getNUC() {
  QLOG_DEBUG() << "Get Raptor Ninox640 NUC";
  char setadrs[]   = { 0x53, 0xE0, 0x01, 0xF9, 0x50 };
  char readadrs[]   = { 0x53, 0xE1, 0x01, 0x50 };
  uchar resp[] = { 0x0 };
  readFeature (setadrs, 5, readadrs, 4, resp, 1);
  QLOG_INFO() << "CameraRAPTORNINOX640::getNUC()> NUC =" <<  resp[0];
  int nuc = 0;
  if ( resp[0] == 0x40 )
   nuc = 0;
  else if ( resp[0] == 0x00 )
   nuc = 1;
  else if ( resp[0] == 0x20 )
   nuc = 2;
  else if ( resp[0] == 0x60 )
   nuc = 3;
  return nuc;
}
void CameraRAPTORNINOX640::setNUC(int n) {
  QLOG_INFO() << "CameraRAPTORNINOX640::setExposure> Set Raptor  Ninox640 NUC " << n;

  char setadrs[]   = { 0x53, 0xE0, 0x02, 0xF9, 0, 0x50 };
  if ( n == 0 )
    setadrs[4] = 0x40;
  else if ( n == 1 )
    setadrs[4] = 0x00;
  else if ( n == 2 )
    setadrs[4] = 0x20;
  else if ( n == 3 )
    setadrs[4] = 0x60;
  writeFeature (setadrs, 6);

}


double CameraRAPTORNINOX640::getCCDtemperature() {
  QLOG_DEBUG() << "Get Raptor CCD temperature";
  char setadrs1[]   = { 0x53, 0xE0, 0x01, 0x6E, 0x50 }; 
  char setadrs2[]   = { 0x53, 0xE0, 0x01, 0x6F, 0x50 };
  char readadrs[]  = { 0x53, 0xE1, 0x01, 0x50 };
  uchar resp1[] = { 0x0 };
  uchar resp2[] = { 0x0 };
  readFeature (setadrs1, 5, readadrs, 4, resp1, 1);
  readFeature (setadrs2, 5, readadrs, 4, resp2, 1);
  double x = 40.0 / (adcCal40-adcCal0);
  double y = (-40.0*adcCal0) / (adcCal40-adcCal0);
  double ccdTemp = (resp1[0]*256+resp2[0])*x+y;
  QLOG_DEBUG() << "ccdtemp =" <<  ccdTemp;
  return ccdTemp;
}
QString CameraRAPTORNINOX640::getBinningFactor() {
  QLOG_DEBUG() << "Get Raptor Binning";
  QString binStr;
  char setadrs[]   = { 0x53, 0xE0, 0x01, 0xEB, 0x50 };
  char readadrs[]  = { 0x53, 0xE1, 0x01, 0x50 };
  uchar resp[] = { 0x0 };
  readFeature (setadrs, 5, readadrs, 4, resp, 1);
  QLOG_INFO() << "binning =" <<  resp[0];
  if ( resp[0] == 0x0 )
   binStr = "1x1";
  else if ( resp[0] == 0x11 )
    binStr = "2x2";
  else if ( resp[0] == 0x22 )
    binStr = "3x3";
   else if ( resp[0] == 0x33 )
    binStr = "4x4";
  else if ( resp[0] == 0x44 )
    binStr = "5x5";
  return binStr;
}
void CameraRAPTORNINOX640::setBinningFactor(int value) {
  QLOG_DEBUG() << "Set Raptor Binning";
  char setadrs[]   = { 0x53, 0xE0, 0x02, 0xEB, 0x0, 0x50 };
  if ( value == 0 ) 
    setadrs[4] = 0x44;
  else if ( value == 1 )
    setadrs[4] = 0x33;
  else if ( value == 2 )
    setadrs[4] = 0x22;
  else if ( value == 3 )
    setadrs[4] = 0x11;
  else if ( value == 4 )
    setadrs[4] = 0x00;
  writeFeature (setadrs, 6);
}
QString CameraRAPTORNINOX640::getBloomState() {
  QString bloomState;
  QLOG_DEBUG() << "Get Raptor Bloom state";

  char setadrs1[]   = { 0x53, 0xE0, 0x01, 0x0F, 0x50 };
  char setadrs2[]   = { 0x53, 0xE0, 0x01, 0x10, 0x50 };
  char readadrs[]   = { 0x53, 0xE1, 0x01, 0x50 };
  uchar resp1[] = { 0x0 };
  uchar resp2[] = { 0x0 };

  readFeature (setadrs1, 5, readadrs, 4, resp1, 1);
  readFeature (setadrs2, 5, readadrs, 4, resp2, 1);

  if ( resp1[0] == 0x0A && resp2[0] == 0x8C )
   bloomState = "ON";
  else if ( resp1[0] == 0x09 && resp2[0] == 0x55 )
   bloomState = "OFF";

  return bloomState;
}
void CameraRAPTORNINOX640::setBloomState(int value) {
  QLOG_DEBUG() << "Set Raptor Bloom state";
  char setadrs1[]   = { 0x53, 0xE0, 0x02, 0x0F, 0x0A, 0x50 };
  char setadrs2[]   = { 0x53, 0xE0, 0x02, 0x10, 0x8C, 0x50 };
  char setadrs3[]   = { 0x53, 0xE0, 0x02, 0x0F, 0x09, 0x50 };
  char setadrs4[]   = { 0x53, 0xE0, 0x02, 0x10, 0x55, 0x50 };
  if ( value == 0 ) {
    writeFeature (setadrs3, 6);
    writeFeature (setadrs4, 6);
  }
  else if ( value == 1 ) {
    writeFeature (setadrs1, 6);
    writeFeature (setadrs2, 6);
  }
}
void CameraRAPTORNINOX640::updateFrameGrabberAOI(int v0, int v1, int v2, int v3) {
     // Update frame grabber AOI
     if (!(xc = pxd_xclibEscape(0, 0, 0)))
      QLOG_DEBUG() << "CameraRAPTORNINOX640::updateFrameGrabberAOI> Cannot update frame grabber state";
     
     xclib_DeclareVidStateStructs2(vidstate, pxd_infoModel(1));
     xclib_InitVidStateStructs2(vidstate, pxd_infoModel(1));
     xc->pxlib.getState(&xc->pxlib, 0, PXMODE_DIGI, &vidstate);
     vidstate.vidformat->xviddim[PXLHCM_MAX] = v1;
     vidstate.vidformat->xdatdim[PXLHCM_MAX] = v1;
     vidstate.vidformat->yviddim[PXLHCM_MAX] = v3;
     vidstate.vidformat->ydatdim[PXLHCM_MAX] = v3;
     vidstate.vidformat->xvidoffset[PXLHCM_MAX] = v1;
     vidstate.vidformat->yvidoffset[PXLHCM_MAX] = v3;
     vidstate.vidformat->xviddim[PXLHCM_MOD] = 0;
     vidstate.vidformat->xdatdim[PXLHCM_MOD] = 0;
     vidstate.vidformat->yviddim[PXLHCM_MOD] = 0;
     vidstate.vidformat->ydatdim[PXLHCM_MOD] = 0;
     vidstate.vidformat->is.hoffset = v0;
     vidstate.vidformat->is.voffset = v2;
     vidstate.vidres->x.setmaxdatsamples = 1;
     vidstate.vidres->x.setmaxvidsamples = 1;
     vidstate.vidres->y.setmaxdatsamples = 1;
     vidstate.vidres->y.setmaxvidsamples = 1;
     vidstate.vidres->setmaxdatfields    = 1;
     vidstate.vidres->setmaxdatphylds    = 1;
     xc->pxlib.defineState(&xc->pxlib, 0, PXMODE_DIGI, &vidstate);
     pxd_xclibEscaped(1, 0, 0);
}
int CameraRAPTORNINOX640::readManufacturingInfo() { // Does not work...
    char setadrs[] = { 0x53, 0xAE, 0x05, 0x01, 0x00, 0x00, 0x02, 0x00, 0x50};
    char readadrs[] = { 0x53, 0xAF, 0x12, 0x50};
    uchar data[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    readFeature (setadrs, 9, readadrs, 4, data, 18);
    QLOG_INFO() << "data = " << hex << data[0] <<  "/" << data[1] <<  "/" << data[2] <<  "/" << data[3] ;
    QLOG_INFO() << "serial # = " <<  (data[0]&0xFF) + (data[1]&0xFF)*256;
    QLOG_INFO() << "build  # = " << data[2] << data[3] << data[4] << data[5] << data[6] << data[7] << data[8] << data[9];
    adcCal0   = data[11]*256 + data[10];
    adcCal40  = data[13]*256 + data[12];
    dacCal0   = data[15]*256 + data[14];
    dacCal40  = data[17]*256 + data[16];
    QLOG_INFO() << "adcCal0 = " << adcCal0;
    QLOG_INFO() << "adcCal40 = " << adcCal40;
    QLOG_INFO() << "dacCal0 = " << dacCal0;
    QLOG_INFO() << "dacCal40 = " << dacCal40; 
    return(0);
}
#endif
