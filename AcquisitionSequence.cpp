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

#include "AcquisitionSequence.h"

AcquisitionSequence::AcquisitionSequence( )
{
  reset = true;
  instrumentName = "";
  instrumentType = "";
  settings = "";
  motorAction = "";
  motorValue = 0;
  dacValue = 0;
  dacOutput = 0;
  comediValue = 0;
  comediData = 0;
  comediOutput = 0;
  scanplan = "";
  loop = -1;
  loopends = -1;
  loopNumber = -1;
  remainingLoops = -1;
  datagroup = "";
  dataname = "";
  group = "";
  position = 0;
  inc_group = 0;
  image = NULL;
  image32 = NULL;
  data_2D_FLOAT = NULL;
  data_2D_FLOAT_DIM_X = 0;
  data_2D_FLOAT_DIM_Y = 0;
  refgrp = 0;
  grp = 0;
  tmpgrp = 0;
  grpname = "";
  avg = "";
  slmimage = NULL;
  fullpath="";
  imagepath = "";
  imagetype = "";
  imgnum = 0;
  keepzero = "";
  startnum = 0;
  stepnum = 0;
  slmwidth = 0;
  slmheight = 0;
  screen_x = 0;
  screen_y = 0;
  treatment = "";
  avgOp = "";
  avgSuccess = -1;
  avgLeft = -1;
  avgRight = -1;
  instrumentRef = "";
  sleep = 0;
  fileParser  = new FileParser();
}

AcquisitionSequence::~AcquisitionSequence()
{
  QLOG_DEBUG ( ) <<"Deleting AcquisitionSequence";
 
    if (image) { free(image); image = NULL;}
    if (image32) { free(image32); image32 = NULL;}
    if (data_2D_FLOAT) { free(data_2D_FLOAT); data_2D_FLOAT = NULL;}
    if (slmimage)  { free(slmimage);slmimage  = NULL;}
    delete fileParser;
}
void 
AcquisitionSequence::setImage(uchar* buffer, int width, int height) {
  if (image) { free(image); image = NULL;}
  if (image32) { free(image32); image32 = NULL;}
  if (buffer != NULL) {
    imageWidth = width;
    imageHeight = height;
    image = (uchar*) malloc (sizeof(uchar) * imageWidth * imageHeight);
    memcpy(image,buffer,sizeof(uchar) * imageWidth * imageHeight);
  }
  else
   QLOG_WARN () << "AcquisitionSequence::setImage> Input buffer is NULL..";
}
void
AcquisitionSequence::setImage32(int* buffer32, int width, int height) {
  if (image32) { free(image32); image32 = NULL;}
  if (image) { free(image); image = NULL;}
  if (buffer32 != NULL) {
    imageWidth = width;
    imageHeight = height;
    image32 = (int*) malloc (sizeof(int) * imageWidth * imageHeight);
    memcpy(image32,buffer32,sizeof(int) * imageWidth * imageHeight);
  }
  else
   QLOG_WARN () << "AcquisitionSequence::setImage32> Input buffer32 is NULL..";
}

void AcquisitionSequence::setImageMin(int _imageMin)
{
  imageMin = _imageMin;

}
void AcquisitionSequence::setImageMax(int _imageMax)
{
  imageMax = _imageMax;
}
uchar*
AcquisitionSequence::getImage() {
 return image;
}
uchar*
AcquisitionSequence::getImageFromFile() {
  slmimage = new QImage(fullpath);
  imageWidth = slmimage->width();
  imageHeight = slmimage->height();
  return slmimage->bits();
}
int*
AcquisitionSequence::getImage32() {
 return image32;
}

