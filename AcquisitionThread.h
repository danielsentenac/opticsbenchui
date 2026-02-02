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
#include "SuperK.h"
#include "Dac.h"
#include "Comedi.h"
#include "Raspi.h"
#include "QsLog.h"


class AcquisitionThread : public QThread
{
  Q_OBJECT

public:
  explicit AcquisitionThread(QObject* parent = nullptr);
  ~AcquisitionThread() override;

  void setCamera(const QVector<Camera*>& cameraList);
  void setMotor(Motor* motor);
  void setSuperK(SuperK* superk);
  void setDac(Dac* dac);
  void setComediCounter(Comedi* comedi);
  void setComediDac(Comedi* comedi);
  void setRaspiDac(Raspi* raspi);
  void setFile(const QString& filename, int filenumber);
  void setSequenceList(const QVector<AcquisitionSequence*>& sequenceList);
  void setIsOpenCameraWindow(const QVector<bool>& isOpenCameraWindow);
  void wakeSplashScreen();
  void stop();

signals:
  void getPosition(QString positionQString);
  void getSuperKData(QString dataStr);
  void getPower(int power);
  void getNd(int nd);
  void getSwp(int swp);
  void getLwp(int lwp);
  void getCameraStatus(bool imagesuccess);
  void getDacStatus(bool dacsuccess);
  void getMotorStatus(bool motorsuccess);
  void getSuperKStatus(bool superksuccess);
  void getTreatmentStatus(bool treatmentsuccess); 
  void getAcquiring(int record);
  void getFilenumber(int number);
  void showWarning(QString message);
  void splashScreen(QString imagepath, int screen_x, int screen_y);	

protected:
  void run() override;

private:
  void execute(AcquisitionSequence *sequence);
  void nextRecord(AcquisitionSequence *sequence, int cur_record);
  void saveData(AcquisitionSequence *sequence, int cur_record);
  void setImageFromCamera(uchar*, int, int, int);
  bool isSnakeReverseForRecord(int cur_record) const;

  QVector<Camera*> cameraList;
  QVector<AcquisitionSequence*> sequenceList;
  QVector<bool> isopencamerawindow;
  QMutex* mutex;
  QWaitCondition* splashScreenOk;

  int record;
  Motor* motor;
  SuperK* superk;
  Dac* dac;
  Comedi* comedicounter;
  Comedi* comedidac;
  Raspi* raspidac;
  bool dacsuccess;
  bool comedicountersuccess;
  bool comedidacsuccess;
  bool raspidacsuccess;
  bool slmsuccess;
  bool imagesuccess;
  bool filesuccess;
  bool treatmentsuccess;
  bool suspend;

  // Data File
  hid_t file_id;
  QVector<hid_t> ids;
  herr_t status;
  QString filename;
  int filenumber;
 
};

#endif // ACQUISITIONTHREAD_H
