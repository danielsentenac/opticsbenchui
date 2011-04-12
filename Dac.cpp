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

#include "Dac.h"

Dac::Dac(QObject *parent, QString _appDirPath)
  : QObject(parent)
{ 
  path = _appDirPath;
  fd = 0;
  dac = "";
  connectSuccess = false;
  // Connect dac db
  dbConnexion();
  dacvalues = new QVector<float>();
}
Dac::~Dac()
{
  QLOG_DEBUG ( ) <<"deleting Dac";
  if (fd)
    DRV_DeviceClose(&fd);
  delete dacvalues;
  {
    QSqlDatabase db = QSqlDatabase::database(path);
    db.close();
  }
  QSqlDatabase::removeDatabase(path);
  
}
void Dac::setDbPath(QString _path){
  // Close Db
  {
    QSqlDatabase db = QSqlDatabase::database(path);
    db.close();
  }
  QSqlDatabase::removeDatabase(path);
  
  // Re-Init object
  path = _path;
  if (fd)
    DRV_DeviceClose(&fd);
  delete dacvalues;
  
  fd = 0;
  dac = "";
  connectSuccess = false;
  dbConnexion();
  dacvalues = new QVector<float>();
}
bool
Dac::connectDac(QString newdac) {
  
  if (newdac == "") return false;
  
  char err_msg[100];
  QSqlDatabase db = QSqlDatabase::database(path);
  QSqlQuery query(db);
  int status;
  QString description;
  QString dacvaluesString = "";
  dacSettings = "";
  //
  // Configure Dac
  //
  query.prepare("select settings,description,dacvalues from dac_settings where name = ?");
  query.addBindValue(newdac);
  query.exec();
  
  while (query.next()) {
    dacSettings = query.value(0).toString();
    description = query.value(1).toString();
    dacvaluesString = query.value(2).toString();
  }
  if (dacSettings == "") {
    emit showWarning(tr("%1 does not exist").arg(newdac));
    return false;
  }
 
  //
  // Check if dac already configured
  //
  if (dac != newdac || connectSuccess == false) {
    dac = newdac;
    QStringList settings;
    QStringList subsettings;
    settings = dacSettings.split(" ");
    subsettings = settings.at(0).split("=");
    fname = subsettings.at(1);
    subsettings = settings.at(1).split("=");
    mode  = subsettings.at(1);
    subsettings = settings.at(2).split("=");
    outputs = subsettings.at(1).toInt();
    subsettings = settings.at(3).split("=");
    max = subsettings.at(1).toFloat();
    subsettings = settings.at(4).split("=");
    min = subsettings.at(1).toFloat();
    QLOG_DEBUG ( ) << "fname " <<  fname;
    QLOG_DEBUG ( ) << "mode = "  <<   mode;
    QLOG_DEBUG ( ) << "Outputs = " << outputs;
    QLOG_DEBUG ( ) << "Max =  " << max;
    QLOG_DEBUG ( ) << "Min =  " << min;
   
    if (fd) DRV_DeviceClose(&fd);
    // Open device
    status = DRV_DeviceOpen((char*)fname.toStdString().c_str(), &fd);
   
    if (status) {
      DRV_GetErrorMessage(status, err_msg);
      emit showWarning(tr("DRV_DeviceOpen:%1").arg(err_msg));
      connectSuccess = false;
      return false;
    }
    if (mode == "VOLT") {
      for (int i = 0; i < outputs; i++) {
	memset(&config, 0, sizeof(PT_AOConfig));
	config.chan = i;
	config.RefSrc = 0;
	config.MaxValue = max;
	config.MinValue = min;
	status = DRV_AOConfig(fd, &config);
	if (status) {
	  DRV_GetErrorMessage(status, err_msg);
	  emit showWarning(tr("DRV_AOConfig:%1").arg(err_msg));
	  connectSuccess = false;
	  return false;
	}
      }
    }
    else if (mode == "CURRENT") {
      for (int i = 0; i < outputs; i++) {
	memset(&ao_chan_range, 0, sizeof(ao_chan_range));
	ao_chan_range.usGainCount = 0;
	ao_chan_range.usAOSource = 1; /* =0 internal, =1 external */
	ao_chan_range.usAOType = 1; /* =0 voltage, =1 current */
	ao_chan_range.usChan = i;
	ao_chan_range.fAOMax = max;
	ao_chan_range.fAOMin = min;
	status = DRV_DeviceSetProperty(fd, CFG_AoChanRange,
				       &ao_chan_range, sizeof(ao_chan_range));
	if (status) {
	  DRV_GetErrorMessage(status, err_msg);
	  emit showWarning(tr("DRV_DeviceSetProperty:%1").arg(err_msg));
	  connectSuccess = false;
	  return false;
	}
	INT32U size;
	status = DRV_DeviceGetProperty(fd, CFG_AoChanRange,&ao_chan_range,&size);
	if (status) {
	  DRV_GetErrorMessage(status, err_msg);
	  emit showWarning(tr("DRV_DeviceGetProperty:%1").arg(err_msg));
	  connectSuccess = false;
	  return false;
	}
	QLOG_DEBUG ( ) << dac << " Channel " << i << " GainCount: " <<  ao_chan_range.usGainCount;
	QLOG_DEBUG ( ) << "AOSource: " << ao_chan_range.usAOSource;
	QLOG_DEBUG ( ) << "AOType: " << ao_chan_range.usAOType;
	QLOG_DEBUG ( ) << "Chan: " << ao_chan_range.usChan;
	QLOG_DEBUG ( ) << "AOMax: " << ao_chan_range.fAOMax;
	QLOG_DEBUG ( ) << "AOMin: " << ao_chan_range.fAOMin;
	QLOG_DEBUG ( ) << "Configuring " << dac << " Channel " << i << "...Done";
	
      }
    }
    QLOG_DEBUG ( ) << "Dac configuration success";
    connectSuccess = true;
  }
  // Update control widget
  emit getOutputs(outputs,mode);
  emit getDescription(description);
  // Get registered values from Db
  QStringList dacvaluesStringList;
  dacvalues->clear();
  dacvaluesStringList = dacvaluesString.split(" ",QString::SkipEmptyParts);
  for (int i = 0 ; i < dacvaluesStringList.size(); i++ )
    dacvalues->push_back(dacvaluesStringList.at(i).toFloat());
  
  emit getOutputValues(dacvalues);
  return connectSuccess;
}
bool
Dac::resetDac(QString newdac) {
  QSqlDatabase db = QSqlDatabase::database(path);
  QSqlQuery query(db);
  int status;
  char err_msg[100];
  if (dac == newdac && connectSuccess == true) {  
    /* Init device channels to 0 */
    /* enable sync*/
    status = DRV_EnableSyncAO(fd, 1);
    if (status) {
      DRV_GetErrorMessage(status, err_msg);
      emit showWarning(tr("DRV_EnableSyncAO:%1").arg(err_msg));
   
      return false;
    }
    dacvalues->clear();
    float value = 0;
    for (int i = 0; i < outputs; i++) {
      if (mode == "VOLT") {
	memset(&chOutV, 0, sizeof(PT_AOVoltageOut));
	chOutV.chan = i;
	chOutV.OutputValue = value;
      }
      else if (mode == "CURRENT") {
	memset(&chOutC, 0, sizeof(PT_AOCurrentOut));
	chOutC.chan = i;
	chOutC.OutputValue = value;
	
      }
      if (mode == "VOLT")
	status = DRV_AOVoltageOut(fd, &chOutV);
      else if (mode == "CURRENT")
	status = DRV_AOCurrentOut(fd, &chOutC);
      
      if (status) {
	DRV_GetErrorMessage(status, err_msg);
	emit showWarning(tr("DRV_AO/Current/Voltage/Out%1").arg(err_msg));

	return false;
      }
      dacvalues->push_back(value);
    }
    /* write sync*/
    status = DRV_WriteSyncAO(fd);
    if (status) {
      DRV_GetErrorMessage(status, err_msg);
      emit showWarning(tr("DRV_WriteSyncAO:%1").arg(err_msg));
   
      return false;
    } 
    /* disable sync*/
    status = DRV_EnableSyncAO(fd, 0);
    if (status) {
      DRV_GetErrorMessage(status, err_msg);
      emit showWarning(tr("DRV_EnableSyncAO:%1").arg(err_msg));
      
      return false;
    }
    // Set new values in Db
    QString dacvaluesString = "";
    QString valueString;
    for (int i = 0 ; i < outputs; i++) {
      valueString.setNum (dacvalues->at(i), 'f',3);
      dacvaluesString = dacvaluesString + valueString + " ";
    }
    query.prepare("update dac_settings set dacvalues = ? where name = ?");
    query.addBindValue(dacvaluesString);
    query.addBindValue(dac);
    query.exec();
    // Update control widget
    emit getOutputValues(dacvalues);
    return true;
  }
  emit showWarning(tr("Check connection to %1").arg(newdac));
  return false;
}
bool
Dac::setDacValue(int output, float value) {
  int status;
  char err_msg[100];
  QSqlDatabase db = QSqlDatabase::database(path);
  QSqlQuery query(db);

  if (connectSuccess == true) {
    QLOG_DEBUG ( ) << "Dac::setDacValue " << output << ":" << value;
    // Assign Dac value to ouptut
    // enable sync
    status = DRV_EnableSyncAO(fd, 1);
    if (status) {
      DRV_GetErrorMessage(status, err_msg);
      emit showWarning(tr("%1").arg(err_msg));
      return false;
    }
    // check value in range
    if ( value >= min && value <= max ) {
      if (mode == "VOLT") {
	memset(&chOutV, 0, sizeof(PT_AOVoltageOut));
	chOutV.chan = output;
	chOutV.OutputValue = value;
      }
      else if (mode == "CURRENT") {
	memset(&chOutC, 0, sizeof(PT_AOCurrentOut));
	chOutC.chan = output;
	chOutC.OutputValue = value;
      }
      QLOG_DEBUG ( ) << "Setting new " << mode << " " << value
		     << " for " << dac << " at channel " <<  output;
      if (mode == "VOLT")
	status = DRV_AOVoltageOut(fd, &chOutV);
      else if (mode == "CURRENT")
	status = DRV_AOCurrentOut(fd, &chOutC);
      if (status) {
	DRV_GetErrorMessage(status, err_msg);
	emit showWarning(tr("%1").arg(err_msg));
	return false;
      }
      dacvalues->replace(output,value);
    }
    else 
      emit showWarning(tr("Dac value %1 out of range !").arg(value));
    
    /* write sync*/
    status = DRV_WriteSyncAO(fd);
    if (status) {
      DRV_GetErrorMessage(status, err_msg);
      emit showWarning(tr("%1").arg(err_msg));
      return false;
    } 
    /* disable sync*/
    status = DRV_EnableSyncAO(fd, 0);
    if (status) {
      DRV_GetErrorMessage(status, err_msg);
      emit showWarning(tr("%1").arg(err_msg));
      return false;
    }
    // Set new values in Db
    QString dacvaluesString = "";
    QString valueString;
    for (int i = 0 ; i < outputs; i++) {
      valueString.setNum (dacvalues->at(i), 'f',3);
      dacvaluesString = dacvaluesString + valueString + " ";
    }
    query.prepare("update dac_settings set dacvalues = ? where name = ?");
    query.addBindValue(dacvaluesString);
    query.addBindValue(dac);
    query.exec();
    QLOG_WARN ( ) << query.lastError().text();
    // Update control widget
    emit getOutputValues(dacvalues);
  }
  return true;
}
// function : create connexion to the database
void 
Dac::dbConnexion() {

  path.append(QDir::separator()).append("dac.db3");
  path = QDir::toNativeSeparators(path);
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",path);
  db.setDatabaseName(path);  
  if ( !db.open() ) {
    QLOG_WARN ( ) << db.lastError().text();
    emit showWarning(db.lastError().text());
    return;
  }
  // Create dac tables
  QSqlQuery query(db);
  query.exec("create table dac_settings "
	     "(name varchar(128) not null primary key, "
	     "settings varchar(255), "
	     "description varchar(128), "
	     "dacvalues varchar(255))");
  QLOG_DEBUG ( ) << query.lastError().text();      
  
}

