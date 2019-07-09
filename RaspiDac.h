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
#ifndef RASPIDAC_H
#define RASPIDAC_H

//#include <wiringPiSPI.h>
#include <bcm2835.h>
#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"

class RaspiDac : public QObject
{
   Q_OBJECT   
    public:
  RaspiDac(QString dbpath);
  ~RaspiDac();

  bool connectRaspi(QString newraspi);
  bool resetRaspi(QString newraspi);
  bool setRaspiValue(QString newraspi, int output, void *value);
  bool getRaspiValue(QString newraspi, int output, double &value);
  bool updateDBValues(QString newraspi);
  void setDbPath(QString _path);
  
  QString path;
  QString raspitype;

  public slots:

 signals:
  void getDescription(QString description);
  void showWarning(QString message);
  void getOutputs(int outputs);
  void getOutputValues(void *raspivalues);

 private:
  
  void dbConnexion();
  QSqlDatabase connectDb(QString path) {
     QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",path);
     QLOG_INFO ( ) << "ComediDac::dbConnexion> Db path : " << path;
     db.setDatabaseName(path);
     if ( !db.open() ) {
       QLOG_WARN ( ) << db.lastError().text();
     }
     return db;
  }
  QVector<int>  device;          /**< The raspi device file descriptor*/
  QVector<QString> raspiSettings;
  QVector<int> subdev;
  QVector<QString> raspi;
  QVector<QString> mode;
  QVector<float> min,max;
  QVector<int> outputs;
  QVector<QString> fname;
  QVector<bool>  connectSuccess;
  QVector< QVector<double>* > raspivalues;
};
#endif
#endif
