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

#ifndef SUPERK_H
#define SUPERK_H

#include "DriverSuperK.h"

#include <QtSql>
#include <QtGui>
#include "QsLog.h"

class SuperK : public QObject
{
  Q_OBJECT
    
    public:
  SuperK(QObject* parent = 0, QString _appDirPath = 0);
  ~SuperK();

  void  connectSuperK(QString newdriver);
  void  setPower(QString newdriver, int power);
  void  setNd(QString newdriver, int nd);
  void  setSwp(QString newdriver, int swp);
  void  setLwp(QString newdriver, int lwp);
  void  setDbPath(QString _path);
  int   getOperationComplete(QString newdriver);
  int   getPower(QString newdriver);
  int   getNd(QString newdriver);
  int   getSwp(QString newdriver);
  int   getLwp(QString newdriver);

  // parameters
  QVector<int> operationcomplete;
  QString path;

 signals:
  void getPower(int power);
  void getNd(int nd);
  void getSwp(int swp);
  void getLwp(int lwp);
  void getDescription(QString description);
  void showWarning(QString message);
  void stopTimer();

 public slots:
  void operationComplete();
  

 private:

  // function : create connexion to the database
  void dbConnexion();
  QVector<ACCom*>   driverCom;
  QVector<DriverSuperK*>  driverSuperK;
  QVector<QString>  driverSettings;
  QVector<QString>  description;
  QVector<QString>  driver;
  QVector<QString>  data;
  bool iscompleting;
  QVector<bool> connectSuccess;
  QString runningDriver;
};
#endif
