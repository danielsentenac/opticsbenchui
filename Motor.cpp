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

#include "Motor.h"

Motor::Motor(QObject* parent, QString _appDirPath)
  : QObject(parent)
 
{
  path = _appDirPath;
  actuatorCom = NULL;
  actuatorDriver = NULL;
  position = 0;
  connectSuccess = false;
  // Connect motor db
  dbConnexion();
}
Motor::~Motor()
{
  QLOG_DEBUG ( ) << "Deleting Motor";
  if (actuatorCom) 
    actuatorCom->Close();
  if (actuatorDriver) {
    delete actuatorDriver;
  }
  // Close Db
  {
    QSqlDatabase db = QSqlDatabase::database(path);
    db.close();
  }
  QSqlDatabase::removeDatabase(path);
}
void Motor::setDbPath(QString _path) {
  // Close Db
  {
    QSqlDatabase db = QSqlDatabase::database(path);
    db.close();
  }
  QSqlDatabase::removeDatabase(path);
  
  if (actuatorCom) 
    actuatorCom->Close();
  if (actuatorDriver) {
    delete actuatorDriver;
  }
  
  // Re_Init object
  path = _path;
  actuatorCom = NULL;
  actuatorDriver = NULL;
  position = 0;
  connectSuccess = false;
  // Connect motor db
  dbConnexion();

}
void
Motor::connectMotor(QString _actuator) {  
  
  if (_actuator == "") return;
  QSqlQuery query(QSqlDatabase::database(path));
  QString type,address,settings;
  
  position = 0;

  actuator = _actuator;

  //
  // Create communication channel
  //
  query.prepare("select type,address,settings,name from motor_com where name = "
		"(select com from motor_driver where name = "
		"(select driver from motor_actuator where name = ?))");
  query.addBindValue(actuator);
  query.exec();
  QString newcom;
  while (query.next()) {
    type = query.value(0).toString();
    address = query.value(1).toString();
    settings = query.value(2).toString();
    newcom = query.value(3).toString();
  }
  //
  // Check if actuatorCom already loaded
  //
  if (com != newcom || connectSuccess == false) {
    com = newcom;
    if (actuatorCom) {
      actuatorCom->Close();
      delete actuatorCom;
    }
    actuatorCom = ACCom::Create(type.toStdString(),address.toStdString(),settings.toStdString());
    if (!actuatorCom) {
      emit showWarning("Could not create Communication channel");
      return;
    }
    //
    // Init Actuator
    //
    if (actuatorCom->Open()) {
      emit showWarning("Could not open communication with device");
      return;
    }
  }
  //
  // Create driver
  //
  query.prepare("select type,settings,name from motor_driver where name = "
		"(select driver from motor_actuator where name = ?)");
  query.addBindValue(actuator);
  query.exec();
  QString newdriver;
  while (query.next()) {
    type = query.value(0).toString();
    settings = query.value(1).toString();
    newdriver = query.value(2).toString();
  }
  if (driver != newdriver || connectSuccess == false) {
    driver = newdriver;
    if (actuatorDriver) {
      delete actuatorDriver;
    }
    actuatorDriver = Driver::Create(type.toStdString(),settings.toStdString(),actuatorCom);
    if (!actuatorDriver) {
      emit showWarning("Could not create Driver");
      return;
    }
    string driverState;
    if (actuatorDriver->Init(driverState)) {
      emit showWarning("Could not init driver");
      return;
    }
  }
  //
  // Create actuator data
  //
  query.prepare("select settings,description,position from motor_actuator where name = ?");
  query.addBindValue(actuator);
  query.exec();
  while (query.next()) {
    type = query.value(0).toString();
    actuatorSettings = query.value(0).toString();
    description = query.value(1).toString();
    position = query.value(2).toString().toFloat();
  }
  if (actuatorDriver->InitActuator(actuatorSettings.toStdString(),position)) {
    emit showWarning("Could not init actuator");
    return;
  }
  emit getPosition(position);
  emit getDescription(description);
  connectSuccess = true;
}
void
Motor::moveForward(float motion) {
  if (connectSuccess == true) {
    actuatorDriver->Move(actuatorSettings.toStdString(),motion,0);
  }
}
void
Motor::moveBackward(float motion) {
 if (connectSuccess == true) {
    motion = -motion;
    actuatorDriver->Move(actuatorSettings.toStdString(),motion,0);
 }
 
}
void
Motor::moveAbsolute(float motion) {
  if (connectSuccess == true) {
    actuatorDriver->MoveAbs(actuatorSettings.toStdString(),motion,0);
  }
}
void
Motor::stopMotor() {
  if (connectSuccess == true) {
    actuatorDriver->Stop(actuatorSettings.toStdString());
  }
}
void
Motor::operationComplete() {
  string stateData;
  DriverDefinition::ADLimitSwitch limitSwitch;

  if (connectSuccess == true) {
    operationcomplete = actuatorDriver->OperationComplete(stateData,actuatorSettings.toStdString(),
							  limitSwitch);
    
    QString positionQString;
    actuatorDriver->GetPos(actuatorSettings.toStdString(),position);
    positionQString.setNum (position, 'f',3);
    QLOG_DEBUG ( ) << "Operationcomplete " << operationcomplete << " position " << position;
    emit getPosition(position);
    if (operationcomplete > 0 ) {
      // Save last position in Db
      QSqlQuery query(QSqlDatabase::database(path));
      query.prepare("update motor_actuator set position = ? "
		    "where name = ?");
      query.addBindValue(positionQString);
      query.addBindValue(actuator);
      query.exec();
      emit stopTimer();
    }
  }
}
// function : create connexion to the database
void Motor::dbConnexion() {
  
  path.append(QDir::separator()).append("motor.db3");
  path = QDir::toNativeSeparators(path);
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",path);
  db.setDatabaseName(path);  
  if ( !db.open() ) {
    emit showWarning(db.lastError().text());
    return;
  }
  // Create motor tables

  QSqlQuery query(QSqlDatabase::database(path));
  query.exec("create table motor_com "
	     "(name varchar(128) not null primary key, "
	     "type varchar(30), "
	     "address varchar(30), "
	     "settings varchar(255))");
  query.exec("create table motor_driver "
	     "(name varchar(128) not null primary key, "
	     "com  varchar(128), "
	     "type varchar(30), "
	     "settings varchar(255))");
  query.exec("create table  motor_actuator "
	     "(name varchar(128) not null primary key, "
	     "driver  varchar(128), "
	     "description varchar(255), "
	     "settings varchar(255), "
	     "position  float)");
 
}
