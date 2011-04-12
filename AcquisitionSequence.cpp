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
  instrumentName = "";
  instrumentType = "";
  settings = "";
  motorAction = "";
  motorValue = 0;
  dacValue = 0;
  dacOutput = 0;
  scanplan = "";
  loopAction = "";
  loopEndAction = "";
  datagroup = "";
  dataname = "";
  group = "";
  remainingLoops = 0;
  position = 0;
  inc_group = 0;
  image = NULL;
  refgrp = 0;
  grp = 0;
  tmpgrp = 0;
  grpname = "";
  avg = "";
  avgOp = "";
  avgSuccess = -1;
  avgLeft = -1;
  avgRight = -1;
  typeAvg = "";
  sleep = 0;
}

AcquisitionSequence::~AcquisitionSequence()
{
  QLOG_DEBUG ( ) <<"Deleting AcquisitionSequence";
    if (image) { free(image); image = NULL;}

}
void 
AcquisitionSequence::setImage(uchar* buffer, int width, int height) {
  if (image) { free(image); image = NULL;}
  if (buffer != NULL) {
    imageWidth = width;
    imageHeight = height;
    image = (uchar*) malloc (sizeof(uchar) * imageWidth * imageHeight);
    memcpy(image,buffer,sizeof(uchar) * imageWidth * imageHeight);
  }
}
uchar*
AcquisitionSequence::getImage() {
 return image;
}

void 
AcquisitionSequence::prepare() {
  loopAction = "";
  datagroup = "";
  remainingLoops = 0;
  QLOG_DEBUG ( ) <<"Sequence " << settings << ":" << scanplan;
  // Treat settingsList
  QStringList settingsList;
  QStringList subsettingsList;
  settingsList = settings.split(" ",QString::SkipEmptyParts);
  for (int i = 0 ; i < settingsList.size(); i++ ) {
    subsettingsList = settingsList.at(i).split("=",QString::SkipEmptyParts);
    for (int j = 0 ; j < subsettingsList.size(); j++ ) {
      if (subsettingsList.at(j).leftRef(4) == "MOVE" && subsettingsList.size() > j + 1) {
	motorAction = subsettingsList.at(j);
	motorValue = subsettingsList.at(j+1).toFloat();
      }
      else if (subsettingsList.at(j) == "OUTPUT" && subsettingsList.size() > j + 1) {
	dacOutput = subsettingsList.at(j+1).toInt();
      }
      else if (subsettingsList.at(j) == "VALUE" && subsettingsList.size() > j + 1) {
	dacValue = subsettingsList.at(j+1).toFloat();
      }
      else if (subsettingsList.at(j) == "SLEEP" && subsettingsList.size() > j + 1) {
	sleep = (int) (subsettingsList.at(j+1).toFloat()*1e6);
      }
      else if (subsettingsList.at(j) == "AVG" && subsettingsList.size() > j + 1) {
	avg = subsettingsList.at(j+1);
	// Treat avgSettings
	QString subavg;
	QString subsubavg;
	subavg = avg.mid(avg.indexOf("[") + 1, avg.indexOf("]") - 1);

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
    }
  }
  
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
	loopAction = subscanplanList.at(j);
	loopNumber = subscanplanList.at(j+1).toInt();
	loopEndAction = "START";
	if (loopNumber == 0)
	  loopEndAction = subscanplanList.at(j+1);
	remainingLoops = loopNumber;		  
      }
    }
  }  
}

bool
AcquisitionSequence::setAvg(AcquisitionSequence *sequenceLeft, AcquisitionSequence *sequenceRight) {
  
  bool success = false;
  if (sequenceLeft != (AcquisitionSequence *)NULL && sequenceRight != (AcquisitionSequence *)NULL ) {
    if (sequenceLeft->status == true && sequenceRight->status == true) {
      if (sequenceLeft->instrumentType == "CAMERA" && 
	  sequenceRight->instrumentType == "CAMERA") {
	typeAvg = "CAMERA";
	uchar *imageRight = sequenceRight->getImage();
	uchar *imageLeft = sequenceLeft->getImage();
	if (image == NULL) {
	  imageWidth = sequenceLeft->imageWidth;
	  imageHeight = sequenceLeft->imageHeight;
	  image = (uchar*) malloc (sizeof(uchar) * imageWidth * imageHeight) ;
	  memset(image,0,sizeof(image));
	}
	for (int i = 0 ; i < imageWidth * imageHeight; i++) {
	  if ( avgOp == "+" ) 
	    image[i] = image[i] + (imageLeft[i] + imageRight[i]);
	  else if ( avgOp == "-" ) 
	    image[i] = image[i] + (imageLeft[i] - imageRight[i]);
	  else if ( avgOp == "*" )
	    image[i] = image[i] + (imageLeft[i] * imageRight[i]);
	  else if ( avgOp == "/") {
	    if (imageRight[i] != 0)
	      image[i] = image[i] + (imageLeft[i] / imageRight[i]);
	  }
	}
	success = true;
      }
      else if (sequenceLeft->instrumentType == "DAC" && 
	       sequenceRight->instrumentType == "DAC") {
	typeAvg = "DAC";
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
    }
  }
  else if (sequenceLeft != (AcquisitionSequence *)NULL && sequenceRight == (AcquisitionSequence *)NULL ) {
		QLOG_DEBUG ( ) << "sequenceLeft not NULL && sequenceRight is NULL";
    if (sequenceLeft->status == true) {
	QLOG_DEBUG ( ) << "sequenceLeft is true";
      if ( sequenceLeft->instrumentType == "CAMERA" ) {
	typeAvg = "CAMERA";
	uchar *imageLeft = sequenceLeft->getImage();
	if (image == NULL) {
	  imageWidth = sequenceLeft->imageWidth;
	  imageHeight = sequenceLeft->imageHeight;
	  image = (uchar*) malloc (sizeof(uchar) * imageWidth * imageHeight) ;
	  memcpy(image,imageLeft,sizeof(uchar) * imageWidth * imageHeight);
	}
	else {
	  for (int i = 0 ; i < imageWidth * imageHeight; i++)
	    image[i] = image[i] + imageLeft[i];
	}
	success = true;
      }
      else if ( sequenceLeft->instrumentType == "DAC" ) {
	typeAvg = "DAC";
	dacValue = dacValue + sequenceLeft->dacValue ;
	success = true;
      }
    }
  }
  return success;
}
