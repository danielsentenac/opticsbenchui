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

#ifndef COMEDI_H
#define COMEDI_H

#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"

class Comedi : public QObject
{
  Q_OBJECT
    
public:
  ~Comedi() override = default;

  virtual bool connectComedi(QString newcomedi) = 0;
  virtual bool resetComedi(QString newcomedi) = 0;
  virtual bool setComediValue(QString newcomedi, int output, void *value) = 0;
  virtual bool getComediValue(QString newcomedi, int output, double &value) = 0;
  virtual bool updateDBValues(QString newcomedi) = 0;
  virtual void setDbPath(QString path) = 0;
  
  // parameters
  QString path;
  QString comeditype;

public slots:

signals:
  void getDescription(QString description);
  void showWarning(QString message);
  void getOutputs(int outputs,QString);
  void getOutputValues(void *comedivalues);

protected:
   virtual void dbConnexion() = 0;

};
#endif
