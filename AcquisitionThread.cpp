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
  mutex = new QMutex(QMutex::NonRecursive);
  splashScreenOk = new QWaitCondition();
  suspend = true;
}

AcquisitionThread::~AcquisitionThread()
{
  QLOG_DEBUG ( ) << "Deleting AcquisitionThread";
  stop();
  delete mutex;
  delete splashScreenOk;
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
AcquisitionThread::setComediCounter(Comedi* _comedi){
  comedicounter = _comedi;
}
void
AcquisitionThread::setComediDac(Comedi* _comedi){
  comedidac = _comedi;
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
    QLOG_INFO() << "AcquisitionThread::run> start Acquisition : " 
		<< QDateTime::currentDateTime().toString("MMMdd,yy-hh:mm:ss");
    emit getAcquiring(record);
    while ( record < lastrecord ) {
      AcquisitionSequence *sequence = sequenceList.at(record);
      sequence->etime = GetTime();
      int cur_record = record;
      this->execute(sequence);
      this->nextRecord(sequence,cur_record);
      QLOG_DEBUG() << "AcquisitionThread::run> Start sequence " << sequence->seq_record;
      this->saveData(sequence,cur_record);
      if (sequence->sleep > 0) 
         usleep(sequence->sleep);
      sequence->etime = GetTime() - sequence->etime;
      if (suspend == true) break;
      if (H5Fflush(file_id,H5F_SCOPE_GLOBAL) < 0)  {
	QLOG_WARN() << " File flushing failed !";
      }
      H5garbage_collect();
      QLOG_INFO() << "AcquisitionThread::run> sequence " << sequence->seq_record 
		  << ": etime = " << (int)sequence->etime;
    }
    QLOG_INFO() << "AcquisitionThread::run> stop Acquisition : " 
		<< QDateTime::currentDateTime().toString("MMMdd,yy-hh:mm:ss");
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
      emit getAcquiring(sequence->record);
      if ( sequence->instrumentType == "DAC" ) {
        QLOG_DEBUG() << "AcquisitionThread::execute> Update Db DAC values...";
        // Update DB Dac values
        if (dac != NULL)  dac->updateDBValues(sequence->instrumentName);
      }
      if ( sequence->instrumentType == "COMEDICOUNTER" ) {
        QLOG_DEBUG() << "AcquisitionThread::execute> Update Db COMEDI COUNTER values...";
        // Update DB Counter values
        if (comedicounter != NULL)  comedicounter->updateDBValues(sequence->instrumentName);
      }
      if ( sequence->instrumentType == "COMEDIDAC" ) {
        QLOG_DEBUG() << "AcquisitionThread::execute> Update Db COMEDI DAC values...";
        // Update DB Counter values
        if (comedidac != NULL)  comedidac->updateDBValues(sequence->instrumentName);
      }

      if ( sequence->instrumentType == "MOTOR" ) {
        QLOG_DEBUG() << "AcquisitionThread::execute> Update Db MOTOR values...";
        // Emit last position
        QString positionQString;
        positionQString.setNum (sequence->position, 'f',3);
        emit getPosition(positionQString);
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
	       << sequence->remainingLoops << ":"
	       << sequence->loopends << ":" << sequence->loopends;
  
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
    //emit getPosition(positionQString);
    QLOG_INFO () << "AcquisitionThread::execute> " << sequence->instrumentName 
                 << " new position:" << positionQString; 
    sequence->status = true;
    // Save motor position data
    sequence->position = motor->getPosition(sequence->instrumentName);
  }
  else if ( sequence->instrumentType == "DAC" ) {
    QLOG_INFO() << "AcquisitionThread::execute> connecting DAC ..." << sequence->instrumentName;
    dacsuccess = dac->connectDac(sequence->instrumentName);
    QLOG_INFO() << "AcquisitionThread::execute> DAC " << sequence->instrumentName 
		 << " connected " << dacsuccess;
	    
    if (dacsuccess == true) {
      dacsuccess = dac->setDacValue(sequence->instrumentName,
				    sequence->dacOutput,
				    sequence->dacValue);
    }
    sequence->status = dacsuccess;
    //emit getDacStatus(dacsuccess);
  }
  else if ( sequence->instrumentType == "COMEDICOUNTER" ) {
    QLOG_DEBUG() << "AcquisitionThread::execute> connecting COMEDI COUNTER ..." << sequence->instrumentName;
    comedicountersuccess = comedicounter->connectComedi(sequence->instrumentName);
    QLOG_DEBUG() << "AcquisitionThread::execute> COMEDI COUNTER " << sequence->instrumentName
                 << " connected " << comedicountersuccess;

    if (comedicountersuccess == true) {
     comedicountersuccess = comedicounter->setComediValue(sequence->instrumentName,
                                             sequence->comediOutput,
                                             (void*)&sequence->comediValue);
     // Copy comedi data in sequence for saving later
     comedicountersuccess = comedicounter->getComediValue(sequence->instrumentName,
                                            sequence->comediOutput,
                                            sequence->comediData);
    }
    sequence->status = comedicountersuccess;
    //emit getComediStatus(comedicountersuccess);
  }
  else if ( sequence->instrumentType == "COMEDIDAC" ) {
    QLOG_DEBUG() << "AcquisitionThread::execute> connecting COMEDI DAC ..." << sequence->instrumentName;
    comedidacsuccess = comedidac->connectComedi(sequence->instrumentName);
    QLOG_DEBUG() << "AcquisitionThread::execute> COMEDI DAC " << sequence->instrumentName
                 << " connected " << comedidacsuccess;

    if (comedidacsuccess == true) {
     comedidacsuccess = comedidac->setComediValue(sequence->instrumentName,
                                             sequence->comediOutput,
                                             (void*)&sequence->comediValue);
     // Copy comedi data in sequence for saving later
     comedidacsuccess = comedidac->getComediValue(sequence->instrumentName,
                                            sequence->comediOutput,
                                            sequence->comediData);
    }
    sequence->status = comedidacsuccess;
    //emit getComediStatus(comedidacsuccess);
  }
  else if ( sequence->instrumentType == "CAMERA" ) {
    imagesuccess = false;
    bool cameraExists = false;
    Camera *camera  = NULL;
    for (int i = 0 ; i < cameraList.size(); i++) { 
     camera = cameraList.at(i);
     if ( camera->model == sequence->instrumentName ) 
      cameraExists = true;
      break;
     }
     if (cameraExists == true ) {
      if (camera->suspend == true)  {
        QLOG_INFO() << "AcquisitionThread::execute> open CAMERA " << camera->model;
	camera->start();
        while (camera->has_started == false)
	  usleep(100);
      }
      QLOG_INFO() << "AcquisitionThread::execute>  wait for Image Acquisition ";
      camera->mutex->lock();
      camera->acqstart->wait(camera->mutex);
      camera->mutex->unlock();
      camera->mutex->lock();
      camera->acqend->wait(camera->mutex);
      camera->mutex->unlock();
      if ( sequence->settings == "SNAPSHOT" ) 
        sequence->setImage(camera->getSnapshot(),
                           camera->width,
                           camera->height);
      else if ( sequence->settings == "SNAPSHOT32" )
        sequence->setImage32(camera->getSnapshot32(),
                             camera->width,
                             camera->height);
      sequence->setImageMin(camera->snapShotMin);
      sequence->setImageMax(camera->snapShotMax);
      QLOG_INFO() << " Save Image buffer in sequence from " << camera->vendor
                  << " imageMin " << sequence->imageMin
                  << " imageMax " << sequence->imageMax;
      imagesuccess = true;
    }
    else
      QLOG_WARN() << "CAMERA does not exist...";
    
    //emit getCameraStatus(imagesuccess);
    sequence->status = imagesuccess;
  }
  else if ( sequence->instrumentType == "SLM" ) {
     sequence->imgnum = sequence->imgnum +  sequence->stepnum;
     sequence->fullpath = sequence->imagepath + //sequence->keepzero + 
			QString::number(sequence->imgnum) + "." + sequence->imagetype;
     QLOG_INFO() << sequence->fullpath;
     QFile file(sequence->fullpath);
     if ( file.exists() == true ) {
       emit splashScreen(sequence->fullpath, sequence->screen_x, sequence->screen_y);
       mutex->lock();
       splashScreenOk->wait(mutex);
       mutex->unlock();
       slmsuccess = true;
     }
     else {
        QLOG_ERROR() << sequence->fullpath << " not found ! (skip it)";
        slmsuccess = false;
     }
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
  else if ( sequence->instrumentType == "FILE" ) {
    filesuccess = sequence->getFileData();
    QLOG_INFO() << "AcquisitionThread::execute()> type=FILE " << filesuccess;
  }
}
void AcquisitionThread::nextRecord(AcquisitionSequence *sequence, int cur_record) {
  
  if ( sequence->loop == AcquisitionSequence::IS_LOOP && 
       sequence->loopends == AcquisitionSequence::LOOP_END ) {
   QLOG_DEBUG() << "AcquisitionThread::nextRecord> End of loop at record " 
	 << cur_record << " loop "
	 << sequence->loop << " loopNumber " <<  sequence->loopNumber
	 << " instrument " << sequence->instrumentType << ":" << sequence->instrumentName;
    
    // go back to starting loop record 
    int loopnumber = 0;
    for ( int i = cur_record - 1; i >= 0 ; i-- ) {
      AcquisitionSequence *tmpSequence = sequenceList.at(i);
      QLOG_DEBUG() << "AcquisitionThread::nextRecord> Search start of loop at record " 
      		   << cur_record << " loop "
		   << tmpSequence->loop << " loopends " << tmpSequence->loopends
		   << " loopNumber " << tmpSequence->loopNumber << " instrument "
		   << tmpSequence->instrumentType << ":" << tmpSequence->instrumentName;
	      
      if (tmpSequence->loop == AcquisitionSequence::IS_LOOP && 
          tmpSequence->loopends == AcquisitionSequence::LOOP_END) {
	loopnumber++;
	continue;
      }
      if (tmpSequence->loop == AcquisitionSequence::IS_LOOP && 
          tmpSequence->loopends == AcquisitionSequence::LOOP_START) {
	if (loopnumber > 0 ) {
	  loopnumber--;
	  continue;
	}
	else if (loopnumber == 0 && tmpSequence->loopNumber > 0) {
	  QLOG_DEBUG() << "AcquisitionThread::nextRecord> Start of loop at record " 
		       << tmpSequence->record << " loop "
		       << tmpSequence->loop << " loopNumber " 
		       << tmpSequence->loopNumber
		       << " instrument " << tmpSequence->instrumentType << ":" 
		       << tmpSequence->instrumentName;

	  if ( tmpSequence->remainingLoops > 1 ) {
	    record = tmpSequence->record;
	    tmpSequence->remainingLoops--;
	    // Set new group name
            if (sequence->inc_group >= 10)
	      sequence->group = sequence->datagroup + "_" + QString::number(sequence->inc_group);
            else
              sequence->group = sequence->datagroup + "_0" + QString::number(sequence->inc_group);
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
    if (sequence->inc_group >= 10)
       sequence->group = sequence->datagroup + "_" + QString::number(sequence->inc_group);
    else
       sequence->group = sequence->datagroup + "_0" + QString::number(sequence->inc_group);
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
    if (tmpSequence->loop == AcquisitionSequence::IS_LOOP && 
        tmpSequence->loopends == AcquisitionSequence::LOOP_END) {
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
		  << i << " loop "
		  << tmpSequence->loop << " loopends " << tmpSequence->loopends
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
      if (tmpSequence->datagroup != "") {
	QLOG_DEBUG() << "AcquisitionThread::saveData> Found grand parent reference loop at record " 
		     << j << " loop "
		     << tmpSequence->loop << " loopNumber " << tmpSequence->loopNumber
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
    QLOG_DEBUG() << "AcquisitionThread::saveData> Open new group " << sequence->datagroup 
		 << " in " << sequence->grpname << "(" << sequence->tmpgrp << ")";
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
  else if (sequence->instrumentType == "DAC" && dacsuccess == true) {
    status = H5LTset_attribute_float(sequence->refgrp, sequence->grpname.toStdString().c_str(), 
				     sequence->dataname.toStdString().c_str(), 
				     &(sequence->dacValue),1);
  }
  // Save COMEDI data in the group
  else if ( sequence->instrumentType == "COMEDICOUNTER" && comedicountersuccess == true) {
    QLOG_INFO() << "AcquisitionThread::saveData> save COMEDI COUNTER data";
    status = H5LTset_attribute_double(sequence->refgrp, sequence->grpname.toStdString().c_str(),
                                     sequence->dataname.toStdString().c_str(),
                                     &(sequence->comediData),1);
  }
  else if ( sequence->instrumentType == "COMEDIDAC" && comedidacsuccess == true) {
    QLOG_INFO() << "AcquisitionThread::saveData> save COMEDI DAC data";
    status = H5LTset_attribute_double(sequence->refgrp, sequence->grpname.toStdString().c_str(),
                                     sequence->dataname.toStdString().c_str(),
                                     &(sequence->comediData),1);
  }
  // Save SLM data in the group
  else if (sequence->instrumentType == "SLM" && slmsuccess == true) {
    status = H5LTset_attribute_int(sequence->refgrp, sequence->grpname.toStdString().c_str(), 
				     sequence->dataname.toStdString().c_str(), 
				     &(sequence->imgnum),1);
    status = H5IMmake_image_8bit(sequence->grp,sequence->dataname.toStdString().c_str(),
				 sequence->imageWidth,sequence->imageHeight,sequence->getImageFromFile());
  }
  // Save CAMERA data in the group
  else if (sequence->instrumentType == "CAMERA" && imagesuccess == true) {
    hsize_t dset_dims[2];
    dset_dims[0] = sequence->imageHeight;
    dset_dims[1] = sequence->imageWidth;
    /* create and write an double type dataset named "dset" */
    QLOG_INFO () << " Save image data " << sequence->dataname
                 << " width " << sequence->imageWidth
                 << " height " << sequence->imageHeight;
   if ( sequence->settings == "SNAPSHOT" ) 
       status = H5IMmake_image_8bit(sequence->grp,sequence->dataname.toStdString().c_str(),
				 sequence->imageWidth,sequence->imageHeight,sequence->getImage());
   else if ( sequence->settings == "SNAPSHOT32" ) 
       status = H5LTmake_dataset_int(sequence->grp,sequence->dataname.toStdString().c_str(),
                                 2,dset_dims,sequence->getImage32());
   
   status = H5LTset_attribute_int(sequence->grp, sequence->dataname.toStdString().c_str(), 
				   "min", 
				   &sequence->imageMin,1);
   status = H5LTset_attribute_int(sequence->grp, sequence->dataname.toStdString().c_str(), 
				   "max", 
				   &sequence->imageMax,1);
  }
  else if ( sequence->instrumentType == "FILE"  && filesuccess == true ) {
      FileParser *fparser = sequence->fileParser; 
      if ( fparser->getType() == "ARRAY" ) {
         hsize_t dset_dims[2];
         dset_dims[0] = fparser->getArrayHeight();
         dset_dims[1] = fparser->getArrayWidth();
         status = H5LTmake_dataset_double(sequence->grp,sequence->dataname.toStdString().c_str(),
                                 2,dset_dims,fparser->getArray());
         status = H5LTset_attribute_string(sequence->grp, sequence->dataname.toStdString().c_str(),
                                   "File name",fparser->getParsedFileName().toStdString().c_str());
      }
      if ( fparser->getType() == "LIST" ) {
         QLOG_INFO () << "AcquisitionThread::saveData> treating LIST type";
         hsize_t dset_dims[1];
         dset_dims[0] = 1;
         QVector<QString> nameList = fparser->getNameList();
         QVector<double> dataList = fparser->getDataList();
         status = H5LTmake_dataset_int(sequence->grp,sequence->dataname.toStdString().c_str(),
                                 1,dset_dims,0);
         for ( int i = 0 ; i < dataList.size(); i++ ) {
            QLOG_INFO () << "AcquisitionThread::saveData> " << nameList.at(i) << " = " << dataList.at(i);
            status = H5LTset_attribute_double(sequence->grp, sequence->dataname.toStdString().c_str(),
                                   nameList.at(i).toStdString().c_str(),&dataList.at(i),1);
         }
      }

  }
  else if (sequence->instrumentType == "TREATMENT") {
    QLOG_DEBUG() << "AcquisitionThread::saveData> sequence->treatment " << sequence->treatment;
    if (sequence->treatment != "" && sequence->avg != "" ) {
      // Check parent group and save average in grand parent group
      QLOG_DEBUG() << "AcquisitionThread::saveData> Save avergage data remainingLoops " 
		   << parentSequence->remainingLoops;
      if ( sequence->treatment == "AVG" && parentSequence->remainingLoops == 1 
	   && grandparentSequence != NULL) {
	if (sequence->instrumentRef == "CAMERA" && treatmentsuccess == true) {
	  hsize_t dset_dims[2];
	  dset_dims[0] = sequence->data_2D_FLOAT_DIM_Y;
	  dset_dims[1] = sequence->data_2D_FLOAT_DIM_X;
	  // Average data
	  for (int i = 0 ; i < sequence->data_2D_FLOAT_DIM_X * sequence->data_2D_FLOAT_DIM_Y; i++)
	    sequence->data_2D_FLOAT[i] /= parentSequence->loopNumber;
	  status = H5LTmake_dataset_float(grandparentSequence->grp,
					  sequence->dataname.toStdString().c_str(),
					  2,dset_dims,sequence->data_2D_FLOAT);
	  // reset sequence data
	  sequence->reset =true;
	}
	else if (sequence->instrumentRef == "DAC" && treatmentsuccess == true ) {
	  QLOG_DEBUG() << "AcquisitionThread::saveData> Save average DAC data in groupname " 
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
        else if ( sequence->instrumentRef == "COMEDICOUNTER" && treatmentsuccess == true ) {
          QLOG_DEBUG() << "AcquisitionThread::saveData> Save average COMEDI COUNTER data in groupname " 
                       << grandparentSequence->grpname
                       << " refgrp " << grandparentSequence->refgrp;

          // Average data
          sequence->comediData /= parentSequence->loopNumber;
          status = H5LTset_attribute_double(grandparentSequence->refgrp,
                                           grandparentSequence->grpname.toStdString().c_str(),
                                           sequence->dataname.toStdString().c_str(),
                                           &(sequence->comediData),1);
          // reset sequence data
          sequence->reset =true;
        }
        else if ( sequence->instrumentType == "COMEDIDAC" && treatmentsuccess == true ) {
          QLOG_DEBUG() << "AcquisitionThread::saveData> Save average COMEDI DAC data in groupname "
                       << grandparentSequence->grpname
                       << " refgrp " << grandparentSequence->refgrp;

          // Average data
          sequence->comediData /= parentSequence->loopNumber;
          status = H5LTset_attribute_double(grandparentSequence->refgrp,
                                           grandparentSequence->grpname.toStdString().c_str(),
                                           sequence->dataname.toStdString().c_str(),
                                           &(sequence->comediData),1);
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
