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
#ifdef COMEDIDAC
#include "ComediDac.h"
#include <unistd.h>

ComediDac::ComediDac(QString _appDirPath)
  : Comedi()				       
{ 
  path = _appDirPath;
  comeditype = "COMEDIDAC";
  // Connect comedi db
  dbConnexion();
} 

ComediDac::~ComediDac()
{
  QLOG_DEBUG ( ) <<"Deleting ComediDac";
   for (int i = 0 ; i < comedivalues.size(); i++)
    if (comedivalues.at(i)) delete comedivalues.at(i);
  for (int i = 0 ; i < device.size(); i++)
    if (device.at(i))  comedi_close(device.at(i));
  device.clear();
  comedivalues.clear();
  comediSettings.clear();
  comedi.clear();
  subdev.clear();
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
}
void 
ComediDac::setDbPath(QString _path){

  for (int i = 0 ; i < comedivalues.size(); i++)
    if (comedivalues.at(i)) delete comedivalues.at(i);
  for (int i = 0 ; i < device.size(); i++)
    if (device.at(i))  comedi_close(device.at(i));
  device.clear();
  comedivalues.clear();
  comediSettings.clear();
  comedi.clear();
  subdev.clear();
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
  // Connect comedi db
  dbConnexion();
}
bool
ComediDac::connectComedi(QString newcomedi) {
 
  int index;
  QLOG_INFO () << "NEW COMEDI:" << newcomedi ;
  if (newcomedi == "") return false;

  QLOG_INFO() << "ComediDac::connectComedi> connecting DAC " << newcomedi;
  QLOG_INFO() << "PATH:" << path;
  QSqlDatabase db = connectDb(path);
  QSqlQuery query(db);
  QString description;
  QString comedivaluesString = "";
  QString comediSettings = "";

  query.prepare("select settings,description,comedivalues from comedi_settings where name = ?");
  query.addBindValue(newcomedi);
  QLOG_INFO() << query.lastQuery();
  query.exec();
  QLOG_INFO ( ) << query.lastError().text();

  while (query.next()) {
    comediSettings = query.value(0).toString();
    description = query.value(1).toString();
    comedivaluesString = query.value(2).toString();
  }
  QLOG_INFO () << "comediSettings:" << comediSettings;

  if (comediSettings == "") {
    emit showWarning(tr("%1 does not exist").arg(newcomedi));
    return false;
  }

  //
  // Check if comedi already configured
  //
  for (int i = 0 ; i < comedi.size(); i++)  {
    if ( comedi.at(i) == newcomedi ) {
      QLOG_INFO () << newcomedi << " already connected";
      if ( connectSuccess.at(i) == true )
        return true;
      else  {
        QLOG_INFO () << newcomedi << " try to reconnect...";
        if (comedivalues.at(i)) delete comedivalues.at(i);
        comedivalues.remove(i);
        comediSettings.remove(i);
        comedi.remove(i);
        subdev.remove(i);
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
  // First time ComediDac connection

  comedi.push_back(newcomedi);
  // Assign index
  index = comedi.size() - 1;
  QStringList settings;
  QStringList subsettings;
  settings = comediSettings.split(" ");
  subsettings = settings.at(0).split("=");
  fname.push_back(subsettings.at(1));
  subsettings = settings.at(1).split("=");
  mode.push_back(subsettings.at(1));
  subsettings = settings.at(2).split("=");
  subdev.push_back(subsettings.at(1).toInt());
  subsettings = settings.at(3).split("=");
  outputs.push_back(subsettings.at(1).toInt());
  subsettings = settings.at(4).split("=");
  min.push_back(subsettings.at(1).toFloat());
  subsettings = settings.at(5).split("=");
  max.push_back(subsettings.at(1).toFloat());
  connectSuccess.push_back(false);
  comedivalues.push_back(new QVector<double>());

  QLOG_INFO() << "ComediDac::connectComedi> mode : " << mode.at(index);
  QLOG_INFO() << "ComediDac::connectComedi> subdev : " << subdev.at(index);
  QLOG_INFO() << "ComediDac::connectComedi> outputs : " << outputs.at(index);
  QLOG_INFO() << "ComediDac::connectComedi> max : " << max.at(index);
  QLOG_INFO() << "DaComedi::connectComedi> min : " << min.at(index);
  QLOG_INFO() << "DaComedi::connectComedi> values : " << comedivaluesString;
  // Open device
  QLOG_INFO() << "ComediDac::connectComedi> open device "
              << fname.at(index) << "(index = " << index << ")";
  comedi_t *newdevice;
  newdevice = comedi_open(fname.at(index).toStdString().c_str());
  if (newdevice == NULL) {
    emit showWarning(tr("comedi_open:%1 (%2)").arg(fname.at(index).toStdString().c_str(),
                                                comedi_strerror(comedi_errno())));
    connectSuccess.replace(index, false);
    return false;
  }
  device.push_back(newdevice);
  comedi_set_global_oor_behavior(COMEDI_OOR_NAN);
  QLOG_INFO ( ) << "ComediDac configuration success";
  connectSuccess.replace(index, true);
  // Update control widget
  emit getOutputs(outputs.at(index),mode.at(index));
  emit getDescription(description);
  // Get registered values from Db
  QStringList comedivaluesStringList;
  comedivaluesStringList = comedivaluesString.split(" ",QString::SkipEmptyParts);
  for (int i = 0 ; i < comedivaluesStringList.size(); i++ )
    comedivalues.at(index)->push_back(comedivaluesStringList.at(i).toFloat());
  emit getOutputValues((void*)comedivalues.at(index));

  // Reset and configure comedi device
  connectSuccess.replace(index,resetComedi(newcomedi));

  return connectSuccess.at(index);
}
bool
ComediDac::resetComedi(QString newcomedi) {
  double value;
  lsampl_t data;
  int ret;
  comedi_range *rng;
  int maxdata;

  // Set all tensions to 0, reread tensions from device and update DB
  for (int index = 0 ; index < comedi.size(); index++)  {
    if ( comedi.at(index) == newcomedi && connectSuccess.at(index) == true) {
      for (int channel = 0 ; channel < outputs.at(index); channel++)  {
        double zerovalue = 0.001;
        setComediValue(newcomedi, channel, &zerovalue);
        double real_value;
        getComediValue(newcomedi, channel, real_value);
        comedivalues.at(index)->replace(channel,real_value);
        QLOG_INFO ( ) << "ComediDac::resetComedi> read device " << newcomedi << ":subdev " << subdev.at(index) 
                    << ":channel " << channel << ":value" << QString::number(real_value);
      }
      updateDBValues(newcomedi);
      return true;
    }
  }
  emit showWarning(tr("Check connection to %1").arg(newcomedi));
  return false;
}
bool
ComediDac::setComediValue(QString newcomedi, int output, void *value) {

  lsampl_t data;
  int ret;
  comedi_range *rng;
  int maxdata;
 
  // Convert value to expected type
  double *dvalue;
  dvalue = (double*) value;
  
  for (int index = 0 ; index < comedi.size(); index++)  {
    if ( comedi.at(index) == newcomedi && connectSuccess.at(index) == true) {
        if ( *dvalue < min.at(index) || *dvalue > max.at(index)) {
          emit showWarning(tr("%1: channel %2:value out of range").arg(newcomedi,
   				QString::number(output)));
          return false;
        }
        maxdata = comedi_get_maxdata(device.at(index), subdev.at(index), output); 
        rng=comedi_get_range(device.at(index), subdev.at(index), output, 0);
        data = comedi_from_phys(*dvalue,rng,maxdata);
        ret = comedi_data_write(device.at(index), subdev.at(index), output, 0, AREF_GROUND, data);
        if (ret < 0) {
          comedi_perror(comedi.at(index).toStdString().c_str());
          return false;
        }
        QLOG_INFO() << "ComediDac::setComediValue> replace at " << output
                    << " old value " << QString::number(comedivalues.at(index)->at(output))
                    << " new value " << QString::number(*dvalue);
        double real_value;
        getComediValue(newcomedi, output, real_value);
        comedivalues.at(index)->replace(output,real_value);
        return true;
    }
  }
  emit showWarning(tr("Check connection to %1").arg(newcomedi));
  return false; 
}
bool
ComediDac::getComediValue(QString newcomedi, int output, double &value) { // read value from board directly
  lsampl_t data;
  int ret;
  comedi_range *rng;
  int maxdata;
  for (int index = 0 ; index < comedi.size(); index++)  {
    if ( comedi.at(index) == newcomedi ) {
        maxdata = comedi_get_maxdata(device.at(index), subdev.at(index), output);
        rng=comedi_get_range(device.at(index), subdev.at(index), output, 0);
        ret = comedi_data_read(device.at(index), subdev.at(index), output, 0, AREF_GROUND, &data);
        if (ret < 0){
          comedi_perror(comedi.at(index).toStdString().c_str());
          return false;
        }
       value = comedi_to_phys(data,rng,maxdata);
       QLOG_INFO ( ) << "ComediDac::getComediValue> read device " << newcomedi << ":subdev " << subdev.at(index) 
                    << ":channel " << output << ":value" << QString::number(value);
       //value =  comedivalues.at(index)->at(output);
       return true;
    }
  }
 return false;
}

// function : update DB with ComediDac value
bool
ComediDac::updateDBValues(QString newcomedi) {
  
  for (int index = 0 ; index < comedi.size(); index++)  {
    if ( comedi.at(index) == newcomedi ) {
      QLOG_DEBUG () << " Update DB COMEDI DAC Values";
      QSqlDatabase db = QSqlDatabase::database(path);
      QSqlQuery query(db);
      // Set new values in Db
      QString comedivaluesString = "";
      QString valueString;
      for (int i = 0 ; i < outputs.at(index); i++) {
	valueString.setNum (comedivalues.at(index)->at(i),'f',4);
	comedivaluesString = comedivaluesString + valueString + " ";
      }
      query.prepare("update comedi_settings set comedivalues = ? where name = ?");
      query.addBindValue(comedivaluesString);
      query.addBindValue(comedi.at(index));
      query.exec();
      
      // Update control widget
      emit getOutputValues(comedivalues.at(index));
      return  true;
    }
  }
  emit showWarning(tr("Cannot find comedi %1").arg(newcomedi));
  return false; 
}
// function : create connexion to the database
void 
ComediDac::dbConnexion() {

  path.append(QDir::separator()).append("comedidac.db3");
  path = QDir::toNativeSeparators(path);
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",path);
  QLOG_INFO ( ) << "ComediDac::dbConnexion> Db path : " << path;
  db.setDatabaseName(path);  
  if ( !db.open() ) {
    QLOG_WARN ( ) << db.lastError().text();
    emit showWarning(db.lastError().text());
  }
  // Create comedi tables
  QSqlQuery query(db);
  query.exec("create table comedi_settings "
	     "(name varchar(128) not null primary key, "
	     "settings varchar(255), "
	     "description varchar(128), "
	     "comedivalues varchar(255))");
  QLOG_DEBUG ( ) << query.lastError().text();      
  
}
#endif