bool 
AcquisitionSequence::getFileData() {
  fileParser->getFileData();
}
void 
AcquisitionSequence::prepare() {
  datagroup = "";
  QLOG_DEBUG ( ) <<"Sequence " << settings << ":" << scanplan;
  // Treat settingsList
  QStringList settingsList;
  QStringList subsettingsList;
  settingsList = settings.split(" ",QString::SkipEmptyParts);
  foreach (const QString &itemlist, settingsList)
    QLOG_DEBUG () << "AcquisitionSequence::prepare> settingsList : " << itemlist;
  for (int i = 0 ; i < settingsList.size(); i++ ) {
    subsettingsList = settingsList.at(i).split("=",QString::SkipEmptyParts);
    foreach (const QString &subitemlist, subsettingsList)
      QLOG_DEBUG () << "AcquisitionSequence::prepare> subsettingsList : " << subitemlist;
    for (int j = 0 ; j < subsettingsList.size(); j++ ) {
      QLOG_DEBUG () << "subsettingsList.at(j) = " << subsettingsList.at(j);
      // File section
      if (subsettingsList.at(j) == "DIR" && subsettingsList.size() > j + 1) {
        fileParser->setDir(subsettingsList.at(j+1));
      }
      else if (subsettingsList.at(j) == "PREFIX" && subsettingsList.size() > j + 1) {
        fileParser->setPrefix(subsettingsList.at(j+1));
      }
      else if (subsettingsList.at(j) == "SUFFIX" && subsettingsList.size() > j + 1) {
        fileParser->setSuffix(subsettingsList.at(j+1));
      }
      else if (subsettingsList.at(j) == "DEL" && subsettingsList.size() > j + 1) {
        fileParser->setDel(subsettingsList.at(j+1));
      }
      else if (subsettingsList.at(j) == "SEP" && subsettingsList.size() > j + 1) {
        fileParser->setSep(subsettingsList.at(j+1));
      }
      else if (subsettingsList.at(j) == "FORMAT" && subsettingsList.size() > j + 1) {
        fileParser->setFormat(subsettingsList.at(j+1));
      }
      else if (subsettingsList.at(j) == "TYPE" && subsettingsList.size() > j + 1) {
         fileParser->setType(subsettingsList.at(j+1));
      }
      else if (subsettingsList.at(j) == "DELAY" && subsettingsList.size() > j + 1) {
        fileParser->setDelay(subsettingsList.at(j+1));
      }
      else if (subsettingsList.at(j) == "ORDER" && subsettingsList.size() > j + 1) {
        fileParser->setOrder(subsettingsList.at(j+1));
      }
      // Motor section
      else if (subsettingsList.at(j).leftRef(4) == "MOVE" && subsettingsList.size() > j + 1) {
	motorAction = subsettingsList.at(j);
	motorValue = subsettingsList.at(j+1).toFloat();
      }
     // Dac and Comedi section
      else if (subsettingsList.at(j) == "OUTPUT" && subsettingsList.size() > j + 1) {
	dacOutput = subsettingsList.at(j+1).toInt();
        comediOutput = subsettingsList.at(j+1).toInt();
      }
      else if (subsettingsList.at(j) == "VALUE" && subsettingsList.size() > j + 1) {
	dacValue = subsettingsList.at(j+1).toDouble();
      }
      // ComediCounter section
      else if (subsettingsList.at(j) == "ITIME" && subsettingsList.size() > j + 1) {
        comediValue = subsettingsList.at(j+1).toInt();
      }
     // Acquisition section
      else if (subsettingsList.at(j) == "SLEEP" && subsettingsList.size() > j + 1) {
	sleep = (int) (subsettingsList.at(j+1).toFloat()*1e6);
      }
     // SLM section
      else if (subsettingsList.at(j) == "IMAGEPATH" && subsettingsList.size() > j + 1) {
        treatment = "IMAGEPATH";
        imagepath = subsettingsList.at(j+1);
      }
      else if (subsettingsList.at(j) == "IMAGETYPE" && subsettingsList.size() > j + 1) {
        treatment = "IMAGETYPE";
        imagetype = subsettingsList.at(j+1);
      }
      else if (subsettingsList.at(j) == "START" && subsettingsList.size() > j + 1) {
        treatment = "START";
        startnum = subsettingsList.at(j+1).toInt();
        for ( int k = 0 ; k < subsettingsList.at(j+1).size(); k++) {
          if (subsettingsList.at(j+1).at(k) == '0') { 
            keepzero = keepzero + "0";
          }
          else
           break;
        } 
      }
      else if (subsettingsList.at(j) == "STEP" && subsettingsList.size() > j + 1) {
        treatment = "STEP";
        stepnum = subsettingsList.at(j+1).toInt();
      }
      else if (subsettingsList.at(j) == "SLMWIDTH" && subsettingsList.size() > j + 1) {
        treatment = "SLMWIDTH";
        slmwidth = subsettingsList.at(j+1).toInt();
      }
      else if (subsettingsList.at(j) == "SLMHEIGHT" && subsettingsList.size() > j + 1) {
        treatment = "SLMHEIGHT";
        slmheight = subsettingsList.at(j+1).toInt();
      }
      else if (subsettingsList.at(j) == "SCREEN_X" && subsettingsList.size() > j + 1) {
        treatment = "SCREEN_X";
        screen_x = subsettingsList.at(j+1).toInt();
      }
      else if (subsettingsList.at(j) == "SCREEN_Y" && subsettingsList.size() > j + 1) {
        treatment = "SCREEN_Y";
        screen_y = subsettingsList.at(j+1).toInt();
      }
     // Treatment section
      else if (subsettingsList.at(j) == "AVG" && subsettingsList.size() > j + 1) {
	treatment = "AVG";
	avg = subsettingsList.at(j+1);
	// Treat AVG Settings
	QString subavg;
	QString subsubavg;
	subavg = avg.mid(avg.indexOf("[") + 1, avg.indexOf("]") - avg.indexOf("[") - 1);

	avgSuccess = subavg.indexOf("+");
	if (avgSuccess >= 0 ) avgOp = "+";
	else {
	  avgSuccess = subavg.indexOf("-");
	  if (avgSuccess >= 0 ) avgOp = "-";
	  else {
	    avgSuccess = subavg.indexOf("*");
	    if (avgSuccess >= 0 ) avgOp = "*";
	    else {
	      avgSuccess = subavg.indexOf("/");
	      if (avgSuccess >= 0 ) avgOp = "/";
	    }
	  }
	}
	if (avgSuccess == -1)  {
		avgLeft = subavg.toInt();
		QLOG_DEBUG ( ) << "Average " << avg << " " << subavg << " Op " << avgOp << " Left " 
			 <<  avgLeft << " Right " << avgRight;
	}
	else {
	  avgLeft = subavg.left(subavg.indexOf(avgOp)).toInt();
	  avgRight = subavg.right(subavg.size() - subavg.indexOf(avgOp) - 1).toInt();
	  
	  QLOG_DEBUG ( ) << "Average " << avg << " " << subavg << " Op " << avgOp << " Left " 
			 <<  avgLeft << " Right " << avgRight;
	}
      }
      else if (subsettingsList.at(j) == "PHASE" && subsettingsList.size() > j + 1) {
	treatment = "PHASE";
	avg = subsettingsList.at(j+1);
	// Treat PHASE Settings
	QString subavg;
	QString subsubavg;
	subavg = avg.mid(avg.indexOf("[") + 1, avg.indexOf("]") - avg.indexOf("[") - 1);
	
	avgSuccess = subavg.indexOf(",");
	if (avgSuccess >= 0 ) avgOp = ",";
	if (avgSuccess != -1)  {
	  avgLeft = subavg.left(subavg.indexOf(avgOp)).toInt();
	  avgRight = subavg.right(subavg.size() - subavg.indexOf(avgOp) - 1).toInt();
	  QLOG_DEBUG ( ) << "Phase " << avg << " " << subavg << " Op " << avgOp << " Left " 
			 <<  avgLeft << " Right " << avgRight;
	}
      }
      else if (subsettingsList.at(j) == "AMPLITUDE" && subsettingsList.size() > j + 1) {
	treatment = "AMPLITUDE";
	avg = subsettingsList.at(j+1);
	// Treat AMPLITUDE Settings
	QString subavg;
	QString subsubavg;
	subavg = avg.mid(avg.indexOf("[") + 1, avg.indexOf("]") - avg.indexOf("[") - 1);
	
	avgSuccess = subavg.indexOf(",");
	if (avgSuccess >= 0 ) avgOp = ",";
	if (avgSuccess != -1)  {
	  avgLeft = subavg.left(subavg.indexOf(avgOp)).toInt();
	  avgRight = subavg.right(subavg.size() - subavg.indexOf(avgOp) - 1).toInt();
	  QLOG_DEBUG ( ) << "Amplitude " << avg << " " << subavg << " Op " << avgOp << " Left " 
			 <<  avgLeft << " Right " << avgRight;
	}
      }
      else if (subsettingsList.at(j) == "IMAGE" && subsettingsList.size() > j + 1) {
	treatment = "IMAGE";
	avg = subsettingsList.at(j+1);
	// Treat IMAGE Settings
	QString subavg;
	QString subsubavg;
	subavg = avg.mid(avg.indexOf("[") + 1, avg.indexOf("]") - avg.indexOf("[") - 1);
	avgLeft = subavg.toInt();
	QLOG_DEBUG ( ) << "Image " << avg << " " << subavg << " Op " << avgOp << " Left " 
		       <<  avgLeft << " Right " << avgRight;
      }
    }
  }

  // Preparation configuration
  // For SLM
  imgnum = startnum - stepnum;

  // Treat scanplanList
  QStringList scanplanList;
  QStringList subscanplanList;
  scanplanList = scanplan.split(" ",QString::SkipEmptyParts);
  for (int i = 0 ; i < scanplanList.size(); i++ ) {
    subscanplanList = scanplanList.at(i).split("=",QString::SkipEmptyParts);
    for (int j = 0 ; j < subscanplanList.size(); j++ ) {
      if (subscanplanList.at(j) == "DATAGROUP" && subscanplanList.size() > j + 1)
	datagroup = subscanplanList.at(j+1);
      else if (subscanplanList.at(j) == "DATANAME" && subscanplanList.size() > j + 1)
	dataname = subscanplanList.at(j+1);
      else if (subscanplanList.at(j) == "LOOP" && subscanplanList.size() > j + 1) {
	loop = IS_LOOP;
	loopNumber = subscanplanList.at(j+1).toInt();
	loopends = LOOP_START;
	if (loopNumber == 0)
	  loopends = LOOP_END;
	remainingLoops = loopNumber;		  
      }
    }
  }  
}
bool
AcquisitionSequence::setAvg(AcquisitionSequence *sequenceLeft, AcquisitionSequence *sequenceRight) {
  
  uchar *imageRight = NULL;
  uchar *imageLeft = NULL;
  int  *imageRight32 = NULL;
  int  *imageLeft32 = NULL;
  bool success = false;
  if (sequenceLeft != (AcquisitionSequence *)NULL && sequenceRight != (AcquisitionSequence *)NULL ) {
    if (sequenceLeft->status == true && sequenceRight->status == true) {
      if (sequenceLeft->instrumentType == "CAMERA" && 
	  sequenceRight->instrumentType == "CAMERA") {
	instrumentRef = "CAMERA";
        if ( sequenceRight->getImage() )
          imageRight = sequenceRight->getImage();
        else if ( sequenceRight->getImage32() )
          imageRight32 = sequenceRight->getImage32();
        if ( sequenceLeft->getImage() )
          imageLeft = sequenceLeft->getImage();
        else if ( sequenceLeft->getImage32() )
          imageLeft32 = sequenceLeft->getImage32();
        if (reset == true) {
	  if (data_2D_FLOAT) {
	    free(data_2D_FLOAT);
	    data_2D_FLOAT = NULL;
	  }
	  if (sequenceLeft->imageWidth <= sequenceRight->imageWidth)
	    data_2D_FLOAT_DIM_X = sequenceLeft->imageWidth;
	  else
	    data_2D_FLOAT_DIM_X = sequenceRight->imageWidth;
	  if (sequenceLeft->imageHeight <= sequenceRight->imageHeight)
	    data_2D_FLOAT_DIM_Y = sequenceLeft->imageHeight;
	  else
	    data_2D_FLOAT_DIM_Y = sequenceRight->imageHeight;
	  data_2D_FLOAT = (float*) malloc (sizeof(float) * data_2D_FLOAT_DIM_X * data_2D_FLOAT_DIM_Y);
	  memset(data_2D_FLOAT,0,sizeof(data_2D_FLOAT));
	  reset = false;
	}
	for (int i = 0 ; i < data_2D_FLOAT_DIM_X * data_2D_FLOAT_DIM_Y; i++) {
          if ( imageLeft &&  imageRight ) {
	    if ( avgOp == "+" ) 
	      data_2D_FLOAT[i] = data_2D_FLOAT[i] + (imageLeft[i] + imageRight[i]);
	    else if ( avgOp == "-" ) 
	      data_2D_FLOAT[i] = data_2D_FLOAT[i] + (imageLeft[i] - imageRight[i]);
	    else if ( avgOp == "*" )
	      data_2D_FLOAT[i] = data_2D_FLOAT[i] + (imageLeft[i] * imageRight[i]);
	    else if ( avgOp == "/") {
	      if (imageRight[i] != 0)
	        data_2D_FLOAT[i] = data_2D_FLOAT[i] + (imageLeft[i] / imageRight[i]);
            }
          }
          else if ( imageLeft32 && imageRight32 ) {
            if ( avgOp == "+" )
              data_2D_FLOAT[i] = data_2D_FLOAT[i] + (imageLeft32[i] + imageRight32[i]);
            else if ( avgOp == "-" )
              data_2D_FLOAT[i] = data_2D_FLOAT[i] + (imageLeft32[i] - imageRight32[i]);
            else if ( avgOp == "*" )
              data_2D_FLOAT[i] = data_2D_FLOAT[i] + (imageLeft32[i] * imageRight32[i]);
            else if ( avgOp == "/") {
              if (imageRight[i] != 0)
                data_2D_FLOAT[i] = data_2D_FLOAT[i] + (imageLeft32[i] / imageRight32[i]);
            }
	  }
	}
	success = true;
      }
      else if (sequenceLeft->instrumentType.contains("DAC") && 
	       sequenceRight->instrumentType.contains("DAC")) {
	instrumentRef = "DAC";
	if (reset == true) {
	  dacValue = 0;
	}
	if ( avgOp == "+" ) 
	  dacValue = dacValue + (sequenceLeft->dacValue + sequenceRight->dacValue);
	else if ( avgOp == "-" ) 
	  dacValue = dacValue + (sequenceLeft->dacValue - sequenceRight->dacValue);
	else if ( avgOp == "*" ) 
	  dacValue = dacValue + (sequenceLeft->dacValue * sequenceRight->dacValue);
	else if ( avgOp == "/" )
	  dacValue = dacValue + (sequenceLeft->dacValue / sequenceRight->dacValue);
	success = true;
      }
      else if (sequenceLeft->instrumentType == "COMEDICOUNTER" &&
               sequenceRight->instrumentType == "COMEDICOUNTER") {
        instrumentRef = "COUNTER";
        if (reset == true) {
          comediData = 0;
        }
        if ( avgOp == "+" )
          comediData = comediData + (sequenceLeft->comediData + sequenceRight->comediData);
        else if ( avgOp == "-" )
          comediData = comediData + (sequenceLeft->comediData - sequenceRight->comediData);
        else if ( avgOp == "*" )
          comediData = comediData + (sequenceLeft->comediData * sequenceRight->comediData);
        else if ( avgOp == "/" )
          comediData = comediData + (sequenceLeft->comediData / sequenceRight->comediData);
        success = true;
      }
    }
  }
  else if (sequenceLeft != (AcquisitionSequence *)NULL && sequenceRight == (AcquisitionSequence *)NULL ) {
    QLOG_DEBUG ( ) << "sequenceLeft not NULL && sequenceRight is NULL";
    if (sequenceLeft->status == true) {
      QLOG_DEBUG ( ) << "sequenceLeft is true";
      if ( sequenceLeft->instrumentType == "CAMERA" ) {
	instrumentRef = "CAMERA";
        if ( sequenceLeft->getImage() )
          imageLeft = sequenceLeft->getImage();
        else if ( sequenceLeft->getImage32() )
          imageLeft32 = sequenceLeft->getImage32();
	if (reset == true) {
	  if (data_2D_FLOAT) {
	    free(data_2D_FLOAT);
	    data_2D_FLOAT = NULL;
	  }
	  data_2D_FLOAT_DIM_X  = sequenceLeft->imageWidth;
	  data_2D_FLOAT_DIM_Y = sequenceLeft->imageHeight;
	  data_2D_FLOAT = (float*) malloc (sizeof(float) * data_2D_FLOAT_DIM_X * data_2D_FLOAT_DIM_Y);
	  memset(data_2D_FLOAT,0,sizeof(data_2D_FLOAT));
	  reset = false;
	}
	for (int i = 0 ; i < data_2D_FLOAT_DIM_X * data_2D_FLOAT_DIM_Y; i++) {
          if ( imageLeft ) 
	    data_2D_FLOAT[i] = data_2D_FLOAT[i] + imageLeft[i];
          else if ( imageLeft32 )
            data_2D_FLOAT[i] = data_2D_FLOAT[i] + imageLeft32[i];
        }
	success = true;
      }
      else if ( sequenceLeft->instrumentType == "DAC" ) {
	instrumentRef = "DAC";
	dacValue = dacValue + sequenceLeft->dacValue ;
	success = true;
      }
      else if ( sequenceLeft->instrumentType == "COMEDICOUNTER" ) {
        instrumentRef = "COUNTER";
        comediData = comediData + sequenceLeft->comediData ;
        success = true;
      }
      else if ( sequenceLeft->instrumentType == "COMEDIDAC" ) {
        instrumentRef = "DAC";
        dacValue = dacValue + sequenceLeft->dacValue ;
        success = true;
      }
    }
  }
  return success;
}
bool
AcquisitionSequence::setAmplitude(AcquisitionSequence *sequenceLeft, AcquisitionSequence *sequenceRight) {
  
  bool success = false;
  if (sequenceLeft != (AcquisitionSequence *)NULL && sequenceRight != (AcquisitionSequence *)NULL ) {
    if (sequenceLeft->status == true && sequenceRight->status == true && sequenceLeft->data_2D_FLOAT != NULL
	&& sequenceRight->data_2D_FLOAT != NULL) {
      if (sequenceLeft->instrumentType == "TREATMENT" && 
	  sequenceRight->instrumentType == "TREATMENT") {
	instrumentRef = "CAMERA";
	if (reset == true) {
	  if (data_2D_FLOAT) {
	    free(data_2D_FLOAT);
	    data_2D_FLOAT = NULL;
	  }
	  if (sequenceLeft->data_2D_FLOAT_DIM_X <= sequenceRight->data_2D_FLOAT_DIM_X)
	    data_2D_FLOAT_DIM_X = sequenceLeft->data_2D_FLOAT_DIM_X;
	  else
	    data_2D_FLOAT_DIM_X = sequenceRight->data_2D_FLOAT_DIM_X;
	  if (sequenceLeft->data_2D_FLOAT_DIM_Y <= sequenceRight->data_2D_FLOAT_DIM_Y)
	    data_2D_FLOAT_DIM_Y = sequenceLeft->data_2D_FLOAT_DIM_Y;
	  else
	    data_2D_FLOAT_DIM_Y = sequenceRight->data_2D_FLOAT_DIM_Y;
	  data_2D_FLOAT = (float*) malloc (sizeof(float) * data_2D_FLOAT_DIM_X * data_2D_FLOAT_DIM_Y);
	  memset(data_2D_FLOAT,0,sizeof(data_2D_FLOAT));
	  reset = false;
	}
	// Amplitude calculation
	for (int i = 0 ; i < data_2D_FLOAT_DIM_X * data_2D_FLOAT_DIM_Y; i++) {
	  data_2D_FLOAT[i] = sqrt(sequenceLeft->data_2D_FLOAT[i] * sequenceLeft->data_2D_FLOAT[i] +
				  sequenceRight->data_2D_FLOAT[i] * sequenceRight->data_2D_FLOAT[i]);
	}
      }
      success = true;
    }
  }
  return success;
}
bool
AcquisitionSequence::setPhase(AcquisitionSequence *sequenceLeft, AcquisitionSequence *sequenceRight) {
  
  bool success = false;
  if (sequenceLeft != (AcquisitionSequence *)NULL && sequenceRight != (AcquisitionSequence *)NULL ) {
    if (sequenceLeft->status == true && sequenceRight->status == true && sequenceLeft->data_2D_FLOAT != NULL
	&& sequenceRight->data_2D_FLOAT != NULL) {
      if (sequenceLeft->instrumentType == "TREATMENT" && 
	  sequenceRight->instrumentType == "TREATMENT") {
	instrumentRef = "CAMERA";
	if (reset == true) {
	  if (data_2D_FLOAT) {
	    free(data_2D_FLOAT);
	    data_2D_FLOAT = NULL;
	  }
          if (sequenceLeft->data_2D_FLOAT_DIM_X <= sequenceRight->data_2D_FLOAT_DIM_X)
	    data_2D_FLOAT_DIM_X = sequenceLeft->data_2D_FLOAT_DIM_X;
	  else
	    data_2D_FLOAT_DIM_X = sequenceRight->data_2D_FLOAT_DIM_X;
	  if (sequenceLeft->data_2D_FLOAT_DIM_Y <= sequenceRight->data_2D_FLOAT_DIM_Y)
	    data_2D_FLOAT_DIM_Y = sequenceLeft->data_2D_FLOAT_DIM_Y;
	  else
	    data_2D_FLOAT_DIM_Y = sequenceRight->data_2D_FLOAT_DIM_Y;
	  data_2D_FLOAT = (float*) malloc (sizeof(float) * data_2D_FLOAT_DIM_X * data_2D_FLOAT_DIM_Y);
	  memset(data_2D_FLOAT,0,sizeof(data_2D_FLOAT));
	  reset = false;
	}
	// Phase calculation
	for (int i = 0 ; i < data_2D_FLOAT_DIM_X * data_2D_FLOAT_DIM_Y; i++) {
	  data_2D_FLOAT[i] = atan2(sequenceLeft->data_2D_FLOAT[i],sequenceRight->data_2D_FLOAT[i]);
	}
      }
      success = true;
    }
  }
  return success;
}
bool
AcquisitionSequence::setImage(AcquisitionSequence *sequenceLeft) {
  bool success = false;
  if (sequenceLeft != (AcquisitionSequence *)NULL && sequenceLeft->data_2D_FLOAT != NULL) {
    if (sequenceLeft->status == true) {
      if (sequenceLeft->instrumentType == "TREATMENT") {
	instrumentRef = "CAMERA";
	if (reset == true) {
	  if (image) {
	    setImage(NULL,0,0);
	  }
	  imageWidth = sequenceLeft->data_2D_FLOAT_DIM_X;
	  imageHeight = sequenceLeft->data_2D_FLOAT_DIM_Y;
	  image = (uchar*) malloc (sizeof(uchar) * imageWidth * imageHeight);
	  memset(image,0,sizeof(image));
	  reset = false;
	}
	// Calculate data_2D_FLOAT_MIN, data_2D_FLOAT_MAX for 2D array
	float absmax = -1e37;
	for (int i = 0 ; i < imageWidth * imageHeight; i++) {
	  if (sequenceLeft->data_2D_FLOAT[i] > absmax)
	    absmax = sequenceLeft->data_2D_FLOAT[i];
	}
	sequenceLeft->data_2D_FLOAT_MAX = absmax;
	float absmin = 1e37;
	for (int i = 0 ; i < imageWidth * imageHeight; i++) {
	  if (sequenceLeft->data_2D_FLOAT[i] < absmin)
	    absmin = sequenceLeft->data_2D_FLOAT[i];
	}
	sequenceLeft->data_2D_FLOAT_MIN = absmin;
	QLOG_DEBUG() << "MIN = " << absmin << ": MAX = " << absmax;
	// Convert 2D data array to image 8bits (0 - 255 range)
	for (int i = 0 ; i < imageWidth * imageHeight; i++) {
	  image[i] = (uchar) (( 255 * (sequenceLeft->data_2D_FLOAT[i] - sequenceLeft->data_2D_FLOAT_MIN) ) /
			      (sequenceLeft->data_2D_FLOAT_MAX - sequenceLeft->data_2D_FLOAT_MIN));
	}
	success = true;
      }
    }
  }
  return success;
}
