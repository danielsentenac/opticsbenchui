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

#ifndef MOTOR_H
#define MOTOR_H

#include "ACCom.h"
#include "Driver.h"

#include <QtSql>
#include <QtGui>
#include "QsLog.h"

class Motor : public QObject
{
  Q_OBJECT
    
    public:
  Motor(QObject* parent = 0, QString _appDirPath = 0);
  ~Motor();

  void  connectMotor(QString actuator);
  void  moveForward(float motion);
  void  moveBackward(float motion);
  void  moveAbsolute(float motion);
  void  stopMotor();
  void setDbPath(QString _path);
   // parameters
  bool      connectSuccess;
  int       operationcomplete;
  float     position;
  QString   path;

 signals:
  void getPosition(float position);
  void getDescription(QString description);
  void showWarning(QString message);
  void stopTimer();

 public slots:
  void operationComplete();
  

 private:

  // function : create connexion to the database
  void dbConnexion();
  

  ACCom     *actuatorCom;
  Driver    *actuatorDriver;
  QString   actuatorSettings;
  QString   description;
  QString   com;
  QString   driver;
  QString   actuator;

  bool iscompleting;
 
};
#endif
