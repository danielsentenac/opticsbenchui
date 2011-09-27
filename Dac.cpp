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
  // Connect dac db
  dbConnexion();
} 
Dac::~Dac()
{
  QLOG_DEBUG ( ) <<"deleting Dac";
  for (int i = 0 ; i < fd.size(); i++) {
    if (fd.at(i) )   {
      if ( connectSuccess.at(i) == true) DRV_DeviceClose(fd.at(i));
      free(fd.at(i)); fd.replace(i,NULL);
    }
  }
  fd.clear();
  for (int i = 0 ; i < config.size(); i++) {
    if (config.at(i))   {
      free(config.at(i)); config.replace(i, NULL); 
    }
  }
  config.clear();
  for (int i = 0 ; i < chOutV.size(); i++) {
    if (chOutV.at(i))   {
      free(chOutV.at(i)); chOutV.replace(i, NULL); 
    }
  }
  chOutV.clear();
  for (int i = 0 ; i < chOutC.size(); i++) {
    if (chOutC.at(i))   {
      free(chOutC.at(i)); chOutC.replace(i, NULL); 
    }
  }
  chOutC.clear();
  for (int i = 0 ; i < ao_chan_range.size(); i++) {
    if (ao_chan_range.at(i))   {
      free(ao_chan_range.at(i)); ao_chan_range.replace(i, NULL); 
    }
  }
  ao_chan_range.clear();
  for (int i = 0 ; i < dacvalues.size(); i++)
    if (dacvalues.at(i)) delete dacvalues.at(i);
  dacvalues.clear();
  dacSettings.clear();
  dac.clear();
  mode.clear();
  min.clear();
  max.clear();
  outputs.clear();
  fname.clear();
  connectSuccess.clear();
  {
    QSqlDatabase db = QSqlDatabase::database(path);
    db.close();
  }
  QSqlDatabase::removeDatabase(path);
}
void 
Dac::setDbPath(QString _path){

  // Re-Init object
  for (int i = 0 ; i < fd.size(); i++) {
    if (fd.at(i))   {
      if ( connectSuccess.at(i) == true) DRV_DeviceClose(fd.at(i));
      free(fd.at(i)); fd.replace(i, NULL);
    }
  }
  fd.clear();
  for (int i = 0 ; i < config.size(); i++) {
    if (config.at(i))   {
      free(config.at(i)); config.replace(i, NULL); 
    }
  }
  config.clear();
  for (int i = 0 ; i < chOutV.size(); i++) {
    if (chOutV.at(i))   {
      free(chOutV.at(i)); chOutV.replace(i, NULL); 
    }
  }
  chOutV.clear();
  for (int i = 0 ; i < chOutC.size(); i++) {
    if (chOutC.at(i))   {
      free(chOutC.at(i)); chOutC.replace(i, NULL); 
    }
  }
  chOutC.clear();
  for (int i = 0 ; i < ao_chan_range.size(); i++) {
    if (ao_chan_range.at(i))   {
      free(ao_chan_range.at(i)); ao_chan_range.replace(i, NULL); 
    }
  }
  ao_chan_range.clear();
 
  for (int i = 0 ; i < dacvalues.size(); i++)
    if (dacvalues.at(i)) delete dacvalues.at(i); 
  dacvalues.clear();
  dacSettings.clear();
  dac.clear();
  mode.clear();
  min.clear();
  max.clear();
  outputs.clear();
  fname.clear();
  connectSuccess.clear();

  // Close Db
  {
    QSqlDatabase db = QSqlDatabase::database(path);
    db.close();
  }
  QSqlDatabase::removeDatabase(path);
  path = _path;
  // Connect dac db
  dbConnexion();
}
bool
Dac::connectDac(QString newdac) {
  
  int index;
  if (newdac == "") return false;
  
  char err_msg[100];
  QSqlDatabase db = QSqlDatabase::database(path);
  QSqlQuery query(db);
  int status;
  QString description;
  QString dacvaluesString = "";
  QString dacSettings = "";
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
  for (int i = 0 ; i < dac.size(); i++)  {
    if ( dac.at(i) == newdac ) {
      QLOG_DEBUG () << newdac << " already connected";
      if ( connectSuccess.at(i) == true )
	return true;
      else  {
	QLOG_DEBUG () << newdac << " try to reconnect...";
	//
	// Free DAC related structures
	//
	if (fd.at(i))   {
	  free(fd.at(i));
	  fd.remove(i);
	}
	if (config.at(i))   {
	  free(config.at(i));
	  config.remove(i);
	}
	if (chOutV.at(i))   {
	  free(chOutV.at(i));
	  chOutV.remove(i);
	}
	if (chOutC.at(i))   {
	  free(chOutC.at(i));
	  chOutC.remove(i);
	}
	if (ao_chan_range.at(i))   {
	  free(ao_chan_range.at(i));
	  ao_chan_range.remove(i);
	}
	if (dacvalues.at(i)) delete dacvalues.at(i); 
	dacvalues.remove(i);
	dacSettings.remove(i);
	dac.remove(i);
	mode.remove(i);
	min.remove(i);
	max.remove(i);
	outputs.remove(i);
	fname.remove(i);
	connectSuccess.remove(i);
	break;
      }
    }
  }
  // First time Dac connection
  dac.push_back(newdac);
  // Assign index
  index = dac.size() - 1;
  QStringList settings;
  QStringList subsettings;
  settings = dacSettings.split(" ");
  subsettings = settings.at(0).split("=");
  fname.push_back(subsettings.at(1));
  subsettings = settings.at(1).split("=");
  mode.push_back(subsettings.at(1));
  subsettings = settings.at(2).split("=");
  outputs.push_back(subsettings.at(1).toInt());
  subsettings = settings.at(3).split("=");
  max.push_back(subsettings.at(1).toFloat());
  subsettings = settings.at(4).split("=");
  min.push_back(subsettings.at(1).toFloat());
  connectSuccess.push_back(false);
  dacvalues.push_back(new QVector<float>);
  PTR_T* fdPtr = (PTR_T*)malloc(sizeof(PTR_T));
  fd.push_back(fdPtr);
  PT_AOConfig* configPtr = (PT_AOConfig*)malloc(sizeof(PT_AOConfig));
  config.push_back(configPtr);
  PT_AOVoltageOut* chOutVPtr = (PT_AOVoltageOut*)malloc(sizeof(PT_AOVoltageOut));
  chOutV.push_back(chOutVPtr);
  PT_AOCurrentOut* chOutCPtr = (PT_AOCurrentOut*)malloc(sizeof(PT_AOCurrentOut));
  chOutC.push_back(chOutCPtr);
  AORANGESET* ao_chan_rangePtr = (AORANGESET*)malloc(sizeof(AORANGESET));
  ao_chan_range.push_back(ao_chan_rangePtr);
  
  QLOG_DEBUG ( ) << "fname " <<  fname.at(index);
  QLOG_DEBUG ( ) << "mode = "  <<   mode.at(index);
  QLOG_DEBUG ( ) << "Outputs = " << outputs.at(index);
  QLOG_DEBUG ( ) << "Max =  " << max.at(index);
  QLOG_DEBUG ( ) << "Min =  " << min.at(index);
  
  // Open device
  QLOG_INFO() << " Open device " << fname.at(index);
  status = DRV_DeviceOpen((char*)fname.at(index).toStdString().c_str(), fd.at(index));
  if (status) {
    DRV_GetErrorMessage(status, err_msg);
    emit showWarning(tr("DRV_DeviceOpen:%1").arg(err_msg));
    connectSuccess.replace(index, false);
    return false;
  }
  if (mode.at(index) == "VOLT") {
    for (int i = 0; i < outputs.at(index); i++) {
      memset(config.at(index), 0, sizeof(PT_AOConfig));
      config.at(index)->chan = i;
      config.at(index)->RefSrc = 0;
      config.at(index)->MaxValue = max.at(index);
      config.at(index)->MinValue = min.at(index);
      status = DRV_AOConfig(*(fd.at(index)), config.at(index));
      if (status) {
	DRV_GetErrorMessage(status, err_msg);
	emit showWarning(tr("DRV_AOConfig:%1").arg(err_msg));
	connectSuccess.replace(index, false);
	return false;
      }
    }
  }
  else if (mode.at(index) == "CURRENT") {
    AORANGESET aorange;
    for (int i = 0; i < outputs.at(index); i++) {
      memset(ao_chan_range.at(index), 0, sizeof(aorange));
      ao_chan_range.at(index)->usGainCount = 0;
      ao_chan_range.at(index)->usAOSource = 1; /* =0 internal, =1 external */
      ao_chan_range.at(index)->usAOType = 1; /* =0 voltage, =1 current */
      ao_chan_range.at(index)->usChan = i;
      ao_chan_range.at(index)->fAOMax = max.at(index);
      ao_chan_range.at(index)->fAOMin = min.at(index);
      status = DRV_DeviceSetProperty(*(fd.at(index)), 
				     CFG_AoChanRange,ao_chan_range.at(index),
				     sizeof(aorange));
      if (status) {
	DRV_GetErrorMessage(status, err_msg);
	emit showWarning(tr("DRV_DeviceSetProperty:%1").arg(err_msg));
	connectSuccess.replace(index, false);
	return false;
      }
      INT32U size;
      status = DRV_DeviceGetProperty(*(fd.at(index)), 
				     CFG_AoChanRange, 
				     ao_chan_range.at(index),&size);
      if (status) {
	DRV_GetErrorMessage(status, err_msg);
	emit showWarning(tr("DRV_DeviceGetProperty:%1").arg(err_msg));
	connectSuccess.replace(index, false);
	return false;
      }
      QLOG_DEBUG ( ) << dac.at(index) << " Channel " << i 
		     << " GainCount: " <<  ao_chan_range.at(index)->usGainCount;
      QLOG_DEBUG ( ) << "AOSource: " << ao_chan_range.at(index)->usAOSource;
      QLOG_DEBUG ( ) << "AOType: " << ao_chan_range.at(index)->usAOType;
      QLOG_DEBUG ( ) << "Chan: " << ao_chan_range.at(index)->usChan;
      QLOG_DEBUG ( ) << "AOMax: " << ao_chan_range.at(index)->fAOMax;
      QLOG_DEBUG ( ) << "AOMin: " << ao_chan_range.at(index)->fAOMin;
      QLOG_DEBUG ( ) << "Configuring " << dac.at(index) << " Channel " << i << "...Done";
    }
  }
  QLOG_DEBUG ( ) << "Dac configuration success";
  connectSuccess.replace(index, true);
  
  // Update control widget
  emit getOutputs(outputs.at(index),mode.at(index));
  emit getDescription(description);
  // Get registered values from Db
  QStringList dacvaluesStringList;
  dacvaluesStringList = dacvaluesString.split(" ",QString::SkipEmptyParts);
  for (int i = 0 ; i < dacvaluesStringList.size(); i++ )
    dacvalues.at(index)->push_back(dacvaluesStringList.at(i).toFloat());
  
  emit getOutputValues(dacvalues.at(index));
  return connectSuccess.at(index);
}
bool
Dac::resetDac(QString newdac) {
  QSqlDatabase db = QSqlDatabase::database(path);
  QSqlQuery query(db);
  int status;
  char err_msg[100];
  for (int index = 0 ; index < dac.size(); index++)  {
    if ( dac.at(index) == newdac && connectSuccess.at(index) == true) {
      /* Init device channels to 0 */
      /* enable sync*/
      status = DRV_EnableSyncAO(*(fd.at(index)), 1);
      if (status) {
	DRV_GetErrorMessage(status, err_msg);
	emit showWarning(tr("DRV_EnableSyncAO:%1").arg(err_msg));
	return false;
      }
      dacvalues.at(index)->clear();
      float value = 0;
      for (int i = 0; i < outputs.at(index); i++) {
	if (mode.at(index) == "VOLT") {
	  memset(chOutV.at(index), 0, sizeof(PT_AOVoltageOut));
	  chOutV.at(index)->chan = i;
	  chOutV.at(index)->OutputValue = value;
	}
	else if (mode.at(index) == "CURRENT") {
	  memset(chOutC.at(index), 0, sizeof(PT_AOCurrentOut));
	  chOutC.at(index)->chan = i;
	  chOutC.at(index)->OutputValue = value;
	  
	}
	if (mode.at(index) == "VOLT")
	  status = DRV_AOVoltageOut(*(fd.at(index)), chOutV.at(index));
	else if (mode.at(index) == "CURRENT")
	  status = DRV_AOCurrentOut(*(fd.at(index)), chOutC.at(index));
	
	if (status) {
	  DRV_GetErrorMessage(status, err_msg);
	  emit showWarning(tr("DRV_AO/Current/Voltage/Out%1").arg(err_msg));
	  return false;
	}
	dacvalues.at(index)->push_back(value);
      }
      /* write sync*/
      status = DRV_WriteSyncAO(*(fd.at(index)));
      if (status) {
	DRV_GetErrorMessage(status, err_msg);
	emit showWarning(tr("DRV_WriteSyncAO:%1").arg(err_msg));
	return false;
      } 
      /* disable sync*/
      status = DRV_EnableSyncAO(*(fd.at(index)), 0);
      if (status) {
	DRV_GetErrorMessage(status, err_msg);
	emit showWarning(tr("DRV_EnableSyncAO:%1").arg(err_msg));
	return false;
      }
      // Set new values in Db
      QString dacvaluesString = "";
      QString valueString;
      for (int i = 0 ; i < outputs.at(index); i++) {
	valueString.setNum (dacvalues.at(index)->at(i), 'f',3);
	dacvaluesString = dacvaluesString + valueString + " ";
      }
      query.prepare("update dac_settings set dacvalues = ? where name = ?");
      query.addBindValue(dacvaluesString);
      query.addBindValue(dac.at(index));
      query.exec();
      // Update control widget
      emit getOutputValues(dacvalues.at(index));
      return true;
    }
  }
  emit showWarning(tr("Check connection to %1").arg(newdac));
  return false; 
}
bool
Dac::setDacValue(QString newdac, int output, float value) {
  int status;
  char err_msg[100];

  for (int index = 0 ; index < dac.size(); index++)  {
    if ( dac.at(index) == newdac && connectSuccess.at(index) == true) {
      QLOG_DEBUG ( ) << "Dac::setDacValue " << newdac << ": " << output << ":" << value;
      // Assign Dac value to ouptut
      // enable sync
      status = DRV_EnableSyncAO(*(fd.at(index)), 1);
      if (status) {
	DRV_GetErrorMessage(status, err_msg);
	emit showWarning(tr("%1").arg(err_msg));
	return false;
      }
      // check value in range
      if ( value >= min.at(index) && value <= max.at(index) ) {
	if (mode.at(index) == "VOLT") {
	  memset(chOutV.at(index), 0, sizeof(PT_AOVoltageOut));
	  chOutV.at(index)->chan = output;
	  chOutV.at(index)->OutputValue = value;
	}
	else if (mode.at(index) == "CURRENT") {
	  memset(chOutC.at(index), 0, sizeof(PT_AOCurrentOut));
	  chOutC.at(index)->chan = output;
	  chOutC.at(index)->OutputValue = value;
	}
	QLOG_DEBUG ( ) << "Setting new " << mode.at(index) << " " << value
		       << " for " << dac.at(index) << " at channel " <<  output;
	if (mode.at(index) == "VOLT")
	  status = DRV_AOVoltageOut(*(fd.at(index)), chOutV.at(index));
	else if (mode.at(index) == "CURRENT")
	  status = DRV_AOCurrentOut(*(fd.at(index)), chOutC.at(index));
	if (status) {
	  DRV_GetErrorMessage(status, err_msg);
	  emit showWarning(tr("%1").arg(err_msg));
	  return false;
	}
	dacvalues.at(index)->replace(output,value);
      }
      else 
	emit showWarning(tr("Dac value %1 out of range !").arg(value));
      
      /* write sync*/
      status = DRV_WriteSyncAO(*(fd.at(index)));
      if (status) {
	DRV_GetErrorMessage(status, err_msg);
	emit showWarning(tr("%1").arg(err_msg));
	return false;
      } 
      /* disable sync*/
      status = DRV_EnableSyncAO(*(fd.at(index)), 0);
      if (status) {
	DRV_GetErrorMessage(status, err_msg);
	emit showWarning(tr("%1").arg(err_msg));
	return false;
      }
      return true;
    }
  }
  emit showWarning(tr("Check connection to %1").arg(newdac));
  return false; 
}

