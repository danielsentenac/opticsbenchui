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
  // Connect motor db
  dbConnexion();
}
Motor::~Motor()
{
  QLOG_INFO ( ) << "Deleting Motor";
  QLOG_DEBUG () << " actuatorCom.size " << actuatorCom.size();
  QLOG_DEBUG () << " actuatorDriver.size " << actuatorDriver.size();

  float a,b,c;
  for (int i = actuatorDriver.size() - 1 ; i >= 0; i--)
    if (actuatorDriver.at(i)) actuatorDriver.at(i)->Exit(actuatorSettings.at(i).toStdString());

  for (int i = actuatorCom.size() - 1 ; i >= 0; i--)
    if (actuatorCom.at(i)) actuatorCom.at(i)->Close();

  actuatorCom.clear();
  actuatorDriver.clear();
  actuatorSettings.clear();
  description.clear();
  com.clear();
  driver.clear();
  actuator.clear();
  connectSuccess.clear();
  operationcomplete.clear();
  position.clear();

  // Close Db
  {
    QSqlDatabase db = QSqlDatabase::database(path);
    db.close();
  }
  QSqlDatabase::removeDatabase(path);
}
void Motor::setDbPath(QString _path) {
  
  // Re_Init object
  for (int i = 0 ; i < actuatorCom.size(); i++)
    if (actuatorCom.at(i)) actuatorCom.at(i)->Close();
 
  actuatorCom.clear();
  actuatorDriver.clear();
  actuatorSettings.clear();
  description.clear();
  com.clear();
  driver.clear();
  actuator.clear();
  connectSuccess.clear();
  operationcomplete.clear();
  position.clear();
  
  // Close Db
  {
    QSqlDatabase db = QSqlDatabase::database(path);
    db.close();
  }
  QSqlDatabase::removeDatabase(path);
  path = _path;
  // Connect motor db
  dbConnexion();
}

