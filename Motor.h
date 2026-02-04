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
 * @file Motor.h
 * @brief Motor device interface.
 */

#ifndef MOTOR_H
#define MOTOR_H

#include "ACCom.h"
#include "Driver.h"

#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"
#include "Utils.h"

/// \ingroup motors
/// Controls and queries motorized actuators defined in the database.
///
/// This class provides a thin UI-facing API for connecting to motors,
/// issuing motion commands, and reading positions. It also emits Qt
/// signals for UI updates and warnings.
class Motor : public QObject
{
  Q_OBJECT
    
    public:
  /// Create a motor controller.
  /// \param parent QObject parent.
  /// \param _appDirPath Application directory for locating databases.
  Motor(QObject* parent = 0, QString _appDirPath = 0);
  /// Destructor.
  ~Motor();

  /// Connect to a motorized actuator by name.
  /// \param newactuator Actuator identifier from the database.
  void  connectMotor(QString newactuator);
  /// Move an actuator forward by a relative distance.
  /// \param newactuator Actuator identifier from the database.
  /// \param motion Relative motion amount (device units).
  void  moveForward(QString newactuator,float motion);
  /// Move an actuator backward by a relative distance.
  /// \param newactuator Actuator identifier from the database.
  /// \param motion Relative motion amount (device units).
  void  moveBackward(QString newactuator,float motion);
  /// Move an actuator to an absolute position.
  /// \param newactuator Actuator identifier from the database.
  /// \param motion Absolute position (device units).
  void  moveAbsolute(QString newactuator,float motion);
  /// Stop the current actuator motion.
  /// \param newactuator Actuator identifier from the database.
  void  stopMotor(QString newactuator);
  /// Set the path to the motor database.
  /// \param _path Filesystem path to the motor SQLite DB.
  void  setDbPath(QString _path);
  /// Return nonzero when the actuator operation is complete.
  /// \param newactuator Actuator identifier from the database.
  int   getOperationComplete(QString newactuator);
  /// Read current actuator position.
  /// \param newactuator Actuator identifier from the database.
  float getPosition(QString newactuator);
  
  // parameters
  /// Per-actuator completion flags.
  QVector<int> operationcomplete;
  /// Path to the motor database.
  QString path;

 signals:
  /// Emit the current position for display.
  /// \param position Position value (device units).
  void getPosition(float position);
  /// Emit a human-readable description of the actuator.
  /// \param description Actuator description string.
  void getDescription(QString description);
  /// Emit warnings to the UI.
  /// \param message Warning message.
  void showWarning(QString message);
  /// Request timers to stop (UI coordination).
  void stopTimer();

 public slots:
  /// Slot called when a motor operation completes.
  void operationComplete();
  

 private:

  // function : create connexion to the database
  void dbConnexion();
  QSqlDatabase connectDb(const QString& path);

  QVector<ACCom*>   actuatorCom;
  QVector<Driver*>  actuatorDriver;
  QVector<QString>  actuatorSettings;
  QVector<QString>  description;
  QVector<QString>  com;
  QVector<QString>  driver;
  QVector<QString>  actuator;
  bool iscompleting;
  QVector<bool> connectSuccess;
  QVector<float> position;
  QString runningActuator;
};
#endif
