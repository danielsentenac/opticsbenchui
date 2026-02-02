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
#ifdef ADVANTECHDAC
#ifndef ADVANTECHDAC_H
#define ADVANTECHDAC_H

#include <Advantech/advdevice.h>
#include <Advantech/advdaq.h>
#include "Dac.h"

#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"
#include "Utils.h"

class DacAdvantech : public Dac
{
  Q_OBJECT
    
    public:
  DacAdvantech(QString dbpath);
  ~DacAdvantech();

  bool connectDac(QString newdac);
  bool resetDac(QString newdac);
  bool setDacValue(QString newdac, int output, double value);
  bool setDacRValue(QString newdac, int output, double rvalue);
  float getDacValue(QString newdac,int output);
  bool updateDBValues(QString newdac);
  void setDbPath(QString _path);
  
  public slots:

 signals:
  void getDescription(QString description);
  void showWarning(QString message);
  void getOutputs(int outputs,QString);
  void getOutputValues(QVector<float> *dacvalues);

 private:

  void dbConnexion();
  QSqlDatabase connectDb(QString path) {
     return Utils::ConnectSqliteDb(path, "DacAdvantech::dbConnexion>");
  }

  QVector<PTR_T*>  fd;                /**< The device file descriptor*/
  QVector<PT_AOConfig*> config;       /**< Configurator*/
  QVector<PT_AOVoltageOut*> chOutV;   /**< The voltage value structure */
  QVector<PT_AOCurrentOut*> chOutC;   /**< The current value structure */
  QVector<AORANGESET*> ao_chan_range; /**< The channel property structure*/
  QVector<QString> dacSettings;
  QVector<QString> dac;
  QVector<QString> mode;
  QVector<float> min,max;
  QVector<int> outputs;
  QVector<QString> fname;
  QVector<bool>  connectSuccess;
  QVector< QVector<float>* > dacvalues;
};
#endif
#endif
