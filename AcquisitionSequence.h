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
#include "FileParser.h"
#include "QsLog.h"



class AcquisitionSequence
{  
 public:
 
  enum {
   IS_LOOP = 0,
   LOOP_START = 1,
   LOOP_END = 2
  };
   
  AcquisitionSequence();
  ~AcquisitionSequence();

  double etime;   
  int record;
  int seq_record;
  bool status;

  QString instrumentName;
  QString instrumentType;
  QString settings;
  QString scanplan;
  int loop;
  int loopends;
  int loopNumber;
  int remainingLoops;
  
  // File attributes
  FileParser *fileParser;

  // Treatment attributes
  QString avg ;
  bool    reset;
  QString treatment;
  QString avgOp;
  int avgSuccess;
  int avgRight;
  int avgLeft;
  QString instrumentRef;
  
  // Acquisition attributes
  int sleep;

  // Motor attributes
  QString motorAction;
  QString superkAction;
  float   motorValue; 
  float   position;

  // SuperK attributes
  float   superkPowerValue;
  float   superkNdValue;
  float   superkSwpValue;
  float   superkLwpValue;
  float   superkCwValue;
  float   superkBwValue;
  float   superkRPowerValue;
  float   superkRNdValue;
  float   superkRSwpValue;
  float   superkRLwpValue;
  float   superkRCwValue;
  float   superkRBwValue;

  // Dac attributes
  double   dacValue;
  double   dacRValue;
  int      dacOutput;

  // Counter Comedi attributes
  int    comediValue;
  double comediData;
  int    comediOutput;

  // SLM attributes
  QString fullpath;
  QString imagepath;
  QString imagetype;
  QString keepzero;
  QImage *slmimage;
  int imgnum;
  int startnum;
  int stepnum;
  int slmwidth;
  int slmheight;
  int screen_x;
  int screen_y;
  QLabel *slmLabel;

  // Camera attributes
  float   *data_2D_FLOAT;
  int data_2D_FLOAT_DIM_X, data_2D_FLOAT_DIM_Y;
  float data_2D_FLOAT_MIN, data_2D_FLOAT_MAX;
  uchar *image;
  int   *image32;
  int   imageMin;
  int   imageMax;
  int   imageWidth;
  int   imageHeight;

  // HDF5 attributes
  QString group;
  QString datagroup;
  QString dataname;
  int     inc_group;
  hid_t   refgrp,grp,tmpgrp;
  QString grpname;

  void setImage(uchar* buffer, int width, int height);
  void setImage32(int* buffer32, int width, int height);
  bool setImage(AcquisitionSequence *sequenceLeft);
  void setImageMin(int _imageMin);
  void setImageMax(int _imageMax);
  void prepare();
  bool getFileData();
  uchar* getImage();
  int* getImage32();
  uchar* getImageFromFile();
  bool setAvg(AcquisitionSequence *sequenceLeft, AcquisitionSequence *sequenceRight);
  bool setPhase(AcquisitionSequence *sequenceLeft, AcquisitionSequence *sequenceRight);
  bool setAmplitude(AcquisitionSequence *sequenceLeft, AcquisitionSequence *sequenceRight);
  
};

#endif // ACQUISITIONSEQUENCE_H
