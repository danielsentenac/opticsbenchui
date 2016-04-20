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
#ifdef RAPTORCAMERA
#include "CameraRAPTOR.h"
#include <sys/time.h>

#define RAPTOR_AOI_NUMBER 5
#define BINNING_NUMBER 5

char *raptor_features[]  = {
                        (char*)"Exposure",
                        (char*)"Gain",
                        (char*)"AOI",
			(char*)"Binning",
                        (char*)"Anti Blooming"
			
                        };
char *raptor_props[]  = {
                        (char*)"PCB Temp.",
			(char*)"CCD Temp.",
                        (char*)"Image Size",
			(char*)"Binning",
                        (char*)"EM Gain",
                        (char*)"Exposure",
                        (char*)"PC Rate",
                        (char*)"Frame Rate",
                        (char*)"Anti Blooming"
                        };
int raptor_aoi_settings[][4] = {
                        {0,200,0,200},
                        {0,250,0,250},
                        {0,333,0,333},
                        {0,500,0,500},
                        /*{400,600,400,600},
                        {375,625,375,625},
                        {333,666,333,666},
                        {250,750,250,750},*/
                        {0,1000,0,1000}};

char* raptor_binning_settings[] = {"5x5",
				   "4x4",
				   "3x3",
				   "2x2",
				   "1x1"};
                        

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

CameraRAPTOR::CameraRAPTOR()
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
}

CameraRAPTOR::~CameraRAPTOR()
{
  QLOG_DEBUG() << "Deleting CameraRAPTOR";
  stop();
  cleanup_and_exit();
  delete mutex;
  delete snapshotMutex;
  delete acquireMutex;
  delete acqstart;
  delete acqend;
}
void
CameraRAPTOR::setCamera(void* _camera, int _id)
{
  /* Init camera*/
  vendor = "FalconRaptor";
  model = "Raptor";
  camera_err = connectCamera();

  QLOG_DEBUG() << "CameraRAPTOR::setCamera " << vendor << " model : "
              << model << " - Err : " << camera_err;

  int featureCnt = 0;
 // Exposure feature
 double exp_min, exp_max;
 featureIdList.push_back(featureCnt);
 featureNameList.push_back(raptor_features[featureCnt]);
 exp_min = 1;
 exp_max = 10000;
 featureMinList.push_back(exp_min);
 featureMaxList.push_back(exp_max);
 double exposure = getExposure() ;
 featureValueList.push_back( exposure );
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(exposure);
 featureModeAutoList.push_back(false);

 // Gain feature
 double gain_min, gain_max;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raptor_features[featureCnt]);
 gain_min = 0;
 gain_max = 3500;
 featureMinList.push_back(gain_min);
 featureMaxList.push_back(gain_max);
 double gain = getEMgain() ;
 featureValueList.push_back( gain );
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(gain);
 featureModeAutoList.push_back(false);
 
 // AOI feature
 int aoi_num = RAPTOR_AOI_NUMBER - 1;
 int aoi_min = 0, aoi_max = RAPTOR_AOI_NUMBER - 1;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raptor_features[featureCnt]);
 featureValueList.push_back(aoi_num);
 featureMinList.push_back(aoi_min);
 featureMaxList.push_back(aoi_max);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(aoi_num);
 featureModeAutoList.push_back(false);

 // Binning feature
 int bin_num = BINNING_NUMBER - 1;
 int bin_min = 0, bin_max = BINNING_NUMBER - 1;
 QString initbinStr = getBinningFactor();
 for ( int i = 0; i < BINNING_NUMBER; i++ ) {
  if (initbinStr ==  raptor_binning_settings[i] ) {
    bin_num = i;
    break;
  }
 }
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raptor_features[featureCnt]);
 featureValueList.push_back(bin_num);
 featureMinList.push_back(bin_min);
 featureMaxList.push_back(bin_max);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(bin_num);
 featureModeAutoList.push_back(false);

 // Antiblooming feature
 int bloom_num = 0;
 int bloom_min = 0, bloom_max = 1;
 featureIdList.push_back(++featureCnt);
 featureNameList.push_back(raptor_features[featureCnt]);
 featureValueList.push_back(bloom_num);
 featureMinList.push_back(bloom_min);
 featureMaxList.push_back(bloom_max);
 featureAbsCapableList.push_back(false);
 featureAbsValueList.push_back(bloom_num);
 featureModeAutoList.push_back(false);

 // Properties
 int propCnt = 0;

 // PCB Temperature prop
  QString pcbtempStr = raptor_props[propCnt];
  pcbtempStr.append(" : " + QString::number(getPCBtemperature()));
  pcbtempStr.append(" C");
  propList.push_back(pcbtempStr);
  
  // CCD Temperature prop
  QString ccdtempStr = raptor_props[++propCnt];
  ccdtempStr.append(" : " + QString::number(getCCDtemperature()));
  ccdtempStr.append(" C");
  propList.push_back(ccdtempStr);

  // Image Size prop
  QString imgSizeStr = raptor_props[++propCnt];;
  imgSizeStr.append(" : " + QString::number(pxd_imageXdim()) + "x" + QString::number(pxd_imageYdim()));
  imgSizeStr.append(" pixels");
  propList.push_back(imgSizeStr);

  // Binning prop
  QString binStr = raptor_props[++propCnt];;
  binStr.append(" : " + getBinningFactor());
  propList.push_back(binStr);

  // EM gain prop
  QString emgainStr = raptor_props[++propCnt];
  emgainStr.append(" : " + QString::number(getEMgain()));
  propList.push_back(emgainStr);

  // Exposure prop
  QString exposureStr = raptor_props[++propCnt];
  exposureStr.append(" : " + QString::number(getExposure()));
  exposureStr.append(" ms");
  propList.push_back(exposureStr);

  // PC Rate prop
  QString freqStr = raptor_props[++propCnt];;
  freqStr.append(" : " + QString::number((int)frequency));
  freqStr.append(" Hz");
  propList.push_back(freqStr);

  // Frame Rate prop
  QString frateStr = raptor_props[++propCnt];;
  frateStr.append(" : " + QString::number((int)frate));
  frateStr.append(" Hz");
  propList.push_back(frateStr);

  // Antiblooming prop
  QString bloomStr = raptor_props[++propCnt];;
  bloomStr.append(" : " + getBloomState());
  propList.push_back(bloomStr);

  // Init AOI to max and Binning to 1x1
  setFeature(2,4);
  setFeature(3,4);

}

