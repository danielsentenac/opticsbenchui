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
#ifdef COMEDICOUNTER
#ifndef COMEDICOUNTER_H
#define COMEDICOUNTER_H

#include <comedilib.h>
#include "Comedi.h"
#include "Utils.h"

/// \ingroup dac
/// Comedi counter implementation.
class ComediCounter : public Comedi
{
  Q_OBJECT
    
public:
  /// Create a Comedi counter using the given DB path.
  /// \param dbpath Filesystem path to the Comedi DB.
  explicit ComediCounter(QString dbpath);
  /// Destructor.
  ~ComediCounter() override;

  /// Connect to the Comedi counter device.
  /// \param newcomedi Device identifier from the database.
  bool connectComedi(QString newcomedi) override;
  /// Reset the counter device.
  /// \param newcomedi Device identifier from the database.
  bool resetComedi(QString newcomedi) override;
  /// Set a counter value or mode (device-specific).
  /// \param newcomedi Device identifier from the database.
  /// \param output Output channel index.
  /// \param value Pointer to a device-specific value.
  bool setComediValue(QString newcomedi, int output, void *value) override;
  /// Read a counter value.
  /// \param newcomedi Device identifier from the database.
  /// \param output Output channel index.
  /// \param value Output value in engineering units.
  bool getComediValue(QString newcomedi, int output, double &value) override;
  /// Sync or update database-backed settings.
  /// \param newcomedi Device identifier from the database.
  bool updateDBValues(QString newcomedi) override;
  /// Set the database path used by this device.
  /// \param path Filesystem path to the Comedi DB.
  void setDbPath(QString path) override;

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
  /// Emit output values (device-specific type).
  /// \param comedivalues Pointer to device-specific values.
  void getOutputValues(void *comedivalues);

private:

  int reset_counter(comedi_t *device, unsigned subdevice);
  int arm(comedi_t *device, unsigned subdevice, lsampl_t source);
  int set_counter_mode(comedi_t *device, unsigned subdevice, lsampl_t mode_bits);
  int set_clock_source(comedi_t *device, unsigned subdevice, lsampl_t clock, lsampl_t period_ns);
  int set_gate_source(comedi_t *device, unsigned subdevice, lsampl_t gate_index, lsampl_t gate_source);
  int start_up_down_counting(comedi_t *device, unsigned subdevice);
  int stop_up_down_counting(comedi_t *device, unsigned subdevice);
  long getTime(comedi_t *device);

  void dbConnexion() override;
  QSqlDatabase connectDb(QString path) {
     return Utils::ConnectSqliteDb(path, "ComediCounter::dbConnexion>");
  }
  QVector<comedi_t*>  device;          /**< The comedi device file descriptor*/
  QVector<QString> comediSettings;
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
