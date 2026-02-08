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

/**
 * @file RaspiDac.h
 * @brief Raspi Dac source file.
 */
#ifdef RASPIDAC
#ifndef RASPIDAC_H
#define RASPIDAC_H

//#include <wiringPiSPI.h>
#include <bcm2835.h>
#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"
#include "Utils.h"

/// \ingroup dac
/// Raspberry Pi DAC implementation.
class RaspiDac : public Raspi
{
   Q_OBJECT 
     
    public:
  /// Create a Raspi DAC using the given DB path.
  /// \param dbpath Filesystem path to the Raspi DB.
  explicit RaspiDac(QString dbpath);
  /// Destructor.
  ~RaspiDac();

  /// Connect to the Raspi DAC device.
  /// \param newraspi Device identifier from the database.
  bool connectRaspi(QString newraspi);
  /// Reset the DAC device.
  /// \param newraspi Device identifier from the database.
  bool resetRaspi(QString newraspi);
  /// Set a DAC output value (device-specific).
  /// \param newraspi Device identifier from the database.
  /// \param output Output channel index.
  /// \param value Pointer to a device-specific value.
  bool setRaspiValue(QString newraspi, int output, void *value);
  /// Read a DAC output value.
  /// \param newraspi Device identifier from the database.
  /// \param output Output channel index.
  /// \param value Output value in engineering units.
  bool getRaspiValue(QString newraspi, int output, double &value);
  /// Sync or update database-backed settings.
  /// \param newraspi Device identifier from the database.
  bool updateDBValues(QString newraspi);
  /// Set the database path used by this device.
  /// \param _path Filesystem path to the Raspi DB.
  void setDbPath(QString _path);
  
  QString path;
  QString raspitype;

  public slots:

 signals:
  /// Emit a human-readable description of the device.
  /// \param description Device description string.
  void getDescription(QString description);
  /// Emit warnings to the UI.
  /// \param message Warning message.
  void showWarning(QString message);
  /// Emit output count.
  /// \param outputs Number of outputs.
  void getOutputs(int outputs);
  /// Emit output values (device-specific type).
  /// \param raspivalues Pointer to device-specific values.
  void getOutputValues(void *raspivalues);

 private:
  
  void dbConnexion();
  QSqlDatabase connectDb(QString path) {
     return Utils::ConnectSqliteDb(path, "RaspiDac::dbConnexion>");
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
