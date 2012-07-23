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

#ifndef ACQUISITIONTHREAD_H
#define ACQUISITIONTHREAD_H

#include "AcquisitionSequence.h"
#include "Camera.h"
#include "Motor.h"
#include "Dac.h"
#include "Comedi.h"
#include "QsLog.h"


class AcquisitionThread : public QThread
{
  Q_OBJECT
    
    public:
  AcquisitionThread(QObject* parent = 0);
  ~AcquisitionThread();

  void setCamera(QVector<Camera*> _cameraList);
  void setMotor(Motor* _motor);
  void setDac(Dac* _dac);
  void setComedi(Comedi* _comedi);
  void setFile(QString _filename, int _filenumber);

  QVector<bool> isopencamerawindow;
  QMutex *mutex;
  QWaitCondition *splashScreenOk;

  QVector<AcquisitionSequence*> sequenceList;
  void execute(AcquisitionSequence *sequence);
  void nextRecord(AcquisitionSequence *sequence, int cur_record);
  void saveData(AcquisitionSequence *sequence, int cur_record);
  void setSequenceList(QVector<AcquisitionSequence*> _sequenceList);
  void stop();
  

 signals:
  void getPosition(QString positionQString);
  void getCameraStatus(bool imagesuccess);
  void getDacStatus(bool dacsuccess);
  void getMotorStatus(bool motorsuccess);
  void getTreatmentStatus(bool treatmentsuccess); 
  void getAcquiring(int record);
  void getFilenumber(int number);
  void showWarning(QString message);
  void splashScreen(QString imagepath, int screen_x, int screen_y);	

 protected:
  virtual void run();

 private:
  void setImageFromCamera(uchar*, int, int, int);

  QVector<Camera*> cameraList;
  int    record;
  Motor  *motor;
  Dac    *dac;
  Comedi *comedi;
  bool   dacsuccess;
  bool   comedisuccess;
  bool   slmsuccess;
  bool   imagesuccess;
  bool   filesuccess;
  bool   treatmentsuccess;  
  bool   suspend;

  // Data File
  hid_t   file_id;
  QVector<hid_t> ids;
  herr_t  status;
  QString filename;
  int     filenumber;
 
};

#endif // ACQUISITIONTHREAD_H