uchar* 
CameraRAPTOR::getSnapshot() {
  snapshotMutex->lock();
  memcpy(snapshot,buffer,width * height * sizeof(uchar));
  snapShotMin = min;
  snapShotMax = max;
  snapshotMutex->unlock();
  return snapshot;
}

int*
CameraRAPTOR::getSnapshot32() {
  snapshotMutex->lock();
  memcpy(snapshot32,buffer32, width * height * sizeof(int));
  snapShotMin = min;
  snapShotMax = max;
  snapshotMutex->unlock();
  return snapshot32;
}
void 
CameraRAPTOR::stop() {
  suspend = true;
  has_started = false;
  wait();
  exit();  
}

void 
CameraRAPTOR::run() {
  int acq_err = 0;
  int acq_cnt = 0;
  if (camera_err == 0 && suspend == true ) {
    suspend = false;
    eTimeTotal = 0;
    frameTotal = pxd_videoFieldCount(1);
    while (suspend == false) {
      QLOG_DEBUG () << "CameraRAPTOR::run> " << id << " : start new Acquisition";
      double eTime = GetTime();
      acqstart->wakeAll();
      acq_err = acquireImage();
      QLOG_DEBUG () << "CameraRAPTOR::run> " << id << " : done";
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
       QLOG_DEBUG() << "CameraRAPTOR::run> Acquisition "
                   << "freq " << (int) frequency << " Hz";
       eTimeTotal = 0;
       frameTotal = pxd_videoFieldCount(1);
       acq_cnt = 0;
       // Update props
       //getProps();
      }
     has_started = true;
    }
    QLOG_DEBUG() << "CameraRAPTOR thread exiting";
  }
}

