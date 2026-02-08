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
 * @file Comedi.h
 * @brief Comedi device interface.
 */

#ifndef COMEDI_H
#define COMEDI_H

#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"

/// \ingroup dac
/// Abstract interface for Comedi-based devices (counter/DAC).
class Comedi : public QObject
{
  Q_OBJECT
    
public:
  /// Virtual destructor.
  ~Comedi() override = default;

  /// Connect to a Comedi device by name.
  /// \param newcomedi Device identifier from the database.
  virtual bool connectComedi(QString newcomedi) = 0;
  /// Reset the Comedi device to a known state.
  /// \param newcomedi Device identifier from the database.
  virtual bool resetComedi(QString newcomedi) = 0;
  /// Set an output value (type depends on device).
  /// \param newcomedi Device identifier from the database.
  /// \param output Output channel index.
  /// \param value Pointer to a device-specific value.
  virtual bool setComediValue(QString newcomedi, int output, void *value) = 0;
  /// Read an output/input value from the device.
  /// \param newcomedi Device identifier from the database.
  /// \param output Output channel index.
  /// \param value Output value in engineering units.
  virtual bool getComediValue(QString newcomedi, int output, double &value) = 0;
  /// Sync or update database-backed settings.
  /// \param newcomedi Device identifier from the database.
  virtual bool updateDBValues(QString newcomedi) = 0;
  /// Set the database path used by this device.
  /// \param path Filesystem path to the Comedi SQLite DB.
  virtual void setDbPath(QString path) = 0;
  
  // parameters
  /// Path to the Comedi database.
  QString path;
  /// Comedi device type identifier.
  QString comeditype;

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
  /// \param mode Device name or mode string.
  void getOutputs(int outputs, QString mode);
  /// Emit output values (device-specific type).
  /// \param comedivalues Pointer to device-specific values.
  void getOutputValues(void *comedivalues);

protected:
   /// Create or refresh the device database connection.
   virtual void dbConnexion() = 0;

};
#endif
