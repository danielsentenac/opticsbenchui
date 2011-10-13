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

#include "AcquisitionThread.h"
#include <unistd.h>
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

AcquisitionThread::AcquisitionThread( QObject* parent)
	      :QThread(parent)
{
  imageBuffer = NULL;
  suspend = true;
}

AcquisitionThread::~AcquisitionThread()
{
  QLOG_DEBUG ( ) << "Deleting AcquisitionThread";
  stop();
}
void 
AcquisitionThread::setCamera(QVector<Camera*> _cameraList){
  cameraList = _cameraList;
}
void 
AcquisitionThread::setMotor(Motor* _motor){
  motor = _motor;
}
void 
AcquisitionThread::setDac(Dac* _dac){
  dac = _dac;
}
void 
AcquisitionThread::setFile(QString _filename, int _filenumber) {
  filename = _filename;
  filenumber = _filenumber;
}
void 
AcquisitionThread::setSequenceList(QVector<AcquisitionSequence*> _sequenceList)
{
  sequenceList = _sequenceList;
}
void 
AcquisitionThread::stop(){
  suspend = true;
  wait();
  exit();
}
void 
AcquisitionThread::run() {
  if (suspend == true) {
    suspend = false;

    /* create a new data File */
    file_id = H5Fcreate(filename.toStdString().c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if ( file_id < 0 ) {
      QLOG_WARN () << "Unable to open file - " << filename << " - Aborting scan";
      emit showWarning("Unable to open file - " + filename + " - Aborting scan");
      return;
    }
    record = 0;
    int lastrecord = sequenceList.size();
    QLOG_DEBUG() << "AcquisitionThread::run> Number of sequences " << lastrecord;
    // run acquisition sequence
    QLOG_INFO() << " Start Acquisition : " << QDateTime::currentDateTime().toString("MMMdd,yy-hh:mm:ss");
    emit getAcquiring(record);
    while ( record < lastrecord ) {
      AcquisitionSequence *sequence = sequenceList.at(record);
      
      sequence->etime = GetTime();
      int cur_record = record;
      this->execute(sequence);
      this->nextRecord(sequence,cur_record);
      QLOG_DEBUG() << "AcquisitionThread::run> Start sequence " << sequence->seq_record;
      this->saveData(sequence,cur_record);
      usleep(sequence->sleep);
      if (suspend == true) break;
      sequence->etime = GetTime() - sequence->etime;
      if ( H5Fflush(file_id,H5F_SCOPE_GLOBAL) < 0)
	QLOG_WARN() << " File flushing failed !";
      H5garbage_collect();
      QLOG_DEBUG() << "Sequence " << sequence->seq_record << ": etime = " << (int)sequence->etime;
    }
    QLOG_INFO() << " Stop Acquisition : " << QDateTime::currentDateTime().toString("MMMdd,yy-hh:mm:ss");
    emit getAcquiring(record);
    filenumber++;
    emit getFilenumber(filenumber);
    // close all groups
    for (int i = ids.size(); i >= 0 ; i--)
      H5Gclose(ids.at(i));
    ids.clear();
    status = H5Fclose(file_id);
    // Close eventually open camera
    for (int i = 0 ; i <   cameraList.size(); i++) {
      Camera *camera = cameraList.at(i);
      if (isopencamerawindow.at(i) == false)
	camera->stop();      
    }
    for (int i = 0 ; i <   sequenceList.size(); i++) {
      AcquisitionSequence *sequence = sequenceList.at(i);
      if ( sequence->instrumentType == "DAC" ) {
        QLOG_DEBUG() << "AcquisitionThread::execute> Connecting DAC ...";
        // Update DB Dac values
        if (dac != NULL)  dac->updateDBValues(sequence->instrumentName);
      }
      delete sequence;
    }
    sequenceList.clear(); 
  }
  suspend = true;
  QLOG_DEBUG() << "AcquisitionThread::run> End of thread";
}
void AcquisitionThread::execute(AcquisitionSequence *sequence) {
  
  // New acquisition

  //emit getAcquiring(record);
  
  QLOG_DEBUG() << "AcquisitionThread::execute> Execute record " << record << " instrument " 
	       <<  sequence->instrumentType
	       << ":" << sequence->instrumentName << " remaining loops " 
	       << sequence->loopAction << ":"
	       << sequence->loopEndAction << ":" << sequence->remainingLoops;
  
  if ( sequence->instrumentType == "MOTOR" ) {
    motor->connectMotor(sequence->instrumentName);
    
    if ( sequence->motorAction == "MOVEREL" ) {
      if ( sequence->motorValue < 0 )
	motor->moveBackward(sequence->instrumentName,(-sequence->motorValue));
      else
	motor->moveForward(sequence->instrumentName,sequence->motorValue);
    }
    else if (sequence->motorAction == "MOVEABS" ) 
      motor->moveAbsolute(sequence->instrumentName,sequence->motorValue);
    // Wait for motor movement to be completed
    while (motor->getOperationComplete(sequence->instrumentName) <= 0 ) {
      if (suspend == true) motor->stopMotor(sequence->instrumentName);
      usleep(100);
      motor->operationComplete();
    }
    // New position
    QString positionQString;
    positionQString.setNum (motor->getPosition(sequence->instrumentName), 'f',3);
    emit getPosition(positionQString);
    sequence->status = true;
    // Save motor position data
    sequence->position = motor->getPosition(sequence->instrumentName);
  }
  else if ( sequence->instrumentType == "DAC" ) {
    QLOG_DEBUG() << "AcquisitionThread::execute> Connecting DAC ...";
    dacsuccess = dac->connectDac(sequence->instrumentName);
    QLOG_DEBUG() << "AcquisitionThread::execute> DAC " << sequence->instrumentName 
		 << " connected " << dacsuccess;
	    
    if (dacsuccess == true) {
      dacsuccess = dac->setDacValue(sequence->instrumentName,
				    sequence->dacOutput,
				    sequence->dacValue);
    }
    sequence->status = dacsuccess;
    //emit getDacStatus(dacsuccess);
  }
  else if ( sequence->instrumentType == "CAMERA" ) {
    imagesuccess = false;
    int cameranumber = sequence->instrumentName.toInt();
    if (cameraList.size() > cameranumber) {
      Camera *camera = cameraList.at(cameranumber);
      if (camera->suspend == true)  {
        QLOG_DEBUG() << "OPEN CAMERA START";
	camera->start();
        while (camera->has_started == false)
	  usleep(100);
      }
      QLOG_DEBUG() << " Wait for Image Acquisition ";
      camera->mutex->lock();
      camera->acqstart->wait(camera->mutex);
      camera->mutex->unlock();
      camera->mutex->lock();
      camera->acqend->wait(camera->mutex);
      camera->mutex->unlock();
      QLOG_DEBUG() << " Save Image buffer ";
      setImageFromCamera(camera->getSnapshot(),
			 camera->width,
			 camera->height,
			 camera->video_mode);
      sequence->setImageMin(camera->snapShotMin);
      sequence->setImageMax(camera->snapShotMax);
      sequence->setImage(imageBuffer,imageWidth, imageHeight, videoMode);
    }
    
    //emit getCameraStatus(imagesuccess);
    sequence->status = imagesuccess;
  }
  else if ( sequence->instrumentType == "TREATMENT" ) {
    if ( sequence->treatment != "" && sequence->avg != "") {
      // Treat an average on selected records
      AcquisitionSequence *sequenceLeft = (AcquisitionSequence *)NULL;
      AcquisitionSequence *sequenceRight = (AcquisitionSequence *)NULL;
      if (sequence->avgLeft >= 0 ) {
	for (int k = 0 ; k < sequenceList.size(); k++) {
	  AcquisitionSequence *tmpSequence = sequenceList.at(k);
	  if (tmpSequence->seq_record == sequence->avgLeft) {
	    sequenceLeft = tmpSequence;
	    break;
	  }
	}
      }
      if (sequence->avgRight >= 0 ) {
	for (int k = 0 ; k < sequenceList.size(); k++) {
	  AcquisitionSequence *tmpSequence = sequenceList.at(k);
	  if (tmpSequence->seq_record == sequence->avgRight) {
	    sequenceRight = tmpSequence;
	    break;
	  }
	}
      }
      if ( sequence->treatment == "AVG" )
	treatmentsuccess = sequence->setAvg(sequenceLeft,sequenceRight);
      else if ( sequence->treatment == "PHASE" )
	treatmentsuccess = sequence->setPhase(sequenceLeft,sequenceRight);
      else if ( sequence->treatment == "AMPLITUDE" )
	treatmentsuccess = sequence->setAmplitude(sequenceLeft,sequenceRight);
      else if ( sequence->treatment == "IMAGE" )
	treatmentsuccess = sequence->setImage(sequenceLeft);
      //emit getTreatmentStatus(treatmentsuccess);
      sequence->status = treatmentsuccess;
    }
  }
}
void AcquisitionThread::nextRecord(AcquisitionSequence *sequence, int cur_record) {
  
  if ( sequence->loopAction == "LOOP" && sequence->loopEndAction == "END" ) {
    QLOG_DEBUG() << "AcquisitionThread::nextRecord> End of loop at record " << cur_record << " loopAction "
		 << sequence->loopAction << " loopNumber " <<  sequence->loopNumber
		 << " instrument " << sequence->instrumentType << ":" << sequence->instrumentName;
    
    // go back to starting loop record 
    int loopnumber = 0;
    for ( int i = cur_record - 1; i >= 0 ; i-- ) {
      AcquisitionSequence *tmpSequence = sequenceList.at(i);
      QLOG_DEBUG() << "AcquisitionThread::nextRecord> Search start of loop at record " 
		   << cur_record << " loopAction "
		   << tmpSequence->loopAction << " loopEndAction " << tmpSequence->loopEndAction
		   << " loopNumber " << tmpSequence->loopNumber << " instrument "
		   << tmpSequence->instrumentType << ":" << tmpSequence->instrumentName;
	      
      if (tmpSequence->loopAction == "LOOP" && tmpSequence->loopEndAction =="END") {
	loopnumber++;
	continue;
      }
      if (tmpSequence->loopAction == "LOOP" && tmpSequence->loopEndAction =="START") {
	if (loopnumber > 0 ) {
	  loopnumber--;
	  continue;
	}
	else if (loopnumber == 0 && tmpSequence->loopNumber > 0) {
	  QLOG_DEBUG() << "AcquisitionThread::nextRecord> Start of loop at record " 
		       << tmpSequence->record << " loopAction "
		       << tmpSequence->loopAction << " loopNumber " 
		       << tmpSequence->loopNumber
		       << " instrument " << tmpSequence->instrumentType << ":" 
		       << tmpSequence->instrumentName;

	  if ( tmpSequence->remainingLoops > 1 ) {
	    record = tmpSequence->record;
	    tmpSequence->remainingLoops--;
	    // Set new group name
	    sequence->group = sequence->datagroup + "_" + QString::number(sequence->inc_group);
	    sequence->inc_group++;
	    return;
	  }
	  else {
	    // reset remainScans and go to next record
	    QLOG_DEBUG() << "AcquisitionThread::nextRecord> RemainingLoops = 1";
	    tmpSequence->remainingLoops = tmpSequence->loopNumber;
	    // reset inc_group number for the entire loop sequence
	    for ( int j = cur_record; j >= i ; j-- ) {
	      AcquisitionSequence *tmpSequencebis = sequenceList.at(j);
	      tmpSequencebis->inc_group = 0;
	    }
	    record++;
	    return;
	  }
	}	
      }
    }
  }
  else {
    // go to next record
    record++;
    // Set new group name
    sequence->group = sequence->datagroup + "_" + QString::number(sequence->inc_group);
    sequence->inc_group++;
  }
}
void AcquisitionThread::saveData(AcquisitionSequence *sequence, int cur_record) {

  AcquisitionSequence *parentSequence = NULL;
  AcquisitionSequence *grandparentSequence = NULL;
  int parent_record = 0;
  /* Turn off error handling */
  hid_t error_stack = 0;
  H5Eset_current_stack(error_stack);
  H5Eset_auto(error_stack, NULL, NULL);

  
  if (sequence->refgrp == 0) {
    sequence->refgrp = file_id;
    sequence->tmpgrp = file_id;
  }
  
  //Try to open parent reference group
  int jumpdatagroup = 0;
  for ( int i = cur_record - 1; i >= 0 ; i-- ) {
    AcquisitionSequence *tmpSequence = sequenceList.at(i);
     if (tmpSequence->seq_record < 0 )
      continue;
    // if (tmpSequence->loopAction == "LOOP" && tmpSequence->loopEndAction =="START") {
    if (tmpSequence->loopAction == "LOOP" && tmpSequence->loopEndAction =="END") {
      jumpdatagroup++;
      QLOG_DEBUG() << "AcquisitionThread::saveData> must jump to next DATAGROUP" << jumpdatagroup;
      continue;
    } 
    if (tmpSequence->datagroup != "" ) {
      if ( jumpdatagroup > 0 ) {
	jumpdatagroup--;
	QLOG_DEBUG() << "AcquisitionThread::saveData> Found datagroup not null" << jumpdatagroup;
	continue;
      }
      QLOG_DEBUG() << "AcquisitionThread::saveData> Found parent reference loop at record " 
		  << i << " loopAction "
		  << tmpSequence->loopAction << " loopEndAction " << tmpSequence->loopEndAction
		  << " loopNumber " << tmpSequence->loopNumber << " instrument " 
		  << tmpSequence->instrumentType << ":" << tmpSequence->instrumentName
		  << " group " << tmpSequence->group << " grp " << tmpSequence->grp 
		  << " refgrp " << tmpSequence->refgrp;
      
      sequence->refgrp = tmpSequence->refgrp;
      sequence->grp = tmpSequence->grp;
      sequence->tmpgrp = tmpSequence->grp;
      sequence->grpname = tmpSequence->group;
      parentSequence = tmpSequence;
      parent_record = i;
      break;
    }
  }
  if (parent_record > 0) {
    //Try to open grand parent reference group
    for ( int j = parent_record - 1; j >= 0 ; j-- ) {
      AcquisitionSequence *tmpSequence = sequenceList.at(j);
      //if (tmpSequence->loopAction == "LOOP" && tmpSequence->loopEndAction =="START") {
      if (tmpSequence->datagroup != "") {
	QLOG_DEBUG() << "AcquisitionThread::saveData> Found grand parent reference loop at record " 
		     << j << " loopAction "
		     << tmpSequence->loopAction << " loopNumber " << tmpSequence->loopNumber
		     << " instrument " << tmpSequence->instrumentType << ":" 
		     << tmpSequence->instrumentName
		     << " group " << tmpSequence->group << " grp " << tmpSequence->grp 
		     << " refgrp " << tmpSequence->refgrp;

	grandparentSequence = tmpSequence;
	break;
      }
    }
  }
 
  if (sequence->datagroup != "") {
    QLOG_DEBUG() << "AcquisitionThread::saveData> Open new group " << sequence->datagroup << " in " << sequence->grpname << "(" 
		<< sequence->tmpgrp << ")";
    // Create new sub group
    sequence->grpname = sequence->group;
    sequence->grp = H5Gopen2(sequence->tmpgrp, sequence->grpname.toStdString().c_str(), 
			     H5P_DEFAULT);
    if ( sequence->grp < 0 ) {
      sequence->grp = H5Gcreate2(sequence->tmpgrp, sequence->grpname.toStdString().c_str(), 
				 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT); 
      
      QLOG_DEBUG() << "AcquisitionThread::saveData> Created new group " 
		   << sequence->grp << " in reference group " 
		   << sequence->tmpgrp;
      // store new id group for future deletion
      ids.push_back(sequence->grp);
    }
    sequence->refgrp = sequence->tmpgrp;
  }
  else if (sequence->grpname == "")
    return;
  
  QLOG_DEBUG() << "AcquisitionThread::saveData> Save " << sequence->dataname 
	       << " in refgrp " << sequence->refgrp << " grp "
	       << sequence->grp << " grpname " << sequence->grpname;
	   
  // No dataname means no data to be saved
  if (sequence->dataname == "") return;

  // Save MOTOR data in the group
  if (sequence->instrumentType == "MOTOR")
    status = H5LTset_attribute_float(sequence->refgrp, sequence->grpname.toStdString().c_str(), 
				     sequence->dataname.toStdString().c_str(), 
				     &(sequence->position),1);
  // Save DAC data in the group
  if (sequence->instrumentType == "DAC" && dacsuccess == true) {
    status = H5LTset_attribute_float(sequence->refgrp, sequence->grpname.toStdString().c_str(), 
				     sequence->dataname.toStdString().c_str(), 
				     &(sequence->dacValue),1);
  }
  // Save CAMERA data in the group
  if (sequence->instrumentType == "CAMERA" && imagesuccess == true) {
    hsize_t dset_dims[2];
    dset_dims[0] = sequence->imageWidth;
    dset_dims[1] = sequence->imageHeight;
    /* create and write an double type dataset named "dset" */
    QString video_mode (iidc_video_modes[sequence->videoMode - VIDEO_MODES_OFFSET]);
    status = H5IMmake_image_8bit(sequence->grp,sequence->dataname.toStdString().c_str(),
				 sequence->imageWidth,sequence->imageHeight,sequence->image);
    status = H5LTset_attribute_int(sequence->grp, sequence->dataname.toStdString().c_str(), 
				   "min", 
				   &sequence->imageMin,1);
    status = H5LTset_attribute_int(sequence->grp, sequence->dataname.toStdString().c_str(), 
				   "max", 
				   &sequence->imageMax,1);
				   
    
  }
  QLOG_DEBUG() << "AcquisitionThread::saveData> sequence->treatment " << sequence->treatment;
  if (sequence->instrumentType == "TREATMENT") {
    if (sequence->treatment != "" && sequence->avg != "" ) {
      // Check parent group and save average in grand parent group
      QLOG_DEBUG() << "AcquisitionThread::saveData> Save avergage data remainingLoops " 
		   << parentSequence->remainingLoops;
      if ( sequence->treatment == "AVG" && parentSequence->remainingLoops == 1 && grandparentSequence != NULL) {
	if (sequence->instrumentRef == "CAMERA" && treatmentsuccess == true) {
	  hsize_t dset_dims[2];
	  dset_dims[0] = sequence->data_2D_FLOAT_DIM_Y;
	  dset_dims[1] = sequence->data_2D_FLOAT_DIM_X;
	  // Average data
	  for (int i = 0 ; i < sequence->data_2D_FLOAT_DIM_X * sequence->data_2D_FLOAT_DIM_Y; i++)
	    sequence->data_2D_FLOAT[i] /= parentSequence->loopNumber;
	  status = H5LTmake_dataset_float(grandparentSequence->grp,sequence->dataname.toStdString().c_str(),
					  2,dset_dims,sequence->data_2D_FLOAT);
	  // reset sequence data
	  sequence->reset =true;
	}
	else if (sequence->instrumentRef == "DAC" && treatmentsuccess == true ) {
	  QLOG_DEBUG() << "AcquisitionThread::saveData> Save avergage DAC data in groupname " 
		       << grandparentSequence->grpname
		       << " refgrp " << grandparentSequence->refgrp;
	  
	  // Average data
	  sequence->dacValue /= parentSequence->loopNumber;
	  status = H5LTset_attribute_float(grandparentSequence->refgrp, 
					   grandparentSequence->grpname.toStdString().c_str(), 
					   sequence->dataname.toStdString().c_str(), 
					   &(sequence->dacValue),1);
	  // reset sequence data
	  sequence->reset =true;	  
	}
      }
      else if ( sequence->treatment == "PHASE" || sequence->treatment == "AMPLITUDE" ) {
	QLOG_DEBUG() << "AcquisitionThread::saveData> Save PHASE/AMPLITUDE data in groupname " 
		    << sequence->grpname
		    << " refgrp " << sequence->grp;
	QLOG_DEBUG() << "AcquisitionThread::saveData> instrumentRef " << sequence->instrumentRef;
	QLOG_DEBUG() << "AcquisitionThread::saveData> treatmentsuccess " << treatmentsuccess;
	if (sequence->instrumentRef == "CAMERA" && treatmentsuccess == true) {
	  hsize_t dset_dims[2];
	  dset_dims[0] = sequence->data_2D_FLOAT_DIM_Y;
	  dset_dims[1] = sequence->data_2D_FLOAT_DIM_X;
	  status = H5LTmake_dataset_float(sequence->grp,sequence->dataname.toStdString().c_str(),
					  2,dset_dims,sequence->data_2D_FLOAT);
	  // reset data
	  sequence->reset =true;
	}
      }
      else if (sequence->treatment == "IMAGE" ) {
	QLOG_DEBUG() << "AcquisitionThread::saveData> Save converted IMAGE data in groupname " 
		     << sequence->grpname
		     << " refgrp " << sequence->refgrp;
	QLOG_DEBUG() << "AcquisitionThread::saveData> instrumentRef " << sequence->instrumentRef;
	QLOG_DEBUG() << "AcquisitionThread::saveData> treatmentsuccess " << treatmentsuccess;
	if (sequence->instrumentRef == "CAMERA" && treatmentsuccess == true) {
	  status = H5IMmake_image_8bit(sequence->grp,sequence->dataname.toStdString().c_str(),
				       sequence->imageWidth,sequence->imageHeight,sequence->image);
	  // reset data
	  sequence->reset =true;
	}
      }
    }
  }
}
void AcquisitionThread::setImageFromCamera(uchar *buffer, int width, int height, int videomode) {
  if (imagesuccess == false) {
    QString video_mode (iidc_video_modes[videomode - VIDEO_MODES_OFFSET]);
    QLOG_DEBUG() << " Video Mode : " << videomode<< " : " << video_mode;
    imageWidth = width;
    imageHeight = height;
    videoMode = videomode;
    if (imageBuffer) { free (imageBuffer); imageBuffer = NULL;}
    imageBuffer = (uchar*)malloc(sizeof(uchar)*imageWidth*imageHeight);
    memcpy(imageBuffer,buffer,width * height);
    QLOG_DEBUG() << "AcquisitionThread::setImageFromCamera> Caught snapshot image";
    imagesuccess = true;
  }
}
