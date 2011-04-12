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
  void setFile(QString _filename, int _filenumber);

  QVector<bool> isopencamerawindow;

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
  void getTreatmentStatus(bool treatmentsuccess); 
  void getAcquiring(int record);
  void getFilenumber(int number);

  private slots:
  void setImageFromCamera(uchar*, int,int);

 protected:
  virtual void run();

 private:

  QVector<Camera*> cameraList;
  int    record;
  Motor  *motor;
  Dac    *dac;
  bool   dacsuccess;
  bool   imagesuccess;
  bool   treatmentsuccess;
  uchar* imageBuffer;
  int    imageWidth;
  int    imageHeight;
  
  bool suspend;

  // Data File
  hid_t   file_id;
  QVector<hid_t> ids;
  herr_t  status;
  QString filename;
  int     filenumber;
 
};

#endif // ACQUISITIONTHREAD_H