void
CameraRAPTOR::setFeature(int feature, double value) {

  QLOG_INFO() << "CameraRAPTOR::setFeature> Update feature " << QString(raptor_features[feature])
               << " value " << QString::number(value);
  int err = 0;
  int bin_width,bin_height;
  while (acquireMutex->tryLock() == false) {
         usleep(100);
  }
  switch ( feature ) {
   case 0:
     setExposure(value);
     break;
   case 1:
     setEMgain((int)value);
     break;
   case 2:
     if (binning_changed == false) {
        aoi_changed = true;
        updateFrameGrabberAOI(raptor_aoi_settings[(int)value][0],
                           raptor_aoi_settings[(int)value][1],
                           raptor_aoi_settings[(int)value][2],
                           raptor_aoi_settings[(int)value][3]);
        QLOG_INFO() << "CameraRAPTOR::setFeature> set AOI left " << raptor_aoi_settings[(int)value][0]
                                        <<  " width " << raptor_aoi_settings[(int)value][1]
                                        <<  " top " << raptor_aoi_settings[(int)value][2]
                                        <<  " height " << raptor_aoi_settings[(int)value][3];

        setAOI(raptor_aoi_settings[(int)value][0],raptor_aoi_settings[(int)value][1],
            raptor_aoi_settings[(int)value][2], raptor_aoi_settings[(int)value][3]);
        int bin_num = 0;
        /*QString initbinStr = getBinningFactor();
        for ( int i = 0; i < BINNING_NUMBER; i++ ) {
         if (initbinStr ==  raptor_binning_settings[i] ) {
          bin_num = i;
         break;
         }
        }
        if ((int)value == 1 && bin_num < 1) 
         setBinningFactor(1);
        if ((int)value == 2 && bin_num < 2)    
         setBinningFactor(2);
        if ((int)value == 3 && bin_num < 3)
         setBinningFactor(3);
        if ((int)value == 4 && bin_num < 4)
         setBinningFactor(4);*/
        setBinningFactor(4);
     }
     else {
       aoi_changed = false;
       binning_changed = false;
     }
     break;
   case 3:
     if (aoi_changed == false) {
       binning_changed = true;
       updateFrameGrabberAOI(raptor_aoi_settings[4][0],
                           raptor_aoi_settings[4][1],
                           raptor_aoi_settings[4][2],
                           raptor_aoi_settings[4][3]);
       setAOI(raptor_aoi_settings[4][0],raptor_aoi_settings[4][1],
            raptor_aoi_settings[4][2], raptor_aoi_settings[4][3]);
       setBinningFactor(4);
       updateFrameGrabberAOI(raptor_aoi_settings[(int)value][0],
                           raptor_aoi_settings[(int)value][1],
                           raptor_aoi_settings[(int)value][2],
                           raptor_aoi_settings[(int)value][3]);
       // Apply now Binning change
       setBinningFactor((int)value);
     }
     else { 
        binning_changed = false;
        aoi_changed = false;
     }
     break;
   case 4:
     setBloomState((int)value);
     break;
   default:
     break;
  }
  // Update memory allocation
       width = pxd_imageXdim();
       height =  pxd_imageYdim();
       QLOG_INFO() << "CameraRAPTOR::setFeature> Updated width " << width;
       QLOG_INFO() << "CameraRAPTOR::setFeature> Updated height " << height;
       if (buffer) { free(buffer); buffer = NULL;}
       if (snapshot) { free(snapshot); snapshot = NULL;}
       if (buffer32) { free(buffer32); buffer32 = NULL;}
       if (snapshot32) { free(snapshot32); snapshot32 = NULL;}
       delete image;
       buffer = (uchar*)malloc( sizeof(uchar) * width * height);
       snapshot = (uchar*)malloc( sizeof(uchar) * width * height);
       buffer32 = (int*)malloc( sizeof(int) * width * height);
       snapshot32 = (int*)malloc( sizeof(int) * width * height);
       image16 = (ushort*)malloc( sizeof(ushort) * width * height);
       image = new QImage(buffer,width,height,width,QImage::Format_Indexed8);
       image->setColorTable(*table);

  acquireMutex->unlock();
  getFeatures();
  getProps();
}
void  
CameraRAPTOR::setMode(int feature, bool value) {
}
void
CameraRAPTOR::getFeatures() {
  /*-----------------------------------------------------------------------
   *  report camera's features
   *-----------------------------------------------------------------------*/
 
 int featureCnt = 0;

 // Exposure feature
 double exposure = getExposure() ;
 featureValueList.replace(featureCnt, exposure );

 // Gain feature
 double gain = getEMgain() ;
 featureValueList.replace(++featureCnt, gain );

 // AOI feature
 int width = pxd_imageXdim();
 for (int i = 0 ; i < RAPTOR_AOI_NUMBER; i++) {
   if (width == raptor_aoi_settings[i][1])
     featureValueList.replace(++featureCnt, i );
 }

 // Binning feature
 int bin_num = 0;
 QString initbinStr = getBinningFactor();
 for ( int i = 0; i < BINNING_NUMBER; i++ ) {
  if (initbinStr ==  raptor_binning_settings[i] ) {
    bin_num = i;
    break;
  }
 }
 featureValueList.replace(++featureCnt, bin_num );

 // Bloom feature
 featureCnt++;
 int bloomState = 0;
 if ( getBloomState() == "ON" )
   bloomState = 1;
 featureValueList.replace(featureCnt, bloomState);

 emit updateFeatures();
}
void 
CameraRAPTOR::getProps()  {

// Properties
  int propCnt = 0;

  // PCB Temperature prop
  QString pcbtempStr = raptor_props[propCnt];
  pcbtempStr.append(" : " + QString::number(getPCBtemperature()));
  pcbtempStr.append(" C");
  propList.replace(propCnt,pcbtempStr);
 
  // CCD Temperature prop
  QString ccdtempStr = raptor_props[++propCnt];
  ccdtempStr.append(" : " + QString::number(getCCDtemperature()));
  ccdtempStr.append(" C");
  propList.replace(propCnt,ccdtempStr);
  
  // Image Size prop
  QString imgSizeStr = raptor_props[++propCnt];;
  imgSizeStr.append(" : " + QString::number(pxd_imageXdim()) + "x" + QString::number(pxd_imageYdim()));
  imgSizeStr.append(" pixels");
  propList.replace(propCnt,imgSizeStr);

   // Binning prop
  QString binStr = raptor_props[++propCnt];;
  binStr.append(" : " + getBinningFactor());
  propList.replace(propCnt,binStr);

  // EM gain prop
  QString emgainStr = raptor_props[++propCnt];
  emgainStr.append(" : " + QString::number(getEMgain()));
  QLOG_DEBUG() << "emgainStr.append = " << emgainStr;
  propList.replace(propCnt,emgainStr);

  // Exposure prop
  QString exposureStr = raptor_props[++propCnt];
  exposureStr.append(" : " + QString::number(getExposure()));
  exposureStr.append(" ms");
  propList.replace(propCnt,exposureStr);

  // PC Rate
  QString freqStr = raptor_props[++propCnt];;
  freqStr.append(" : " + QString::number((int)frequency));
  freqStr.append(" Hz");
  propList.replace(propCnt, freqStr);

  // Frame Rate prop
  QString frateStr = raptor_props[++propCnt];;
  frateStr.append(" : " + QString::number((int)frate));
  frateStr.append(" Hz");
  propList.replace(propCnt,frateStr);

  // Bloom state prop
  QString bloomStr = raptor_props[++propCnt];;
  bloomStr.append(" : " + getBloomState());
  propList.replace(propCnt,bloomStr);

  QLOG_DEBUG() << "CameraRAPTOR::getProps> Properties updated";
  emit updateProps();

}
int 
CameraRAPTOR::findCamera() {

  int err = -1;
  QLOG_DEBUG() << "Open Raptor";
  err = pxd_PIXCIopen("", "default", "");
  QLOG_DEBUG() << "Open Raptor" << err;
  if (err < 0) {
    num = 0;
    pxd_PIXCIclose();
    return 0;
  }
  int camera = 1; // Camera internal ID
  cameralist.push_back((int*)&camera);
  vendorlist.push_back("Falcon");
  modelist.push_back("Raptor");
  num = 1; // Camera number

  return 0;
}
int 
CameraRAPTOR::connectCamera() {

  int err;

  QLOG_DEBUG() << "Connect serial to Raptor";
  err = pxd_serialConfigure(0x1, 0, 115200, 8, 0, 1, 0, 0, 0);
  QLOG_DEBUG() << "Connect serial to Raptor" << err;

  char readadrs_v[]   = { 0x56, 0x50 };
  uchar resp_v[] = { 0x0, 0x0 };
  readFeature (readadrs_v, 0, readadrs_v, 2, resp_v, 2);
  QLOG_DEBUG() << "Found Raptor camera Micro Version V" << resp_v[0] << "." << resp_v[1];

  /*-----------------------------------------------------------------------
   *  setup capture
   *-----------------------------------------------------------------------*/

  QLOG_DEBUG() << "CameraRAPTOR::connectCamera()> Xdim=" << pxd_imageXdim();
  QLOG_DEBUG() << "CameraRAPTOR::connectCamera()> Ydim=" << pxd_imageYdim();
  QLOG_DEBUG() << "CameraRAPTOR::connectCamera()> Cdim=" << pxd_imageCdim();
  QLOG_DEBUG() << "CameraRAPTOR::connectCamera()> Bdim=" << pxd_imageBdim();

  // Init AOI and buffers
  width = pxd_imageXdim();
  height =  pxd_imageYdim();
  QLOG_DEBUG() << "CameraRAPTOR::connectCamera> Updated width " << width;
  QLOG_DEBUG() << "CameraRAPTOR::connectCamera> Updated height " << height;
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
  return 0;

}
void  
CameraRAPTOR::cleanup_and_exit()
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
CameraRAPTOR::acquireImage() {

    // Capture a new frame
    while (acquireMutex->tryLock() == false) {
       usleep(100);
    }
    int err = pxd_doSnap(1,1,100);
    err = pxd_readushort(1, 1, 0, 0, width, height, image16, (size_t)(width * height), "Grey");
    if (err < 0)
      QLOG_WARN() << "pxd_readushort:" <<  pxd_mesgErrorCode(err) ;
    else if ( err != pxd_imageXdim() * pxd_imageYdim())
      QLOG_DEBUG() << "pxd_readushort  missing pixels" << err << "!= " << pxd_imageXdim() * pxd_imageYdim();

    // calculate min,max
    max = 0;
    min = 65535;
    // Treat Mono16 case
    for (int i = 0; i < height * width; i++) {
     // QLOG_DEBUG() << "CameraRAPTOR::acquireImage()> image16[" << i << "]=" << image16[i];
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

void CameraRAPTOR::setImageSize(const int &_imageWidth, const int &_imageHeight){
  imageWidth = _imageWidth;
  imageHeight = _imageHeight;
  QLOG_DEBUG ( ) <<  "emit new image with size " << imageWidth  << "x" << imageHeight;
}
int CameraRAPTOR::readFeature(char* sendreg, int size_sreg, char* readreg, int size_rreg, uchar* data, int size_data) {
  int error = 0;
  uchar tmp[] = { 0x0 };

  usleep(5000);
  while (pxd_serialRead(0x1, 0, (char*)tmp, 1) == 1) {
    QLOG_DEBUG() << "CameraRAPTOR::readFeature> tmp=" << tmp[0];
  }
  usleep(5000);
  error = pxd_serialWrite(0x1, 0, sendreg, size_sreg);
  if ( error < 0 ) {
   QLOG_WARN() << "CameraRAPTOR::readFeature> error=" << error;
   return error;
  }
  usleep(5000);
  while (pxd_serialRead(0x1, 0, (char*)tmp, 1) == 1) {
    QLOG_DEBUG() << "CameraRAPTOR::readFeature> tmp=" << tmp[0];
  }
  usleep(5000);
  error = pxd_serialWrite(0x1, 0, readreg, size_rreg);
  if ( error < 0 ) {
   QLOG_WARN() << "CameraRAPTOR::readFeature> error=" << error;
   return error;
  }
  usleep(5000);
  error = pxd_serialRead(0x1, 0, (char*)data, size_data);
  if ( error < 0 ) {
   QLOG_WARN() << "CameraRAPTOR::readFeature> pxd_serialRead error=" << error;
   return error;
  }
  pxd_serialRead(0x1, 0, (char*)tmp, 1);
  for (int i= 0; i < size_data; i++)
    QLOG_DEBUG() << "CameraRAPTOR::readFeature> reads: " << " data[" << i << "]=" << data[i];

  return error;
}
int CameraRAPTOR::writeFeature(char* sendreg, int size_sreg) {
  int error;
  uchar tmp[] = { 0x0 };

  usleep(5000);
  while (pxd_serialRead(0x1, 0, (char*)tmp, 1) == 1) {
    QLOG_DEBUG() << "CameraRAPTOR::readFeature> tmp=" << tmp[0];
  }
  usleep(5000);
  error = pxd_serialWrite(0x1, 0, sendreg, size_sreg);
  if ( error < 0 ) {
   QLOG_WARN() << "CameraRAPTOR::writeFeature> error=" << error;
   return error;
  }
  usleep(5000);
  while (pxd_serialRead(0x1, 0, (char*)tmp, 1) == 1) {
    QLOG_DEBUG() << "CameraRAPTOR::readFeature> tmp=" << tmp[0];
  }
 return error;
}
double CameraRAPTOR::getPCBtemperature() {
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
double CameraRAPTOR::getEMgain() {
  QLOG_DEBUG() << "Get Raptor EM gain";
  char setadrs1[]   = { 0x53, 0xE0, 0x01, 0x09, 0x50 };
  char setadrs2[]   = { 0x53, 0xE0, 0x01, 0x0A, 0x50 };
  char readadrs[]   = { 0x53, 0xE1, 0x01, 0x50 };
  uchar resp1[] = { 0x0 };
  uchar resp2[] = { 0x0 };
  readFeature (setadrs1, 5, readadrs, 4, resp1, 1);
  readFeature (setadrs2, 5, readadrs, 4, resp2, 1);
  int  g = ((resp1[0]&0x0F)<<8) | (resp2[0]&0xFF);
  QLOG_DEBUG() << "gain =" <<  g;
  return g;
}
void CameraRAPTOR::setEMgain(int g) {
  QLOG_DEBUG() << "Set Raptor EM gain";
  char setadrs1[]   = { 0x53, 0xE0, 0x02, 0x09, 0, 0x50 };
  char setadrs2[]   = { 0x53, 0xE0, 0x02, 0x0A, 0, 0x50 };
  setadrs1[4] = (g>>8);
  setadrs2[4] = (g>>0);
  writeFeature (setadrs1, 6);
  writeFeature (setadrs2, 6);
  int  gsend = ((setadrs1[0]&0x0F)<<8) | (setadrs2[0]&0xFF);
  QLOG_DEBUG() << "CameraRAPTOR::setEMgain> Sending gain =" <<  g;
}
void CameraRAPTOR::setAOI(int left, int width, int top, int height) {
  QLOG_DEBUG() << "CameraRAPTOR::setAOI> Set Raptor AOI left " << left 
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
double CameraRAPTOR::getExposure() {
  QLOG_DEBUG() << "Get Raptor exposure";
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
  QLOG_DEBUG() << "exposure =" <<  e / 36000.;
  return e / 36000.;
}
void CameraRAPTOR::setExposure(double e) {
  QLOG_DEBUG() << "CameraRAPTOR::setExposure> Set Raptor exposure " << e;
  char setadrs1[]   = { 0x53, 0xE0, 0x02, 0xEE, 0, 0x50 };
  char setadrs2[]   = { 0x53, 0xE0, 0x02, 0xEF, 0, 0x50 };
  char setadrs3[]   = { 0x53, 0xE0, 0x02, 0xF0, 0, 0x50 };
  char setadrs4[]   = { 0x53, 0xE0, 0x02, 0xF1, 0, 0x50 };
  int value = (int)(e * 36000);
  setadrs1[4] = (value>>24);
  setadrs2[4] = (value>>16);
  setadrs3[4] = (value>>8);
  setadrs4[4] = (value>>0);
  writeFeature (setadrs1, 6);
  writeFeature (setadrs2, 6);
  writeFeature (setadrs3, 6);
  writeFeature (setadrs4, 6);
  
  double esend = ((setadrs1[4]&0x0F)<<24) | ((setadrs2[4]&0xFF)<<16) | ((setadrs3[4]&0xFF)<<8) | (setadrs4[4]&0xFF) ;
  QLOG_DEBUG() << "CameraRAPTOR::setExposure> Sending exposure =" <<  esend / 36000.;
}

double CameraRAPTOR::getCCDtemperature() {
  QLOG_DEBUG() << "Get Raptor CCD temperature";
  char setadrs[]   = { 0x53, 0xE0, 0x01, 0x6F, 0x50 };
  char readadrs[]  = { 0x53, 0xE1, 0x01, 0x50 };
  uchar resp[] = { 0x0 };
  readFeature (setadrs, 5, readadrs, 4, resp, 1);
  double t = -20.0 - 107* -0.45 + (int)(resp[0]&0xFF) * -0.45;
  QLOG_DEBUG() << "ccdtemp =" <<  t;
  return t;
}
QString CameraRAPTOR::getBinningFactor() {
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
void CameraRAPTOR::setBinningFactor(int value) {
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
QString CameraRAPTOR::getBloomState() {
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
void CameraRAPTOR::setBloomState(int value) {
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
void CameraRAPTOR::updateFrameGrabberAOI(int v0, int v1, int v2, int v3) {
     // Update frame grabber AOI
     if (!(xc = pxd_xclibEscape(0, 0, 0)))
      QLOG_DEBUG() << "CameraRAPTOR::updateFrameGrabberAOI> Cannot update frame grabber state";
     
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
#endif
