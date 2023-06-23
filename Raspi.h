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

#ifndef RASPI_H
#define RASPI_H

#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"

class Raspi : public QObject
{
   Q_OBJECT   
   
    public:

  virtual bool connectRaspi(QString newraspi) = 0;
  virtual bool resetRaspi(QString newraspi) = 0;
  virtual bool setRaspiValue(QString newraspi, int output, void *value) = 0;
  virtual bool getRaspiValue(QString newraspi, int output, double &value) = 0;
  virtual bool updateDBValues(QString newraspi) = 0;
  virtual void setDbPath(QString _path) = 0;
  
  QString path;
  QString raspitype;

  public slots:

 signals:
  void getDescription(QString description);
  void showWarning(QString message);
  void getOutputs(int outputs);
  void getOutputValues(void *raspivalues);

 protected:
   virtual void dbConnexion() = 0;
  
};
#endif
