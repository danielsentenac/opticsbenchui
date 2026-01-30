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

#ifndef ACQUISITIONWIDGET_H
#define ACQUISITIONWIDGET_H

#include <QtSql>
#include <QtWidgets>


#include "QsLog.h"
#include "AcquisitionThread.h"
#include "ComboBoxDelegate.h"

class AcquisitionWidget : public QWidget
{
  Q_OBJECT
  
public:
  explicit AcquisitionWidget(QString appDirPath = QString());
  ~AcquisitionWidget() override;
  
  void setCamera(QVector<Camera*> cameraList);
  void setMotor(Motor* motor);
  void setSuperK(SuperK* superk);
  void setDac(Dac* dac);
  void setComediCounter(Comedi* comedi);
  void setComediDac(Comedi* comedi);
  void setRaspiDac(Raspi* raspi);
  void setDelegates();
  void setDbPath(QString path);
  void setAcqFile(QString acqfile);

signals:
  void showWarning(QString message);
  
public slots:
  void update();
  void reload();
  void remove();
  void run();
  void stop();
  void splashScreen(QString imagepath, int screen_x, int screen_y);
  void getPosition(QString positionQString);
  void getSuperKData(QString dataStr);
  void getCameraStatus(bool imagesuccess);
  void getDacStatus(bool dacsuccess);
  void getMotorStatus(bool motorsuccess);
  void getSuperKStatus(bool superksuccess);
  void getTreatmentStatus(bool treatmentstatus);
  void getAcquiring(int record);
  void getFilenumber(int number);
  void isopenCameraWindow(QVector<bool> isopencamerawindow);
  void showAcquisitionWarning(QString message);
 
private:
  void InitConfig();
  void dbConnexion();
  void setupAcquisitionTable();
  void updateStatusForRecord(const QString& status, int record);
  QString statusFromSuccess(bool success) const;

  QString appDirPath;
  QString path;
  QString acqfile;
  QString filenumber;
  int acquisitionrow;
  int cur_record;

  QVector<Camera*> cameraList;
  Motor* motor;
  SuperK* superk;
  Dac* dac;
  Comedi* comedicounter;
  Comedi* comedidac;
  Raspi* raspidac;

  QLabel* splashLabel;
  QLabel* acquisitiontitle;
  QSqlTableModel* acquisitiontable;
  QTableView* acquisitionview;
  QPushButton* reloadButton;
  QPushButton* updateButton;
  QPushButton* removeButton;
  QPushButton* runButton;
  QPushButton* stopButton;
  QGridLayout* gridlayout;

  AcquisitionThread* acquisition;
  QVector<AcquisitionSequence*> sequenceList;

};
#endif
