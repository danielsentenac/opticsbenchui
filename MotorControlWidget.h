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

#ifndef MOTORCONTROLWIDGET_H
#define MOTORCONTROLWIDGET_H

#include "Motor.h"

#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"

class MotorControlWidget : public QWidget
{
  Q_OBJECT
    
    public:
  MotorControlWidget(QVector<QString>  *_actuatorList = 0);
  ~MotorControlWidget();

  void setMotor(Motor *_motor);
  void setActuatorList(QVector<QString>  *_actuatorList);
  
  private slots:
  void connectMotor();
  void moveForward();
  void moveBackward();
  void moveAbsolute();
  void stopMotor();
  void getPosition(float position);
  void getDescription(QString description);
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
