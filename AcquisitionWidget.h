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
  AcquisitionWidget(QString _appDirPath = 0);
  ~AcquisitionWidget();
 
  void setCamera(QVector<Camera*> _cameraList);
  void setMotor(Motor* _motor);
  void setSuperK(SuperK* _superk);
  void setDac(Dac* _dac);
  void setComediCounter(Comedi* _comedi);
  void setComediDac(Comedi* _comedi);
  void setRaspiDac(Raspi* _raspi);
  void setDelegates();

 signals:
  void showWarning(QString message);
  
  public slots:
  void update();
  void reload();
  void remove();
  void run();
  void stop();
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
  void setDbPath(QString _path);
  void setAcqFile(QString _acqfile);
  void showAcquisitionWarning(QString);
  void splashScreen(QString imagepath, int screen_x, int screen_y);
 
 private:
  QVector<Camera*> cameraList;
  Motor  *motor;
  SuperK *superk;
  Dac    *dac;
  Comedi *comedicounter, *comedidac;
  Raspi *raspidac;
  QLabel *splashLabel;

  QString   appDirPath, path;
  QString         acqfile;
  QString        filenumber;
  QLabel         *statusLabel;
  QLabel         *acquisitiontitle;
  QSqlTableModel *acquisitiontable;
  QTableView     *acquisitionview;
  int             acquisitionrow;
  QPushButton    *reloadButton;
  QPushButton    *updateButton;
  QPushButton    *removeButton;
  QPushButton    *runButton;
  QPushButton    *stopButton;
  

  QGridLayout    *gridlayout;

  void InitConfig();
  void dbConnexion();
  QSqlDatabase connectDb(QString path) {
     QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",path);
     QLOG_INFO ( ) << "ComediDac::dbConnexion> Db path : " << path;
     db.setDatabaseName(path);
     if ( !db.open() ) {
       QLOG_WARN ( ) << db.lastError().text();
     }
     return db;
  }

  AcquisitionThread *acquisition;
  QVector<AcquisitionSequence*> sequenceList;
  int cur_record;

};
#endif
