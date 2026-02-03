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

/// \ingroup dac
/// Abstract interface for Raspberry Pi based DAC devices.
class Raspi : public QObject
{
   Q_OBJECT   
   
    public:

  /// Connect to a Raspi device by name.
  /// \param newraspi Device identifier from the database.
  virtual bool connectRaspi(QString newraspi) = 0;
  /// Reset the Raspi device to a known state.
  /// \param newraspi Device identifier from the database.
  virtual bool resetRaspi(QString newraspi) = 0;
  /// Set an output value (type depends on device).
  /// \param newraspi Device identifier from the database.
  /// \param output Output channel index.
  /// \param value Pointer to a device-specific value.
  virtual bool setRaspiValue(QString newraspi, int output, void *value) = 0;
  /// Read an output/input value from the device.
  /// \param newraspi Device identifier from the database.
  /// \param output Output channel index.
  /// \param value Output value in engineering units.
  virtual bool getRaspiValue(QString newraspi, int output, double &value) = 0;
  /// Sync or update database-backed settings.
  /// \param newraspi Device identifier from the database.
  virtual bool updateDBValues(QString newraspi) = 0;
  /// Set the database path used by this device.
  /// \param _path Filesystem path to the Raspi SQLite DB.
  virtual void setDbPath(QString _path) = 0;
  
  /// Path to the Raspi database.
  QString path;
  /// Raspi device type identifier.
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

 protected:
   /// Create or refresh the device database connection.
   virtual void dbConnexion() = 0;
  
};
#endif
