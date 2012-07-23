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
#ifdef COMEDICOUNTER
#include "ComediCounter.h"

ComediCounter::ComediCounter(QString _appDirPath)
  : Comedi()				       
{ 
  path = _appDirPath;
  comeditype = "COUNTER";
  
  // Connect comedi db
  dbConnexion();
} 

ComediCounter::~ComediCounter()
{
  QLOG_DEBUG ( ) <<"deleting ComediCounter";
 
  for (int i = 0 ; i < comedivalues.size(); i++)
    if (comedivalues.at(i)) delete comedivalues.at(i);
  for (int i = 0 ; i < device.size(); i++)
    if (device.at(i))  comedi_close(device.at(i));
  device.clear();
  comedivalues.clear();
  comediSettings.clear();
  comedi.clear();
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
ComediCounter::setDbPath(QString _path){

  for (int i = 0 ; i < comedivalues.size(); i++)
    if (comedivalues.at(i)) delete comedivalues.at(i); 
  for (int i = 0 ; i < device.size(); i++)
    if (device.at(i))  comedi_close(device.at(i));
  device.clear();
  comedivalues.clear();
  comediSettings.clear();
  comedi.clear();
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
ComediCounter::connectComedi(QString newcomedi) {
  
  int index;
  if (newcomedi == "") return false;
  
  QLOG_INFO() << "ComediCounter::connectComedi> connecting COUNTER " << newcomedi;
  QSqlDatabase db = QSqlDatabase::database(path);
  QSqlQuery query(db);
  QString description;
  QString comedivaluesString = "";
  QString comediSettings = "";
 
  //
  // Check if comedi already configured
  //
  for (int i = 0 ; i < comedi.size(); i++)  {
    if ( comedi.at(i) == newcomedi ) {
      QLOG_DEBUG () << newcomedi << " already connected";
      if ( connectSuccess.at(i) == true )
	return true;
      else  {
	QLOG_DEBUG () << newcomedi << " try to reconnect...";
	if (comedivalues.at(i)) delete comedivalues.at(i); 
	comedivalues.remove(i);
	comediSettings.remove(i);
	comedi.remove(i);
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
  // First time ComediCounter connection
  //
  // Configure ComediCounter
  //
  query.prepare("select settings,description,comedivalues from comedi_settings where name = ?");
  query.addBindValue(newcomedi);
  query.exec();

  while (query.next()) {
    comediSettings = query.value(0).toString();
    description = query.value(1).toString();
    comedivaluesString = query.value(2).toString();
  }
  if (comediSettings == "") {
    emit showWarning(tr("%1 does not exist").arg(newcomedi));
    return false;
  }

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
  outputs.push_back(subsettings.at(1).toInt());
  subsettings = settings.at(3).split("=");
  min.push_back(subsettings.at(1).toFloat());
  subsettings = settings.at(4).split("=");
  max.push_back(subsettings.at(1).toFloat());
  connectSuccess.push_back(false);
  comedivalues.push_back(new QVector<float>);
  
  QLOG_INFO() << "ComediCounter::connectComedi> mode : " << mode.at(index);
  QLOG_INFO() << "ComediCounter::connectComedi> outputs : " << outputs.at(index);
  QLOG_INFO() << "ComediCounter::connectComedi> max : " << max.at(index);
  QLOG_INFO() << "ComediCounter::connectComedi> min : " << min.at(index);
  // Open device
  QLOG_INFO() << "ComediCounter::connectComedi> open device " 
              << fname.at(index) << "(index = " << index << ")";
  comedi_t *newdevice;
  newdevice = comedi_open(fname.at(index).toStdString().c_str());
  if (newdevice == NULL)
   {
    emit showWarning(tr("comedi_open:%1 (%2)").arg(fname.at(index).toStdString().c_str(),
						comedi_strerror(comedi_errno())));
    connectSuccess.replace(index, false);
    return false;
   }

  device.push_back(newdevice);
  QLOG_DEBUG ( ) << "ComediCounter configuration success";
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
ComediCounter::resetComedi(QString newcomedi) {
  QSqlDatabase db = QSqlDatabase::database(path);
  QSqlQuery query(db);
  for (int index = 0 ; index < comedi.size(); index++)  {
    if ( comedi.at(index) == newcomedi && connectSuccess.at(index) == true) {
      /* Init device channels to 0 */
      QLOG_INFO() << "ComediCounter::resetComedi> resetting " 
                  << newcomedi << " subdevice range [" 
                  << min.at(index) << "," << max.at(index) 
                  << "]";
      comedivalues.at(index)->clear();
      float value = 0;
      for (int i = 0; i < outputs.at(index); i++) {
	comedivalues.at(index)->push_back(value);
      }
      /* reset and configure counter device*/
      int ret;
      lsampl_t counter_mode;
      for (int subdev = min.at(index); subdev <= max.at(index); subdev++) {
        QLOG_INFO() << "ComediCounter::resetComedi> resetting and configuring " << newcomedi 
                    << " subdevice " << subdev;

        ret = reset_counter(device.at(index), subdev);
        if(ret < 0) return false;
        
/*  Simple counter ( gate source with internal clock ) */
        ret = set_gate_source(device.at(index), subdev, 0,NI_GPCT_GATE_PIN_GATE_SELECT(0));
        if(ret < 0) return false;
        ret = set_gate_source(device.at(index), subdev, 1,NI_GPCT_DISABLED_GATE_SELECT| CR_EDGE );
        if(ret < 0) return false;
        ret = set_clock_source(device.at(index), subdev, NI_GPCT_TIMEBASE_1_CLOCK_SRC_BITS, 50 );
        if(ret < 0) return ret;

/*  Internal clock mode COUNTER 
        ret = set_gate_source(device.at(index), subdev, 0,NI_GPCT_DISABLED_GATE_SELECT| CR_EDGE);
        if(ret < 0) return ret;
        ret = set_gate_source(device.at(index), subdev, 1,NI_GPCT_DISABLED_GATE_SELECT| CR_EDGE );
        if(ret < 0) return ret;
        ret=set_clock_source(device.at(index), subdev, NI_GPCT_TIMEBASE_1_CLOCK_SRC_BITS, 50);
        if(ret < 0) return ret;
*/
        counter_mode = NI_GPCT_COUNTING_MODE_NORMAL_BITS;
        // output pulse on terminal count (doesn't really matter for this application)
        counter_mode |= NI_GPCT_OUTPUT_TC_PULSE_BITS;
        counter_mode |= NI_GPCT_RELOAD_SOURCE_FIXED_BITS;
        // count up
        counter_mode |= NI_GPCT_COUNTING_DIRECTION_UP_BITS;
        // don't stop on terminal count
        counter_mode |= NI_GPCT_STOP_ON_GATE_BITS;
        // don't disarm on terminal count or gate signal
        counter_mode |= NI_GPCT_NO_HARDWARE_DISARM_BITS;
        // Don't alternate the reload source between the load a and load b registers.
        // Doesn't really matter here, since we aren't going to be reloading the counter.
      
        ret = set_counter_mode(device.at(index), subdev, counter_mode);
        if(ret < 0) return false;

        // set initial counter value by writing to channel 0
        ret = comedi_data_write(device.at(index), subdev, 0, 0, 0 ,0);
        if(ret < 0) return false;
        float resetValue = 0.0;
        comedivalues.at(index)->replace(subdev - min.at(index), resetValue);
      }
      // set new values in Db
      QString comedivaluesString = "";
      QString valueString;
      for (int i = 0 ; i < outputs.at(index); i++) {
	valueString.setNum (comedivalues.at(index)->at(i), 'f',3);
	comedivaluesString = comedivaluesString + valueString + " ";
      }
      query.prepare("update comedi_settings set comedivalues = ? where name = ?");
      query.addBindValue(comedivaluesString);
      query.addBindValue(comedi.at(index));
      query.exec();
      // Update control widget
      emit getOutputValues((void*)comedivalues.at(index));
      return true;
    }
  }
  emit showWarning(tr("Check connection to %1").arg(newcomedi));
  return false; 
}
bool
ComediCounter::setComediValue(QString newcomedi, int output, void *value) {
    
  int ret;
  lsampl_t data;
  long eTime;
  // Convert value to expected type
  int *ivalue;
  ivalue = (int*) value;

  //  Start counting for fvalue time (in microseconds)
  for (int index = 0 ; index < comedi.size(); index++)  {
    if ( comedi.at(index) == newcomedi ) {
      QLOG_DEBUG() << "ComediCounter::setComediValue> set value " << *ivalue;
      // Update output to math subdevice index
      int subdev = output + min.at(index);
      // set initial counter value by writing to channel 0
      ret = comedi_data_write(device.at(index), subdev, 0, 0, 0 ,0);
      eTime = getTime(device.at(index));
      ret = start_up_down_counting(device.at(index), subdev);
      usleep(*ivalue);
      ret = stop_up_down_counting(device.at(index), subdev);
      eTime = getTime(device.at(index)) - eTime;
      // Get result counts
      ret = comedi_data_read(device.at(index), subdev, 0, 0, AREF_GROUND, &data);
      if(ret < 0) return false;
      QLOG_INFO() << "ComediCounter::setComediValue> raw data " << (unsigned long) data;
      QLOG_INFO() << "ComediCounter::setComediValue> eTime " << eTime;
      double formattedData = (double) data / eTime; formattedData*=1000000;
      QString dbgStr;
      dbgStr.setNum(formattedData, 'lf', 6);
      QLOG_INFO() << "ComediCounter::setComediValue> formatted data " << dbgStr; 
      comedivalues.at(index)->replace(output, (float) formattedData);
      return true;
    }
  }
  return false; 
}
bool 
ComediCounter::getComediValue(QString newcomedi, int output, float &value) {
 for (int index = 0 ; index < comedi.size(); index++)  {
    if ( comedi.at(index) == newcomedi ) {
       value =  comedivalues.at(index)->at(output);
       return true;
    }
  }
 return false;
}
// function : update DB with counter values
bool
ComediCounter::updateDBValues(QString newcomedi) {
  
  for (int index = 0 ; index < comedi.size(); index++)  {
    if ( comedi.at(index) == newcomedi ) {
      QLOG_DEBUG () << " Update DB counter values";
      QSqlDatabase db = QSqlDatabase::database(path);
      QSqlQuery query(db);
      // Set new values in Db
      QString comedivaluesString = "";
      QString valueString;
      for (int i = 0 ; i < outputs.at(index); i++) {
	valueString.setNum (comedivalues.at(index)->at(i), 'f',3);
	comedivaluesString = comedivaluesString + valueString + " ";
      }
      query.prepare("update comedi_settings set comedivalues = ? where name = ?");
      query.addBindValue(comedivaluesString);
      query.addBindValue(comedi.at(index));
      query.exec();
      
      // Update control widget
      emit getOutputValues((void*)comedivalues.at(index));
      return  true;
    }
  }
  emit showWarning(tr("Cannot find comedi %1").arg(newcomedi));
  return false; 
}
// This resets the count to zero and disarms the counter.  The counter output is set low.
int 
ComediCounter::reset_counter(comedi_t *device, unsigned subdevice)
{
        comedi_insn insn;
        lsampl_t data[1];
        int ret;

        insn.insn = INSN_CONFIG;
        insn.subdev = subdevice;
        insn.chanspec = 0;
        insn.data = data;
        insn.n = sizeof(data) / sizeof(data[0]);
        data[0] = INSN_CONFIG_RESET;

        ret = comedi_do_insn(device, &insn);
        if(ret < 0)
        {
                fprintf(stderr, "%s: error:\n", __FUNCTION__);
                comedi_perror("comedi_do_insn");
                return ret;
        }
        return 0;
}
// arm
int 
ComediCounter::arm(comedi_t *device, unsigned subdevice, lsampl_t source)
{
        comedi_insn insn;
        lsampl_t data[2];
        int ret;

        insn.insn = INSN_CONFIG;
        insn.subdev = subdevice;
        insn.chanspec = 0;
        insn.data = data;
        insn.n = sizeof(data) / sizeof(data[0]);
        data[0] = INSN_CONFIG_ARM;
        data[1] = source;

        ret = comedi_do_insn(device, &insn);
        if(ret < 0)
        {
                fprintf(stderr, "%s: error:\n", __FUNCTION__);
                comedi_perror("comedi_do_insn");
                return ret;
        }
        return 0;
}
// set counter modes
int 
ComediCounter::set_counter_mode(comedi_t *device, unsigned subdevice, lsampl_t mode_bits)
{
        comedi_insn insn;
        lsampl_t data[2];
        int ret;

        insn.insn = INSN_CONFIG;
        insn.subdev = subdevice;
        insn.chanspec = 0;
        insn.data = data;
        insn.n = sizeof(data) / sizeof(data[0]);
        data[0] = INSN_CONFIG_SET_COUNTER_MODE;
        data[1] = mode_bits;

        ret = comedi_do_insn(device, &insn);
        if(ret < 0)
        {
                fprintf(stderr, "%s: error:\n", __FUNCTION__);
                comedi_perror("comedi_do_insn");
                return ret;
        }
        return 0;
}
// set clock source
int 
ComediCounter::set_clock_source(comedi_t *device, unsigned subdevice, lsampl_t clock, lsampl_t period_ns)
{
        comedi_insn insn;
        lsampl_t data[3];
        int ret;

        insn.insn = INSN_CONFIG;
        insn.subdev = subdevice;
        insn.chanspec = 0;
        insn.data = data;
        insn.n = sizeof(data) / sizeof(data[0]);
        data[0] = INSN_CONFIG_SET_CLOCK_SRC;
        data[1] = clock;
        data[2] = period_ns;

        ret = comedi_do_insn(device, &insn);
        if(ret < 0)
        {
                fprintf(stderr, "%s: error:\n", __FUNCTION__);
                comedi_perror("comedi_do_insn");
                return ret;
        }
        return 0;
}
// set gate source
int 
ComediCounter::set_gate_source(comedi_t *device, unsigned subdevice, lsampl_t gate_index, lsampl_t gate_source)
{
        comedi_insn insn;
        lsampl_t data[3];
        int ret;

        insn.insn = INSN_CONFIG;
        insn.subdev = subdevice;
        insn.chanspec = 0;
        insn.data = data;
        insn.n = sizeof(data) / sizeof(data[0]);
        data[0] = INSN_CONFIG_SET_GATE_SRC;
        data[1] = gate_index;
        data[2] = gate_source;

        ret = comedi_do_insn(device, &insn);
        if(ret < 0)
        {
          fprintf(stderr, "%s: error:\n", __FUNCTION__);
          comedi_perror("comedi_do_insn");
          return ret;
        }
        return 0;
}

// start counting
int 
ComediCounter::start_up_down_counting(comedi_t *device, unsigned subdevice)
{
    int ret;

    ret = arm(device, subdevice, NI_GPCT_ARM_IMMEDIATE);
    
    if(ret < 0) 
       return ret;
    return 0;
}
// stop counting
int 
ComediCounter::stop_up_down_counting(comedi_t *device, unsigned subdevice)
{
    lsampl_t data[1];
    comedi_insn insn;
    int ret;

    insn.insn = INSN_CONFIG;
    insn.subdev = subdevice;
    insn.chanspec = 0;
    insn.data = data;
    insn.n = sizeof(data) / sizeof(data[0]);
    data[0] = INSN_CONFIG_DISARM;

    ret = comedi_do_insn(device, &insn);
    if(ret < 0)
    {
     fprintf(stderr, "%s: error:\n", __FUNCTION__);
     comedi_perror("comedi_do_insn");
     return ret;
    }
    return 0;
}
// get current time
long 
ComediCounter::getTime(comedi_t *device)
{
    lsampl_t data[1];
    comedi_insn insn;
    int ret;
    long time;
    lsampl_t t1[2];

    // perform a gettimeofday()
    insn.insn = INSN_GTOD;
    insn.n = 2; //sizeof(data) / sizeof(data[0]);
    insn.data = t1;

    ret = comedi_do_insn(device, &insn);
    time = t1[0]*1000000 + t1[1];

    if(ret < 0)
    {
      fprintf(stderr, "%s: error:\n", __FUNCTION__);
      comedi_perror("comedi_do_insn");
    }

    return time;
}

// function : create connexion to the database
void 
ComediCounter::dbConnexion() {

  path.append(QDir::separator()).append("comedi.db3");
  path = QDir::toNativeSeparators(path);
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",path);
  QLOG_INFO ( ) << "ComediCounter::dbConnexion> Db path : " << path;
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
