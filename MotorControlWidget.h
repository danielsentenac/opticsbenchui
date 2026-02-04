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
 * @file MotorControlWidget.h
 * @brief Motor control widget.
 */

#ifndef MOTORCONTROLWIDGET_H
#define MOTORCONTROLWIDGET_H

#include "Motor.h"

#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"

/// \ingroup ui
/// UI widget for motor control.
class MotorControlWidget : public QWidget
{
  Q_OBJECT
    
    public:
  /// Construct a motor control widget.
  /// \param _actuatorList List of actuator names (optional).
  MotorControlWidget(QVector<QString>  *_actuatorList = 0);
  /// Destructor.
  ~MotorControlWidget();

  /// Attach the motor controller.
  /// \param _motor Motor controller instance.
  void setMotor(Motor *_motor);
  /// Set the list of available actuators.
  /// \param _actuatorList List of actuator names.
  void setActuatorList(QVector<QString>  *_actuatorList);
  
  private slots:
  /// Connect to the selected motor.
  void connectMotor();
  /// Move forward by a relative amount.
  void moveForward();
  /// Move backward by a relative amount.
  void moveBackward();
  /// Move to an absolute position.
  void moveAbsolute();
  /// Stop the motor.
  void stopMotor();
  /// Update the current position display.
  /// \param position Current position.
  void getPosition(float position);
  /// Update the description display.
  /// \param description Actuator description.
  void getDescription(QString description);
  /// Stop periodic updates.
  void stopTimer();

 private:

  QTimer    *timer;
  QPushButton *connectButton; 
  QPushButton *forwardButton;
  QPushButton *backwardButton;
  QPushButton *absButton;
  QPushButton *stopButton;
  QLineEdit   *to_position;
  QLabel      *to_positionLabel;
  QLabel      *cur_positionLabel;
  QLineEdit   *cur_position;
  QComboBox   *actuatorCombo;
  QLabel      *descriptionLabel;
  QVector<QString> *actuatorList;
  Motor *motor;
};
#endif
