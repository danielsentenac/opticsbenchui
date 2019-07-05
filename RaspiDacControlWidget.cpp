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
#include "RaspiDacControlWidget.h"

RaspiDacControlWidget::RaspiDacControlWidget(QVector<QString>  *_raspiList)
{

  raspiList = _raspiList;
 
  layout = new QGridLayout(this);
  connectButton = new  QPushButton("Connect");
  connectButton->setFixedWidth(100);
  connect( connectButton, SIGNAL(clicked()), this, SLOT(connectRaspi()));
  
  resetButton = new  QPushButton("Reset");
  resetButton->setFixedWidth(100);
  connect( resetButton, SIGNAL(clicked()), this, SLOT(resetRaspi()));

  raspiCombo = new QComboBox();
  raspiCombo->setFixedWidth(100);
  descriptionLabel = new QLabel();

  layout->addWidget(connectButton,0,0,1,1);
  layout->addWidget(raspiCombo,0,1,1,1);
  layout->addWidget(descriptionLabel,0,2,1,5);
  layout->addWidget(resetButton,1,0,1,1);
  
  setLayout(layout);
  signalMapper = NULL;
  shiftsignalMapper = NULL;
  outputsList = new QVector<QLabel*>();
  unitsList = new QVector<QLabel*>();
  setButtonList = new QVector<QPushButton*>();
  raspivalueList = new QVector<QLineEdit*>();
  shiftButtonList = new QVector<QPushButton*>();
  raspirvalueList = new QVector<QLineEdit*>();
}
RaspiDacControlWidget::~RaspiDacControlWidget()
{
  QLOG_DEBUG ( ) << "Deleting RaspiDacControlWidget";
}
void
RaspiDacControlWidget::setRaspi(RaspiDac *_raspi) {
  raspi = _raspi;
  connect(raspi,SIGNAL(getDescription(QString)),this,SLOT(getDescription(QString)));
  connect(raspi,SIGNAL(getOutputs(int)),this,SLOT(getOutputs(int)));
  connect(raspi,SIGNAL(getOutputValues(void*)),this,SLOT(getOutputValues(void*)));
}
void
RaspiDacControlWidget::setRaspiList(QVector<QString>  *_raspiList) {
  raspiList = _raspiList;
  raspiCombo->clear();
  for (int i = 0 ; i < raspiList->size(); i++)
    raspiCombo->addItem(raspiList->at(i));
}
void
RaspiDacControlWidget::getOutputs(int outputs) {
  outputsList->clear();
  raspivalueList->clear();
  setButtonList->clear();
  raspirvalueList->clear();
  shiftButtonList->clear();
  // Create a signal mapper for buttons
  if (signalMapper != NULL) delete signalMapper;
  if (shiftsignalMapper != NULL) delete shiftsignalMapper;
  shiftsignalMapper = new QSignalMapper(this);
  signalMapper = new QSignalMapper(this);
  for (int i = 0 ; i < outputs; i++) {
    QLabel *outputLabel = new QLabel(tr("Channel %1").arg(i));
    outputLabel->setFixedWidth(100);
    QLabel *unitsLabel = new QLabel("volts");
    unitsLabel->setFixedWidth(30);
    outputsList->push_back(outputLabel);
    unitsList->push_back(unitsLabel);
    QLineEdit *raspivalue = new QLineEdit("");
    raspivalue->setFixedWidth(100);
    raspivalueList->push_back(raspivalue);
    QLineEdit *raspirvalue = new QLineEdit("");
    raspirvalue->setFixedWidth(100);
    raspirvalueList->push_back(raspirvalue);
    QPushButton *button = new QPushButton("Set Voltage");
    button->setFixedWidth(150);
    connect( button, SIGNAL(clicked()), signalMapper, SLOT(map()));
    signalMapper->setMapping(button, i);
    setButtonList->push_back(button);
    QPushButton *shiftbutton = new QPushButton("SHIFT Voltage");
    shiftbutton->setFixedWidth(150);
    connect( shiftbutton, SIGNAL(clicked()), shiftsignalMapper, SLOT(map()));
    shiftsignalMapper->setMapping(shiftbutton, i);
    shiftButtonList->push_back(shiftbutton);

    layout->addWidget(outputLabel,i + 2,0,1,1);
    layout->addWidget(raspivalue,i + 2,1,1,1);
    layout->addWidget(unitsLabel,i + 2,2,1,1);
    layout->addWidget(button,i + 2,3,1,1);
    layout->addWidget(raspirvalue,i + 2,4,1,1);
    layout->addWidget(shiftbutton,i + 2,5,1,1);
  }
   connect(signalMapper, SIGNAL(mapped(int)),this, SLOT(setRaspiValue(int)));
   connect(shiftsignalMapper, SIGNAL(mapped(int)),this, SLOT(setRaspiRValue(int)));
}
void 
RaspiDacControlWidget::getDescription(QString description){
  descriptionLabel->setText(tr("Connected to : %1").arg(description));
}
void
RaspiDacControlWidget::connectRaspi() {
  QString newraspi;
  newraspi = raspiCombo->itemText(raspiCombo->currentIndex());
  raspi->connectRaspi(newraspi);
}
void
RaspiDacControlWidget::resetRaspi() {
  QString newraspi;
  newraspi = raspiCombo->itemText(raspiCombo->currentIndex());
  raspi->resetRaspi(newraspi);
}

void 
RaspiDacControlWidget::getOutputValues(void *raspivalues) {
  // Cast in expected type
  QVector<double>* vfraspivalues;
  vfraspivalues = (QVector<double>*) raspivalues;

  QLOG_DEBUG ( ) << "RaspiCounterControlWidget::getOutputValues";
  for (int i = 0 ; i < raspivalueList->size(); i++) {
    QString valueString;
    valueString.setNum (vfraspivalues->at(i), 'f',4);
    QLineEdit *raspivalue = raspivalueList->at(i);
    raspivalue->setText(valueString);
  }
}
void
RaspiDacControlWidget::setRaspiValue(int output) {
  // Get value to be set
  QString newraspi;
  double value;
  value = raspivalueList->at(output)->text().toDouble();
  newraspi = raspiCombo->itemText(raspiCombo->currentIndex());
  raspi->setRaspiValue(newraspi,output, (void*)&value);
  raspi->updateDBValues(newraspi);
}
void
RaspiDacControlWidget::setRaspiRValue(int output) {
  // Get value to be set
  QString newraspi;
  double value;
  value = raspivalueList->at(output)->text().toDouble() + raspirvalueList->at(output)->text().toDouble();
  newraspi = raspiCombo->itemText(raspiCombo->currentIndex());
  raspi->setRaspiValue(newraspi,output, (void*)&value);
  raspi->updateDBValues(newraspi);
}