// function : update DB with DAC value
bool
Dac::updateDBValues(QString newdac) {
  
  for (int index = 0 ; index < dac.size(); index++)  {
    if ( dac.at(index) == newdac ) {
      QLOG_DEBUG () << " Update DB DAC Values";
      QSqlDatabase db = QSqlDatabase::database(path);
      QSqlQuery query(db);
      // Set new values in Db
      QString dacvaluesString = "";
      QString valueString;
      for (int i = 0 ; i < outputs.at(index); i++) {
	valueString.setNum (dacvalues.at(index)->at(i), 'f',3);
	dacvaluesString = dacvaluesString + valueString + " ";
      }
      query.prepare("update dac_settings set dacvalues = ? where name = ?");
      query.addBindValue(dacvaluesString);
      query.addBindValue(dac.at(index));
      query.exec();
      
      // Update control widget
      emit getOutputValues(dacvalues.at(index));
      return  true;
    }
  }
  emit showWarning(tr("Cannot find dac %1").arg(newdac));
  return false; 
}
// function : create connexion to the database
void 
Dac::dbConnexion() {

  path.append(QDir::separator()).append("dac.db3");
  path = QDir::toNativeSeparators(path);
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",path);
  QLOG_DEBUG ( ) << " Db path : " << path;
  db.setDatabaseName(path);  
  if ( !db.open() ) {
    QLOG_WARN ( ) << db.lastError().text();
    emit showWarning(db.lastError().text());
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