void
Motor::connectMotor(QString newactuator) {  
  
  bool driverexist = false;
  bool comexist = false;
  QSqlQuery query(QSqlDatabase::database(path));
  
  if (newactuator == "") return;
  
  //
  // Get actuator data
  //
  QString actutype, actusettings, actudescription;
  float actuposition = 0;
  query.prepare("select settings,description,position from motor_actuator where name = ?");
  query.addBindValue(newactuator);
  query.exec();
  while (query.next()) {
    actutype = query.value(0).toString();
    actusettings = query.value(0).toString();
    actudescription = query.value(1).toString();
    actuposition = query.value(2).toString().toFloat();
  }
  //
  // Check if actuator already configured
  //
  for (int i = 0 ; i < actuator.size(); i++)  {
    if ( actuator.at(i) == newactuator ) {
      QLOG_INFO () << newactuator << " already connected";
      if ( connectSuccess.at(i) == true)  {
	emit getPosition(actuposition);
	emit getDescription(actudescription);
	return;
      }
      else  {
	QLOG_DEBUG () << newactuator << " try to reconnect...";
	//
	// Free Actuator related structures
	//
	actuatorCom.remove(i);
	actuatorDriver.remove(i);
	actuatorSettings.remove(i);
	description.remove(i);
	com.remove(i);
	driver.remove(i);
	actuator.remove(i);
	connectSuccess.remove(i);
	operationcomplete.remove(i);
	position.remove(i);
	break;
      }
    }
  }
  //
  // First time actuator initialization
  //
 
  //
  // Check if driver already initialized for the new actuator
  //
  query.prepare("select type,settings,name from motor_driver where name = "
		"(select driver from motor_actuator where name = ?)");
  query.addBindValue(newactuator);
  query.exec();
  QString drvtype, drvsettings, newdriver;
  while (query.next()) {
    drvtype = query.value(0).toString();
    drvsettings = query.value(1).toString();
    newdriver = query.value(2).toString();
  }
  for (int i = 0 ; i < driver.size(); i++)  {
    if ( driver.at(i) == newdriver ) {
      QLOG_INFO () << " Assign previously created driver " << driver.at(i) 
		   << "to actuator " << newactuator;
      driver.push_back(driver.at(i));
      actuatorDriver.push_back(actuatorDriver.at(i));
      driverexist = true;
      break;
    }
  }
  if ( driverexist == false )  {
    //
    // Check if communication already exist for actuator
    //
    query.prepare("select type,address,settings,name from motor_com where name = "
		  "(select com from motor_driver where name = "
		  "(select driver from motor_actuator where name = ?))");
    query.addBindValue(newactuator);
    query.exec();
    QString comtype, comaddress, comsettings, newcom;
    while (query.next()) {
      comtype = query.value(0).toString();
      comaddress = query.value(1).toString();
      comsettings = query.value(2).toString();
      newcom = query.value(3).toString();
    }
    for (int j = 0 ; j < com.size(); j++)  {
      if (com.at(j) == newcom ) {
	QLOG_INFO () << " Assign previously created communication " << com.at(j) 
		     << "to driver ";
	com.push_back(com.at(j));
	actuatorCom.push_back(actuatorCom.at(j));
	comexist = true;
	break;
      }
    }
    if ( comexist == false )  {
      //
      // Create new com
      //
      QLOG_INFO() << " Create new Communication channel at " << comaddress;

      ACCom *accom = ACCom::Create(comtype.toStdString(),
				   comaddress.toStdString(),
				   comsettings.toStdString());
      if (!accom) {
	emit showWarning("Could not create Communication channel");
	return;
      }
      else if (accom->Open()) {
	emit showWarning("Could not open communication with device");
        delete accom;
	return;
      }
      com.push_back(newcom);
      actuatorCom.push_back(accom);
    }
    //
    // Create new driver
    //
    QLOG_INFO() << " Create new Driver with settings " << drvsettings;
    Driver *drv = Driver::Create(drvtype.toStdString(),
				 drvsettings.toStdString(),
				 actuatorCom.at(actuatorCom.size() -1 ));
    if (!drv) {
      emit showWarning("Could not create Driver");
      return;
    }
    else {
      string driverState;
      if (drv->Init(driverState)) {
	emit showWarning("Could not init driver");
        delete drv;
        return;
      }
    }
    actuatorDriver.push_back(drv);
    driver.push_back(newdriver);
  }
  //
  // Create new actuator
  //
  actuator.push_back(newactuator);
  connectSuccess.push_back(true);
  actuatorSettings.push_back(actusettings);
  position.push_back(actuposition);
  description.push_back(actudescription);
  operationcomplete.push_back(0);
  QLOG_INFO() << " Create new Actuator with settings " << actusettings;
  if (actuatorDriver.at(actuator.size() - 1)->InitActuator(actusettings.toStdString(),actuposition)) {
    emit showWarning("Could not init actuator");
    connectSuccess.replace(actuator.size() - 1, false);
    return;
  }
  emit getPosition(actuposition);
  emit getDescription(actudescription);
  return;
}
void
Motor::moveForward(QString newactuator, float motion) {
  for (int i = 0 ; i < actuator.size(); i++) {
    if (actuator.at(i) == newactuator && connectSuccess.at(i) == true) {
      operationcomplete.replace(i,0);
      actuatorDriver.at(i)->Move(actuatorSettings.at(i).toStdString(),motion,0);
      runningActuator = actuator.at(i);
      break;
    }
  }
}
void
Motor::moveBackward(QString newactuator, float motion) {
  for (int i = 0 ; i < actuator.size(); i++) {
    if (actuator.at(i) == newactuator && connectSuccess.at(i) == true) {
      operationcomplete.replace(i,0);
      motion = -motion;
      actuatorDriver.at(i)->Move(actuatorSettings.at(i).toStdString(),motion,0);
      runningActuator = actuator.at(i);
      break;
    }
  }
}
void
Motor::moveAbsolute(QString newactuator, float motion) {
  for (int i = 0 ; i < actuator.size(); i++) {
    if (actuator.at(i) == newactuator && connectSuccess.at(i) == true) {
      operationcomplete.replace(i,0);
      actuatorDriver.at(i)->MoveAbs(actuatorSettings.at(i).toStdString(),motion,0);
      runningActuator = actuator.at(i);
      break;
    } 
  }
}
void
Motor::stopMotor(QString newactuator) {
  for (int i = 0 ; i < actuator.size(); i++) {
    if (actuator.at(i) == newactuator && connectSuccess.at(i) == true) {
      operationcomplete.replace(i,0);
      actuatorDriver.at(i)->Stop(actuatorSettings.at(i).toStdString());
      runningActuator = actuator.at(i);
      break;
    } 
  }
}
float
Motor::getPosition(QString newactuator) {
  float actuposition = 0;
  for (int i = 0 ; i < actuator.size(); i++) {
    if (actuator.at(i) == newactuator ) {
      actuposition = position.at(i);
      break;
    }
  }
  return actuposition;
}
int 
Motor::getOperationComplete(QString newactuator) {
  int status = 1;
  for (int i = 0 ; i < actuator.size(); i++) {
    if (actuator.at(i) == newactuator ) {
      status = operationcomplete.at(i);
      break;
    }
  }
  return status;
}
void
Motor::operationComplete() {
  
  for (int i = 0 ; i < actuator.size(); i++) {
    if (actuator.at(i) == runningActuator ) {
      string stateData;
      DriverDefinition::ADLimitSwitch limitSwitch;
      iscompleting = false;
      if (connectSuccess.at(i) == true && iscompleting == false) {
	int success = 0;
	QString positionQString;
	float curpos;
	iscompleting = true;
	success = actuatorDriver.at(i)->OperationComplete(stateData,
							  actuatorSettings.at(i).toStdString(),
							  limitSwitch);
	actuatorDriver.at(i)->GetPos(actuatorSettings.at(i).toStdString(),curpos);
	position.replace(i, curpos);
	positionQString.setNum (position.at(i), 'f',3);
	QLOG_DEBUG ( ) << "OperationComplete " << success << " position " << position.at(i);
	emit getPosition(position.at(i));
	if (success > 0 ) {
          // Save last position in Db
          actuatorDriver.at(i)->GetPos(actuatorSettings.at(i).toStdString(),curpos);
          position.replace(i, curpos);
          positionQString.setNum (position.at(i), 'f',3);
          QLOG_DEBUG ( ) << "OperationComplete " << success << " position " << position.at(i);
          emit getPosition(position.at(i));
	  QSqlQuery query(QSqlDatabase::database(path));
	  query.prepare("update motor_actuator set position = ? "
			"where name = ?");
	  query.addBindValue(positionQString);
	  query.addBindValue(actuator.at(i));
	  query.exec();
	  operationcomplete.replace(i, success);
          QLOG_DEBUG ( ) << "OperationComplete " << success << " position " << position.at(i);
	  emit stopTimer();
	}
	iscompleting = false;
      }
      break;
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
