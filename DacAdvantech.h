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
#ifndef DACADVANTECH_H
#define DACADVANTECH_H

#include <Advantech/advdevice.h>
#include <Advantech/advdaq.h>
#include "Dac.h"

#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"
#include "Utils.h"

/// \ingroup dac
/// Advantech DAC implementation.
class DacAdvantech : public Dac
{
  Q_OBJECT
    
    public:
  /// Create an Advantech DAC controller.
  /// \param dbpath Filesystem path to the DAC DB.
  explicit DacAdvantech(QString dbpath);
  /// Destructor.
  ~DacAdvantech();

  /// Connect to the configured DAC device.
  /// \param newdac DAC identifier from the database.
  bool connectDac(QString newdac);
  /// Reset the DAC device.
  /// \param newdac DAC identifier from the database.
  bool resetDac(QString newdac);
  /// Set a DAC channel value in engineering units.
  /// \param newdac DAC identifier from the database.
  /// \param output Output channel index.
  /// \param value Value in engineering units.
  bool setDacValue(QString newdac, int output, double value);
  /// Set a DAC channel value in raw units.
  /// \param newdac DAC identifier from the database.
  /// \param output Output channel index.
  /// \param rvalue Raw device value.
  bool setDacRValue(QString newdac, int output, double rvalue);
  /// Read the DAC channel value.
  /// \param newdac DAC identifier from the database.
  /// \param output Output channel index.
  float getDacValue(QString newdac,int output);
  /// Sync or update database-backed settings.
  /// \param newdac DAC identifier from the database.
  bool updateDBValues(QString newdac);
  /// Set the database path used by this device.
  /// \param _path Filesystem path to the DAC DB.
  void setDbPath(QString _path);
  
  public slots:

 signals:
  /// Emit a human-readable description of the device.
  /// \param description Device description string.
  void getDescription(QString description);
  /// Emit warnings to the UI.
  /// \param message Warning message.
  void showWarning(QString message);
  /// Emit output count and device name.
  /// \param outputs Number of outputs.
  /// \param QString Device name or mode string.
  void getOutputs(int outputs,QString);
  /// Emit current output values.
  /// \param dacvalues Pointer to output values.
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
