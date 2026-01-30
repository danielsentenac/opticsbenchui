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
#ifndef COMEDICOUNTER_H
#define COMEDICOUNTER_H

#include <comedilib.h>
#include "Comedi.h"

class ComediCounter : public Comedi
{
  Q_OBJECT
    
public:
  explicit ComediCounter(QString dbpath);
  ~ComediCounter() override;

  bool connectComedi(QString newcomedi) override;
  bool resetComedi(QString newcomedi) override;
  bool setComediValue(QString newcomedi, int output, void *value) override;
  bool getComediValue(QString newcomedi, int output, double &value) override;
  bool updateDBValues(QString newcomedi) override;
  void setDbPath(QString path) override;

public slots:

signals:
  void getDescription(QString description);
  void showWarning(QString message);
  void getOutputs(int outputs,QString);
  void getOutputValues(void *comedivalues);

private:

  int reset_counter(comedi_t *device, unsigned subdevice);
  int arm(comedi_t *device, unsigned subdevice, lsampl_t source);
  int set_counter_mode(comedi_t *device, unsigned subdevice, lsampl_t mode_bits);
  int set_clock_source(comedi_t *device, unsigned subdevice, lsampl_t clock, lsampl_t period_ns);
  int set_gate_source(comedi_t *device, unsigned subdevice, lsampl_t gate_index, lsampl_t gate_source);
  int start_up_down_counting(comedi_t *device, unsigned subdevice);
  int stop_up_down_counting(comedi_t *device, unsigned subdevice);
  long getTime(comedi_t *device);

  void dbConnexion() override;
  QSqlDatabase connectDb(QString path) {
     QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",path);
     QLOG_INFO ( ) << "ComediDac::dbConnexion> Db path : " << path;
     db.setDatabaseName(path);
     if ( !db.open() ) {
       QLOG_WARN ( ) << db.lastError().text();
     }
     return db;
  }
  QVector<comedi_t*>  device;          /**< The comedi device file descriptor*/
  QVector<QString> comediSettings;
  QVector<QString> comedi;
  QVector<QString> mode;
  QVector<float> min,max;
  QVector<int> outputs;
  QVector<QString> fname;
  QVector<bool>  connectSuccess;
  QVector< QVector<double>* > comedivalues;
};
#endif
#endif
