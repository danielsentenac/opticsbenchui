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

#include "SuperK.h"

SuperK::SuperK(QObject* parent, QString _appDirPath)
  : QObject(parent)
{
  path = _appDirPath;
  // Connect superk db
  dbConnexion();
}

SuperK::~SuperK()
{
  QLOG_DEBUG ( ) << "Deleting SuperK";
  QLOG_DEBUG () << " driverCom.size " << driverCom.size();
  QLOG_DEBUG () << " driverSuperK.size " << driverSuperK.size();

  for (int i = driverCom.size() - 1 ; i >= 0; i--)
    if (driverCom.at(i)) driverCom.at(i)->Close();

  driverCom.clear();
  driverSuperK.clear();
  driverSettings.clear();
  description.clear();
  driver.clear();
  connectSuccess.clear();
  operationcomplete.clear();
  data.clear();

  // Close Db
  {
    QSqlDatabase db = QSqlDatabase::database(path);
    db.close();
  }
  QSqlDatabase::removeDatabase(path);
}

void SuperK::setDbPath(QString _path) {
  
  // Re_Init object
  for (int i = 0 ; i < driverCom.size(); i++)
    if (driverCom.at(i)) driverCom.at(i)->Close();
 
  driverCom.clear();
  driverSuperK.clear();
  driverSettings.clear();
  description.clear();
  driver.clear();
  connectSuccess.clear();
  operationcomplete.clear();
  data.clear();
  
  // Close Db
  {
    QSqlDatabase db = QSqlDatabase::database(path);
    db.close();
  }
  QSqlDatabase::removeDatabase(path);
  path = _path;
  // Connect superk db
  dbConnexion();
}

void
SuperK::connectSuperK(QString newdriver) 
{  
  QSqlQuery query(QSqlDatabase::database(path));
  
  if (newdriver == "") return;
  //
  // Get driver data from Db
  //
  QString comtype, comaddress,comsettings;
  QString drvsettings, drvdescription;
  QString drvdata;
  int drvpower,drvnd,drvswp,drvlwp;
  query.prepare("select comtype,comaddress,comsettings,drvsettings,description,data from superk_driver where name = ?");
  query.addBindValue(newdriver);
  query.exec();
  while (query.next()) {
    comtype = query.value(0).toString();
    comaddress = query.value(1).toString();
    comsettings = query.value(2).toString();
    drvsettings = query.value(3).toString();
    drvdescription = query.value(4).toString();
    drvdata = query.value(5).toString();
    QStringList dataSplit = drvdata.split(" ",QString::SkipEmptyParts);
    if (dataSplit.size() == 4) {
       drvpower = dataSplit.at(0).toInt();
       drvnd = dataSplit.at(1).toInt();
       drvswp = dataSplit.at(2).toInt();
       drvlwp = dataSplit.at(3).toInt();
    }
  }
  //
  // Check if driver already configured
  //
  for (int i = 0 ; i < driver.size(); i++)  {
    if ( driver.at(i) == newdriver ) {
      QLOG_DEBUG () << newdriver << " already connected";
      if ( connectSuccess.at(i) == true)  {
	emit getPower(drvpower);
        emit getNd(drvnd);
        emit getSwp(drvswp);
        emit getLwp(drvlwp);
	emit getDescription(drvdescription);
	return;
      }
      else  {
	QLOG_DEBUG () << newdriver << " try to reconnect...";
	//
	// Free Driver related structures
	//
	driverSuperK.remove(i);
	driverSettings.remove(i);
	description.remove(i);
	driver.remove(i);
	connectSuccess.remove(i);
	operationcomplete.remove(i);
	data.remove(i);
	break;
      }
    }
  }
  //
  // New driver initialization
  //
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
  //
  // Create new driver
  //
  QLOG_INFO() << " Create new Driver with settings " << drvsettings;
  DriverSuperK *drv = new DriverSuperK(accom);
  if (!drv) {
    emit showWarning("Could not create Driver");
    return;
  }
  else {
    if (drv->Init()) {
       emit showWarning("Could not init driver");
      delete drv;
      return;
    }
    drv->getPower(drvpower);
    drv->getNd(drvnd);
    drv->getSwp(drvswp);
    drv->getLwp(drvlwp);
    drvdata = QString::number(drvpower) + " " + QString::number(drvnd) + " " +
              QString::number(drvswp) + " " + QString::number(drvlwp);
    driverCom.push_back(accom);
    driverSuperK.push_back(drv);
    driver.push_back(newdriver);
    connectSuccess.push_back(true);
    driverSettings.push_back(drvsettings);
    data.push_back(drvdata);
    description.push_back(drvdescription);
    operationcomplete.push_back(0);
  
    // Save new data in Db
    QSqlQuery query(QSqlDatabase::database(path));
    query.prepare("update superk_driver set data = ? "
                        "where name = ?");
    query.addBindValue(drvdata);
    query.addBindValue(newdriver);
    query.exec();

    emit getPower(drvpower);
    emit getNd(drvnd);
    emit getSwp(drvswp);
    emit getLwp(drvlwp);

    emit getDescription(drvdescription);
    return;
  }
}
void
SuperK::setPower(QString newdriver, int power)
{
  for (int i = 0 ; i < driver.size(); i++) {
    if (driver.at(i) == newdriver && connectSuccess.at(i) == true) {
      operationcomplete.replace(i,0);
      driverSuperK.at(i)->setPower(power);
      runningDriver = driver.at(i);
      break;
    }
  } 
} 
int
SuperK::getPower(QString newdriver)
{
  int drvpower;
  for (int i = 0 ; i < driver.size(); i++) {
    if (driver.at(i) == newdriver ) { 
      QStringList dataSplit = data.at(i).split(" ");
      drvpower = dataSplit.at(0).toInt();
      break; 
    } 
  } 
  return drvpower;
}

