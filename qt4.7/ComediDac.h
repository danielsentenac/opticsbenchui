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
#ifndef COMEDIDAC_H
#define COMEDIDAC_H

#include <comedilib.h>
#include "Comedi.h"

#include <QtSql>
#include <QtGui>
#include "QsLog.h"

class ComediDac : public Comedi
{
  Q_OBJECT
    
    public:
  ComediDac(QString dbpath);
  ~ComediDac();

  bool connectComedi(QString newcomedi);
  bool resetComedi(QString newcomedi);
  bool setComediValue(QString newcomedi, int output, void *value);
  bool getComediValue(QString newcomedi, int output, double &value);
  bool updateDBValues(QString newcomedi);
  void setDbPath(QString _path);
  
  public slots:

 signals:
  void getDescription(QString description);
  void showWarning(QString message);
  void getOutputs(int outputs,QString);
  void getOutputValues(void *comedivalues);

 private:

  void dbConnexion();

  QVector<comedi_t*>  device;          /**< The comedi device file descriptor*/
  QVector<QString> comediSettings;
  QVector<int> subdev;
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
