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

#ifndef ACQUISITIONSEQUENCE_H
#define ACQUISITIONSEQUENCE_H

#include <QtGui>
#include "hdf5.h"
#include "hdf5_hl.h"
#include "QsLog.h"

class AcquisitionSequence
{  
 public:
    
  AcquisitionSequence();
  ~AcquisitionSequence();
    
  int record;
  int seq_record;
  bool status;

  QString instrumentName;
  QString instrumentType;
  QString settings;
  QString scanplan;
  QString loopAction;
  QString loopEndAction;
  int     loopNumber;
  int     remainingLoops;
  
  QString avg ;
  QString avgOp;
  int avgSuccess;
  int avgRight;
  int avgLeft;
  QString typeAvg;
  
  // General attributes
  int sleep;

  // Motor attributes
  QString motorAction;
  float   motorValue; 
  float   position;
  // Dac attributes
  float   dacValue;
  int     dacOutput;

  // Camera attributes
  uchar *image;
  int   imageWidth;
  int   imageHeight;

  // data group
  QString group;
  QString datagroup;
  QString dataname;
  int     inc_group;
  hid_t   refgrp,grp,tmpgrp;
  QString grpname;

  void setImage(uchar* buffer, int width, int height);
  void prepare();
  uchar* getImage();
  bool setAvg(AcquisitionSequence *sequenceLeft, AcquisitionSequence *sequenceRight);
};

#endif // ACQUISITIONSEQUENCE_H