void
SuperK::setNd(QString newdriver, int nd)
{
  for (int i = 0 ; i < driver.size(); i++) {
    if (driver.at(i) == newdriver && connectSuccess.at(i) == true) {
      operationcomplete.replace(i,0);
      driverSuperK.at(i)->setNd(nd);
      runningDriver = driver.at(i);
      break;
    }
  }
}
int
SuperK::getNd(QString newdriver)
{
  int drvnd;
  for (int i = 0 ; i < driver.size(); i++) {
    if (driver.at(i) == newdriver ) {
      QStringList dataSplit = data.at(i).split(" ");
      drvnd = dataSplit.at(1).toInt();
      break;
    }
  }
  return drvnd;
}
void
SuperK::setSwp(QString newdriver, int swp) 
{
  for (int i = 0 ; i < driver.size(); i++) {
    if (driver.at(i) == newdriver && connectSuccess.at(i) == true) {
      operationcomplete.replace(i,0);
      if ( swp < 11000 && swp > 4500 ) 
        driverSuperK.at(i)->setSwp(swp);
      runningDriver = driver.at(i);
      break;
    } 
  }
}
int
SuperK::getSwp(QString newdriver) 
{
  int drvswp;
  for (int i = 0 ; i < driver.size(); i++) {
    if (driver.at(i) == newdriver ) {
      QStringList dataSplit = data.at(i).split(" ");
      drvswp = dataSplit.at(2).toInt();
      break;
    }
  }
  return drvswp;
}
void
SuperK::setLwp(QString newdriver, int lwp)
{
  for (int i = 0 ; i < driver.size(); i++) {
    if (driver.at(i) == newdriver && connectSuccess.at(i) == true) {
      operationcomplete.replace(i,0);
      if ( lwp < 11000 && lwp > 4500 )
        driverSuperK.at(i)->setLwp(lwp);
      runningDriver = driver.at(i);
      break;
    }
  }
}
int
SuperK::getLwp(QString newdriver)
{
  int drvlwp;
  for (int i = 0 ; i < driver.size(); i++) {
    if (driver.at(i) == newdriver ) {
      QStringList dataSplit = data.at(i).split(" ");
      drvlwp = dataSplit.at(3).toInt();
      break;
    }
  }
  return drvlwp;
}
int 
SuperK::getOperationComplete(QString newdriver) 
{
  int status = 1;
  for (int i = 0 ; i < driver.size(); i++) {
    if (driver.at(i) == newdriver ) {
      status = operationcomplete.at(i);
      break;
    }
  }
  return status;
}
void
SuperK::operationComplete() 
{
  QLOG_DEBUG() << "SuperK::operationComplete runningDriver:" << runningDriver;
  for (int i = 0 ; i < driver.size(); i++) {
    if (driver.at(i) == runningDriver ) {
      iscompleting = false;
      if (connectSuccess.at(i) == true && iscompleting == false) {
	int success = 0;
	int power,nd,swp,lwp;
	iscompleting = true;
	success = driverSuperK.at(i)->operationComplete();
        QLOG_DEBUG() << "SuperK::operationComplete success:" << success;
        driverSuperK.at(i)->getPower(power);
        driverSuperK.at(i)->getNd(nd);
	driverSuperK.at(i)->getSwp(swp);
        driverSuperK.at(i)->getLwp(lwp);
        QString dataStr = QString::number(power) + " " + QString::number(nd) + " " + 
                          QString::number(swp) + " " + QString::number(lwp);
	data.replace(i, dataStr);
	emit getPower(power);
        emit getNd(nd);
        emit getSwp(swp);
        emit getLwp(lwp);
	if (success > 0 ) {
	  // Save last data in Db
	  QSqlQuery query(QSqlDatabase::database(path));

	  query.prepare("update superk_driver set data = ? "
			"where name = ?");
	  query.addBindValue(dataStr);
	  query.addBindValue(driver.at(i));
	  query.exec();
	  operationcomplete.replace(i, success);
	  emit stopTimer();
	}
	iscompleting = false;
      }
      break;
    }
  }
}

void
SuperK::dbConnexion() {

  path.append(QDir::separator()).append("superk.db3");
  path = QDir::toNativeSeparators(path);
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",path);
  db.setDatabaseName(path);
  if ( !db.open() ) {
    emit showWarning(db.lastError().text());
  }
  //
  // Create superk table
  //
  QSqlQuery query(QSqlDatabase::database(path));
  query.exec("create table superk_driver "
     "(name varchar(128) not null primary key, "
     "comtype varchar(30), "
     "comaddress varchar(30), "
     "comsettings varchar(255), "
     "drvsettings varchar(255), "
     "description varchar(255), "
     "data varchar(255))");
}

