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

#ifndef DAC_H
#define DAC_H

#include <Advantech/advdevice.h>
#include <Advantech/advdaq.h>

#include <QtSql>
#include <QtGui>
#include "QsLog.h"

class Dac : public QObject
{
  Q_OBJECT
    
    public:
  Dac(QObject *parent = 0, QString _appDirPath = 0);
  ~Dac();

  bool connectDac(QString newdac);
  bool resetDac(QString newdac);
  bool setDacValue(int output, float value);
  void setDbPath(QString _path);
   // parameters
  QString path;

  public slots:

 signals:
  void getDescription(QString description);
  void showWarning(QString message);
  void getOutputs(int outputs,QString);
  void getOutputValues(QVector<float> *dacvalues);

 private:

  void dbConnexion();

  PTR_T                fd;         /**< The device file descriptor*/
  PT_AOConfig	   config;         /**< Configurator*/
  PT_AOVoltageOut    chOutV;        /**< The voltage value structure*/
  PT_AOCurrentOut    chOutC;        /**< The current value structure*/
  AORANGESET ao_chan_range;        /**< The channel property structure*/
  QString   dacSettings;
  QString   dac;
  QString mode;
  float min,max;
  int outputs;
  QString fname;
  bool    connectSuccess;
  QVector<float> *dacvalues;
};
#endif
