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
 * @file SuperK.h
 * @brief Super K device interface.
 */

#ifndef SUPERK_H
#define SUPERK_H

#include "DriverSuperK.h"

#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"
#include "Utils.h"

/// \ingroup lasers
/// Controls a SuperK laser source via its driver.
class SuperK : public QObject
{
  Q_OBJECT
    
    public:
  /// Create a SuperK controller.
  /// \param parent QObject parent.
  /// \param _appDirPath Application directory for locating databases.
  SuperK(QObject* parent = 0, QString _appDirPath = 0);
  /// Destructor.
  ~SuperK();

  /// Connect to a SuperK driver by name.
  /// \param newdriver Driver identifier from the database.
  void  connectSuperK(QString newdriver);
  /// Set the output power.
  /// \param newdriver Driver identifier from the database.
  /// \param power Power value (device units).
  void  setPower(QString newdriver, int power);
  /// Set the neutral density (ND) value.
  /// \param newdriver Driver identifier from the database.
  /// \param nd ND value.
  void  setNd(QString newdriver, int nd);
  /// Set the short-wavelength pass (SWP) value.
  /// \param newdriver Driver identifier from the database.
  /// \param swp SWP value.
  void  setSwp(QString newdriver, int swp);
  /// Set the long-wavelength pass (LWP) value.
  /// \param newdriver Driver identifier from the database.
  /// \param lwp LWP value.
  void  setLwp(QString newdriver, int lwp);
  /// Set the path to the SuperK database.
  /// \param _path Filesystem path to the SuperK SQLite DB.
  void  setDbPath(QString _path);
  /// Return nonzero when the driver operation is complete.
  /// \param newdriver Driver identifier from the database.
  int   getOperationComplete(QString newdriver);
  /// Read current power setting.
  /// \param newdriver Driver identifier from the database.
  int   getPower(QString newdriver);
  /// Read current ND setting.
  /// \param newdriver Driver identifier from the database.
  int   getNd(QString newdriver);
  /// Read current SWP setting.
  /// \param newdriver Driver identifier from the database.
  int   getSwp(QString newdriver);
  /// Read current LWP setting.
  /// \param newdriver Driver identifier from the database.
  int   getLwp(QString newdriver);

  // parameters
  /// Per-driver completion flags.
  QVector<int> operationcomplete;
  /// Path to the SuperK database.
  QString path;

signals:
  /// Emit current power value.
  /// \param power Power value.
  void getPower(int power);
  /// Emit current ND value.
  /// \param nd ND value.
  void getNd(int nd);
  /// Emit current SWP value.
  /// \param swp SWP value.
  void getSwp(int swp);
  /// Emit current LWP value.
  /// \param lwp LWP value.
  void getLwp(int lwp);
  /// Emit a human-readable description of the device.
  /// \param description Device description string.
  void getDescription(QString description);
  /// Emit warnings to the UI.
  /// \param message Warning message.
  void showWarning(QString message);
  /// Request timers to stop (UI coordination).
  void stopTimer();

 public slots:
  /// Slot called when a driver operation completes.
  void operationComplete();
  

 private:

  // function : create connexion to the database
  void dbConnexion();
  QSqlDatabase connectDb(QString path) {
     return Utils::ConnectSqliteDb(path, "SuperK::dbConnexion>");
  }
  QVector<ACCom*>   driverCom;
  QVector<DriverSuperK*>  driverSuperK;
  QVector<QString>  driverSettings;
  QVector<QString>  description;
  QVector<QString>  driver;
  QVector<QString>  data;
  bool iscompleting;
  QVector<bool> connectSuccess;
  QString runningDriver;
};
#endif
