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

#include "MotorControlWidget.h"

MotorControlWidget::MotorControlWidget(QVector<QString>  *_actuatorList)
 
{
  actuatorList = _actuatorList;
  
  QGridLayout *layout = new QGridLayout(this);
  connectButton = new  QPushButton("Connect");
  connect( connectButton, SIGNAL(clicked()), this, SLOT(connectMotor()));
  forwardButton = new  QPushButton("Forward");
  connect( forwardButton, SIGNAL(clicked()), this, SLOT(moveForward()));
  backwardButton = new  QPushButton("Backward");
  connect( backwardButton, SIGNAL(clicked()), this, SLOT(moveBackward()));
  absButton= new  QPushButton("Absolute");
  connect( absButton, SIGNAL(clicked()), this, SLOT(moveAbsolute()));
  stopButton= new  QPushButton("Stop");
  connect( stopButton, SIGNAL(clicked()), this, SLOT(stopMotor()));
  to_position = new QLineEdit();
  to_position->setFixedWidth(150);
  to_positionLabel = new QLabel("Go to position");
  cur_positionLabel = new QLabel("Current position");
  cur_position = new QLineEdit();
  cur_position->setFixedWidth(150);
  actuatorCombo = new QComboBox();
  actuatorCombo->setFixedWidth(150);
  descriptionLabel = new QLabel();
  
  layout->addWidget(connectButton,0,0,1,1);
  layout->addWidget(actuatorCombo,0,1,1,1);
  layout->addWidget(descriptionLabel,0,2,1,3);
  layout->addWidget(forwardButton,2,2,1,1);
  layout->addWidget(backwardButton,3,2,1,1);
  layout->addWidget(absButton,4,2,1,1);
  layout->addWidget(stopButton,5,2,1,1);
  layout->addWidget(to_position,3,1,1,1);
  layout->addWidget(to_positionLabel,3,0,1,1);
  layout->addWidget(cur_positionLabel,5,0,1,1);
  layout->addWidget(cur_position,5,1,1,1);
  
  setLayout(layout);

}
void
MotorControlWidget::getPosition(float position) {
  QString positionQString;
  positionQString.setNum (position, 'f',3);
  cur_position->setText(positionQString);
}
void
MotorControlWidget::setMotor(Motor *_motor) {
  motor = _motor;
  connect(motor,SIGNAL(getPosition(float)),this,SLOT(getPosition(float)));
  connect(motor,SIGNAL(getDescription(QString)),this,SLOT(getDescription(QString)));
  connect(motor,SIGNAL(stopTimer()),this,SLOT(stopTimer()));
  //
  // Launch a timer to get the current position of the actuator every second
  //
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), motor, SLOT(operationComplete()));
}
MotorControlWidget::~MotorControlWidget()
{
  QLOG_DEBUG ( ) << "Deleting MotorControlWidget";
}
void
MotorControlWidget::setActuatorList(QVector<QString>  *_actuatorList) {
  actuatorList = _actuatorList;
  actuatorCombo->clear();
  for (int i = 0 ; i < actuatorList->size(); i++)
    actuatorCombo->addItem(actuatorList->at(i));
}
void
MotorControlWidget::connectMotor() {
  QString actuator;
  actuator = actuatorCombo->itemText(actuatorCombo->currentIndex());
  motor->connectMotor(actuator);
}
void
MotorControlWidget::moveForward() {
    float motion = to_position->text().toFloat();
    motor->moveForward(motion);
    timer->start(1000);
}
void
MotorControlWidget::moveBackward() {
    float motion = to_position->text().toFloat();
    motor->moveBackward(motion);
    timer->start(1000);
}
void
MotorControlWidget::moveAbsolute() {
  float motion = to_position->text().toFloat();
  motor->moveAbsolute(motion);
  timer->start(1000);
 
}
void
MotorControlWidget::stopMotor() {
    motor->stopMotor();
    timer->start(1000);
}
void 
MotorControlWidget::getDescription(QString description) {
  descriptionLabel->setText(tr("Connected to : %1").arg(description));
}
void 
MotorControlWidget::stopTimer() {
  timer->stop();
}
