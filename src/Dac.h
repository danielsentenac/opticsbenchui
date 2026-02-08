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
 * @file Dac.h
 * @brief Dac device interface.
 */

#ifndef DAC_H
#define DAC_H

#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"

/// \ingroup dac
/// Abstract interface for DAC devices.
///
/// Concrete implementations handle device-specific connections and
/// database-backed settings for DAC outputs.
class Dac : public QObject
{
  Q_OBJECT
    
public:
  /// Virtual destructor.
  ~Dac() override = default;

  /// Connect to a DAC by name.
  /// \param newdac DAC identifier from the database.
  virtual bool connectDac(QString newdac) = 0;
  /// Reset the DAC to a known state.
  /// \param newdac DAC identifier from the database.
  virtual bool resetDac(QString newdac) = 0;
  /// Set the output value for a DAC channel.
  /// \param newdac DAC identifier from the database.
  /// \param output Output channel index.
  /// \param value Value in engineering units.
  virtual bool setDacValue(QString newdac, int output, double value) = 0;
  /// Set the output value using a raw device value.
  /// \param newdac DAC identifier from the database.
  /// \param output Output channel index.
  /// \param rvalue Raw device value.
  virtual bool setDacRValue(QString newdac, int output, double rvalue) = 0;
  /// Read the current DAC output value.
  /// \param newdac DAC identifier from the database.
  /// \param output Output channel index.
  virtual float getDacValue(QString newdac,int output) = 0;
  /// Sync or update database-backed settings.
  /// \param newdac DAC identifier from the database.
  virtual bool updateDBValues(QString newdac) = 0;
  /// Set the database path used by this device.
  /// \param path Filesystem path to the DAC SQLite DB.
  virtual void setDbPath(QString path) = 0;
  
  // parameters
  /// Path to the DAC database.
  QString path;

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
  /// Emit current output values.
  /// \param dacvalues Pointer to output values vector.
  void getOutputValues(QVector<float> *dacvalues);

protected:
   /// Create or refresh the device database connection.
   virtual void dbConnexion() = 0;

};
#endif
