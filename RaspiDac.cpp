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
#ifdef RASPIDAC
#include "RaspiDac.h"
#include <unistd.h>

#define LSB(a)    *((unsigned char *)&a)
#define MSB(a)    *(((unsigned char *)&a) + 1)

RaspiDac::RaspiDac(QString _appDirPath)
{ 
  path = _appDirPath;
  raspitype = "RASPIDAC";
  // Connect raspi db
  dbConnexion();
  // Init SPI lib
  if (!bcm2835_init())
    {
     QLOG_ERROR() << "RaspiDac::RaspiDac> bcm2835_init failed. Are you running as root?";
    }

} 

RaspiDac::~RaspiDac()
{
  QLOG_DEBUG ( ) <<"Deleting RaspiDac";
   for (int i = 0 ; i < raspivalues.size(); i++)
    if (raspivalues.at(i)) delete raspivalues.at(i);
  device.clear();
  raspivalues.clear();
  raspiSettings.clear();
  raspi.clear();
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
  // Close SPI Comm
  bcm2835_spi_end();
  bcm2835_close();
}
void 
RaspiDac::setDbPath(QString _path){

  for (int i = 0 ; i < raspivalues.size(); i++)
    if (raspivalues.at(i)) delete raspivalues.at(i);
  for (int i = 0 ; i < device.size(); i++)
  raspivalues.clear();
  raspiSettings.clear();
  raspi.clear();
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
  // Connect raspi db
  dbConnexion();
}
bool
RaspiDac::connectRaspi(QString newraspi) {
 
  int index;
  if (newraspi == "") return false;

  QLOG_INFO() << "RaspiCounter::connectRaspi> connecting DAC " << newraspi;
  QSqlDatabase db = connectDb(path);
  QSqlQuery query(db);
  QString description;
  QString raspivaluesString = "";
  QString raspiSettings = "";

  //
  // Check if raspi already configured
  //
  for (int i = 0 ; i < raspi.size(); i++)  {
    if ( raspi.at(i) == newraspi ) {
      QLOG_DEBUG () << newraspi << " already connected";
      if ( connectSuccess.at(i) == true )
        return true;
      else  {
        QLOG_DEBUG () << newraspi << " try to reconnect...";
        if (raspivalues.at(i)) delete raspivalues.at(i);
        raspivalues.remove(i);
        raspiSettings.remove(i);
        raspi.remove(i);
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
  // First time RaspiDac connection
  //
  // Configure RaspiDac
  //
  query.prepare("select settings,description,raspivalues from raspi_settings where name = ?");
  query.addBindValue(newraspi);
  query.exec();

  while (query.next()) {
    raspiSettings = query.value(0).toString();
    description = query.value(1).toString();
    raspivaluesString = query.value(2).toString();
  }
  if (raspiSettings == "") {
    emit showWarning(tr("%1 does not exist").arg(newraspi));
    return false;
  }

  raspi.push_back(newraspi);
  // Assign index
  index = raspi.size() - 1;
  QStringList settings;
  QStringList subsettings;
  settings = raspiSettings.split(" ");
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
  raspivalues.push_back(new QVector<double>());

  QLOG_INFO() << "RaspiDac::connectRaspi> mode : " << mode.at(index);
  QLOG_INFO() << "RaspiDac::connectRaspi> outputs : " << outputs.at(index);
  QLOG_INFO() << "RaspiDac::connectRaspi> max : " << max.at(index);
  QLOG_INFO() << "DaRaspi::connectRaspi> min : " << min.at(index);
  QLOG_INFO() << "DaRaspi::connectRaspi> values : " << raspivaluesString;
  // Open device
  QLOG_INFO() << "RaspiDac::connectRaspi> open device "
              << fname.at(index) << "(index = " << index  << " mode = " << mode.at(index) << ")";
    if (!bcm2835_spi_begin())
    {
      printf("bcm2835_spi_begin failed. Are you running as root??\n");
      return 1;
    }
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);                  // The default
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16);    // The default
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default
  
  // Prepare data
  char* data;
  char* read_data;
  data = (char*)malloc(3 * sizeof(char));
  read_data = (char*)malloc(3 * sizeof(char));

  read_data[0] = 0;
  read_data[1] = 0;
  read_data[2] = 0;
  
  // SPICONFIG data  // Power-up Device, Disable Soft-toggle
  data[0] = 0x03;
  data[1] = 0x0A;
  data[2] = 0x84;
  bcm2835_spi_writenb(data, 3);
  
  // DACRANGEn data // Set Dac Range for ALL channels
  if (min.at(index) == -5 && max.at(index) == 5) {  
    data[1] = 0x99;
    data[2] = 0x99;
  }
  if (min.at(index) == 0 && max.at(index) == 5) {
    QLOG_INFO() << " SET DAC RANGE (0,5)V";
    data[1] = 0x00;
    data[2] = 0x00;
  }
  if (min.at(index) == -10 && max.at(index) == 10) {
    QLOG_INFO() << " SET DAC RANGE (-10,10)V";
    data[1] = 0xAA;
    data[2] = 0xAA;
  }
  if (min.at(index) == 0 && max.at(index) == 10) {
    QLOG_INFO() << " SET DAC RANGE (0,10)V";
    data[1] = 0x11;
    data[2] = 0x11;
  }
  if (min.at(index) == -20 && max.at(index) == 20) {
    QLOG_INFO() << " SET DAC RANGE (-20,20)V";
    data[1] = 0xCC;
    data[2] = 0xCC;
  }
  if (min.at(index) == 0 && max.at(index) == 20) {
    QLOG_INFO() << " SET DAC RANGE (0,20)V";
    data[1] = 0x22;
    data[2] = 0x22;
  }
  if (min.at(index) == -2.5 && max.at(index) == 2.5) {
    QLOG_INFO() << " SET DAC RANGE (-2.5,2.5)V";
    data[1] = 0xEE;
    data[2] = 0xEE;
  }
  if (min.at(index) == 0 && max.at(index) == 40) {
    QLOG_INFO() << " SET DAC RANGE (0,40)V";
    data[1] = 0x44;
    data[2] = 0x44;
  }
  data[0] = 0x0A;
  bcm2835_spi_writenb(data, 3);
  data[0] = 0x0B;
  bcm2835_spi_writenb(data, 3);
  data[0] = 0x0C;
  bcm2835_spi_writenb(data, 3);
  data[0] = 0x0D;
  bcm2835_spi_writenb(data, 3);
  
  // DACPWDWN  data  // Power-up each single DAC channel
  data[0] = 0x09;
  data[1] = 0x00;
  data[2] = 0x00;
  bcm2835_spi_writenb(data, 3);

  read_data[0] = 0;
  read_data[1] = 0;
  read_data[2] = 0;

  // Check SPICONFIG data
  data[0] = 0x83;
  data[1] = 0;
  data[2] = 0;
  bcm2835_spi_writenb(data,3);
  bcm2835_spi_transfernb(data,read_data,3);
  QLOG_INFO() << "Read SPICONFIG data:" << " " << QString::number(read_data[0])
<< QString::number(read_data[1]) << QString::number(read_data[2]);

  // Check DACPWDWN data
  data[0] = 0x89;
  data[1] = 0;
  data[2] = 0;
  bcm2835_spi_writenb(data,3);
  bcm2835_spi_transfernb(data,read_data,3);
  QLOG_INFO() << "Read DACPWDWN data:" << " " << QString::number(read_data[0])
<< QString::number(read_data[1]) << QString::number(read_data[2]);

  // Check GENCONFIG data
  data[0] = 0x84;
  data[1] = 0;
  data[2] = 0;
  bcm2835_spi_writenb(data,3);
  bcm2835_spi_transfernb(data,read_data,3);
  QLOG_INFO() << "Read GENCONFIG data:" << " " << QString::number(read_data[0])
<< QString::number(read_data[1]) << QString::number(read_data[2]);

  // Check BRDCONFIG data
  data[0] = 0x85;
  data[1] = 0;
  data[2] = 0;
  bcm2835_spi_writenb(data,3);
  bcm2835_spi_transfernb(data,read_data,3);
  QLOG_INFO() << "Read BRDCONFIG data:" << " " << QString::number(read_data[0])
<< QString::number(read_data[1]) << QString::number(read_data[2]);

  // Check BRDCAST data
  data[0] = 0x0F;
  data[1] = 0;
  data[2] = 0;
  bcm2835_spi_writenb(data,3);
  bcm2835_spi_transfernb(data,read_data,3);
  QLOG_INFO() << "Read BRDCAST data:" << " " << QString::number(read_data[0])
<< QString::number(read_data[1]) << QString::number(read_data[2]);

  // Check SYNCCONFIG data
  data[0] = 0x86;
  data[1] = 0;
  data[2] = 0;
  bcm2835_spi_writenb(data,3);
  bcm2835_spi_transfernb(data,read_data,3);
  QLOG_INFO() << "Read SYNCCONFIG data:" << " " << QString::number(read_data[0])
<< QString::number(read_data[1]) << QString::number(read_data[2]);

  // Check TOGGCONFIG0 data
  data[0] = 0x87;
  data[1] = 0;
  data[2] = 0;
  bcm2835_spi_writenb(data,3);
  bcm2835_spi_transfernb(data,read_data,3);
  QLOG_INFO() << "Read TOGGCONFIG0 data:" << " " << QString::number(read_data[0])
<< QString::number(read_data[1]) << QString::number(read_data[2]);

  // Check TRIGGER data
  data[0] = 0x0E;
  data[1] = 0;
  data[2] = 0;
  bcm2835_spi_writenb(data,3);
  bcm2835_spi_transfernb(data,read_data,3);
  QLOG_INFO() << "Read TRIGGER data:" << " " << QString::number(read_data[0])
<< QString::number(read_data[1]) << QString::number(read_data[2]);

  if (data) { free(data); data = NULL;}
  if (read_data) { free(read_data); read_data = NULL;}
  int newdevice = 1;
  device.push_back(newdevice);
  QLOG_INFO ( ) << "RaspiDac configuration success";
  connectSuccess.replace(index, true);
  // Update control widget
  emit getOutputs(outputs.at(index));
  emit getDescription(description);
  // Get registered values from Db
  QStringList raspivaluesStringList;
  raspivaluesStringList = raspivaluesString.split(" ",QString::SkipEmptyParts);
  for (int i = 0 ; i < raspivaluesStringList.size(); i++ )
    raspivalues.at(index)->push_back(raspivaluesStringList.at(i).toFloat());
  emit getOutputValues((void*)raspivalues.at(index));

  // Reset and configure raspi device
  connectSuccess.replace(index,resetRaspi(newraspi));

  return connectSuccess.at(index);
}
bool
RaspiDac::resetRaspi(QString newraspi) {
 
  // Prepare data
  char* data;
  char* read_data;
  data = (char*)malloc(3 * sizeof(char));
  read_data = (char*)malloc(3 * sizeof(char));

  read_data[0] = 0;
  read_data[1] = 0;
  read_data[2] = 0;

  // TRIGGER data  // Soft-reset Device
  data[0] = 0x0E;
  data[1] = 0x00;
  data[2] = 0x0A;
  //bcm2835_spi_writenb(data, 3);

  // SPICONFIG data  // Power-up Device, Disable Soft-toggle
  data[0] = 0x03;
  data[1] = 0x0A;
  data[2] = 0x84;
  bcm2835_spi_writenb(data, 3);

  // DACPWDWN  data  // Power-up each single DAC channel
  data[0] = 0x09;
  data[1] = 0x00;
  data[2] = 0x00;
  bcm2835_spi_writenb(data, 3);

  // Check SPICONFIG data
  data[0] = 0x83;
  data[1] = 0;
  data[2] = 0;
  bcm2835_spi_writenb(data,3);
  bcm2835_spi_transfernb(data,read_data,3);
  QLOG_INFO() << "Read SPICONFIG data:" << " " << QString::number(read_data[0])
<< QString::number(read_data[1]) << QString::number(read_data[2]);

  // Check DACPWDWN data
  data[0] = 0x89;
  data[1] = 0;
  data[2] = 0;
  bcm2835_spi_writenb(data,3);
  bcm2835_spi_transfernb(data,read_data,3);
  QLOG_INFO() << "Read DACPWDWN data:" << " " << QString::number(read_data[0])
<< QString::number(read_data[1]) << QString::number(read_data[2]);
  
  return true;
}
bool
RaspiDac::setRaspiValue(QString newraspi, int output, void *value) {

  int maxdata;
 
  // Convert value to expected type
  double *dvalue;
  dvalue = (double*) value;
  
  for (int index = 0 ; index < raspi.size(); index++)  {
    if ( raspi.at(index) == newraspi && connectSuccess.at(index) == true) {
        if ( *dvalue < min.at(index) || *dvalue > max.at(index)) {
          emit showWarning(tr("%1: channel %2:value out of range").arg(newraspi,
   				QString::number(output)));
          return false;
        }
        QLOG_INFO() << "RaspiDac::setRaspiValue> replace at " << output
                    << " old value " << QString::number(raspivalues.at(index)->at(output))
                    << " new value " << QString::number(*dvalue);
        uint16_t CODE = 0;
        CODE = (*dvalue - min.at(index)) / (max.at(index) - min.at(index))*pow(2,16);
        QLOG_INFO() << " CODE = " << CODE;
        // Set corresponding DAC output register to CODE value
         char *data;
         char *read_data;
         data = (char*)malloc(3 * sizeof(char));
         read_data = (char*)malloc(3 * sizeof(char));
         // DACn data  // Set DAC value
         if ( output == 0 )
            data[0] = 0x10;
         if ( output == 1 )
            data[0] = 0x11;
         if ( output == 2 )
            data[0] = 0x12;
         if ( output == 3 )
            data[0] = 0x13;
         if ( output == 4 )
            data[0] = 0x14;
         if ( output == 5 )
            data[0] = 0x15;
         if ( output == 6 )
            data[0] = 0x16;
         if ( output == 7 )
            data[0] = 0x17;
         if ( output == 8 )
            data[0] = 0x18;
         if ( output == 9 )
            data[0] = 0x19;
         if ( output == 10 )
            data[0] = 0x1A;
         if ( output == 11 )
            data[0] = 0x1B;
         if ( output == 12 )
            data[0] = 0x1C;
         if ( output == 13 )
            data[0] = 0x1D;
         if ( output == 14 )
            data[0] = 0x1E;
         if ( output == 15 )
            data[0] = 0x1F;
         data[1] = MSB(CODE);
         data[2] = LSB(CODE);
         QByteArray lsb(&data[1], 1);
         QByteArray msb(&data[2], 1);
         QLOG_INFO() << " LSB = " << QString(lsb.toHex()) << " MSB = "  << QString(msb.toHex());
         bcm2835_spi_writenb(data, 3);

         if (data) { free(data); data = NULL;}
         if (read_data) { free(read_data); read_data = NULL;}

         // Read back new DAC register value
         double real_value;
         getRaspiValue(newraspi, output, real_value);
         raspivalues.at(index)->replace(output,real_value);
         return true;
    }
  }
  emit showWarning(tr("Check connection to %1").arg(newraspi));
  return false; 
}
bool
RaspiDac::getRaspiValue(QString newraspi, int output, double &value) { // read value from board directly
  int maxdata;
  for (int index = 0 ; index < raspi.size(); index++)  {
    if ( raspi.at(index) == newraspi ) {
       QLOG_INFO ( ) << "RaspiDac::getRaspiValue> read device " << newraspi  
                    << ":channel " << output << ":value" << QString::number(value);
         // Read DAC register value
         char* data;
         char* read_data;
         data = (char*)malloc(3 * sizeof(char));
         read_data = (char*)malloc(3 * sizeof(char));

         // DACn data  // Read DAC value
         if ( output == 0 )
            data[0] = 0x90;
         if ( output == 1 )
            data[0] = 0x91;
         if ( output == 2 )
            data[0] = 0x92;
         if ( output == 3 )
            data[0] = 0x93;
         if ( output == 4 )
            data[0] = 0x94;
         if ( output == 5 )
            data[0] = 0x95;
         if ( output == 6 )
            data[0] = 0x96;
         if ( output == 7 )
            data[0] = 0x97;
         if ( output == 8 )
            data[0] = 0x98;
         if ( output == 9 )
            data[0] = 0x99;
         if ( output == 10 )
            data[0] = 0x9A;
         if ( output == 11 )
            data[0] = 0x9B;
         if ( output == 12 )
            data[0] = 0x9C;
         if ( output == 13 )
            data[0] = 0x9D;
         if ( output == 14 )
            data[0] = 0x9E;
         if ( output == 15 )
            data[0] = 0x9F;
         data[1] = 0;
         data[2] = 0;
         //bcm2835_spi_writenb(data, 3);
         bcm2835_spi_transfernb(data, read_data, 3);
         QLOG_INFO() << "Read DAC channel data:" << " " << QString::number(read_data[0]) << QString::number(read_data[1]) << QString::number(read_data[2]);
         uint16_t CODE = 0 ;
         typedef union
         {
           uint16_t CODE;
           char bytedata[2];
         }  ByteUInt16Conv; 
         ByteUInt16Conv convert;

         convert.bytedata[0] = read_data[2];
         convert.bytedata[1] = read_data[1]; 

         // Convert register CODE to value
         value = ((convert.CODE / pow(2,16)) * (max.at(0) - min.at(0))) + min.at(0);

         QLOG_INFO() << " converted value = " << value;

         if (data) { free(data); data = NULL;}
         if (read_data) { free(read_data); read_data = NULL;}

       return true;
    }
  }
 return false;
}

// function : update DB with RaspiDac value
bool
RaspiDac::updateDBValues(QString newraspi) {
  
  for (int index = 0 ; index < raspi.size(); index++)  {
    if ( raspi.at(index) == newraspi ) {
      QLOG_DEBUG () << " Update DB RASPI DAC Values";
      QSqlDatabase db = connectDb(path);
      QSqlQuery query(db);
      // Set new values in Db
      QString raspivaluesString = "";
      QString valueString;
      for (int i = 0 ; i < outputs.at(index); i++) {
	valueString.setNum (raspivalues.at(index)->at(i),'f',4);
	raspivaluesString = raspivaluesString + valueString + " ";
      }
      query.prepare("update raspi_settings set raspivalues = ? where name = ?");
      query.addBindValue(raspivaluesString);
      query.addBindValue(raspi.at(index));
      query.exec();
      
      // Update control widget
      emit getOutputValues(raspivalues.at(index));
      return  true;
    }
  }
  emit showWarning(tr("Cannot find raspi %1").arg(newraspi));
  return false; 
}
// function : create connexion to the database
void 
RaspiDac::dbConnexion() {

  path.append(QDir::separator()).append("raspidac.db3");
  path = QDir::toNativeSeparators(path);
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",path);
  QLOG_INFO ( ) << "RaspiDac::dbConnexion> Db path : " << path;
  db.setDatabaseName(path);  
  if ( !db.open() ) {
    QLOG_WARN ( ) << db.lastError().text();
    emit showWarning(db.lastError().text());
  }
  // Create raspi tables
  QSqlQuery query(db);
  query.exec("create table raspi_settings "
	     "(name varchar(128) not null primary key, "
	     "settings varchar(255), "
	     "description varchar(128), "
	     "raspivalues varchar(255))");
  QLOG_DEBUG ( ) << query.lastError().text();      
  
}
#endif
