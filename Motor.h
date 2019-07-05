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
#include <QtWidgets>
#include "QsLog.h"

class Motor : public QObject
{
  Q_OBJECT
    
    public:
  Motor(QObject* parent = 0, QString _appDirPath = 0);
  ~Motor();

  void  connectMotor(QString newactuator);
  void  moveForward(QString newactuator,float motion);
  void  moveBackward(QString newactuator,float motion);
  void  moveAbsolute(QString newactuator,float motion);
  void  stopMotor(QString newactuator);
  void  setDbPath(QString _path);
  int   getOperationComplete(QString newactuator);
  float getPosition(QString newactuator);
  
  // parameters
  QVector<int> operationcomplete;
  QString path;

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
  

  QVector<ACCom*>   actuatorCom;
  QVector<Driver*>  actuatorDriver;
  QVector<QString>  actuatorSettings;
  QVector<QString>  description;
  QVector<QString>  com;
  QVector<QString>  driver;
  QVector<QString>  actuator;
  bool iscompleting;
  QVector<bool> connectSuccess;
  QVector<float> position;
  QString runningActuator;
};
#endif
