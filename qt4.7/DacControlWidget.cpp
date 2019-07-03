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
#include "DacControlWidget.h"

DacControlWidget::DacControlWidget(QVector<QString>  *_dacList)
 
{

  dacList = _dacList;
 
  layout = new QGridLayout(this);
  connectButton = new  QPushButton("Connect");
  connectButton->setFixedWidth(100);
  connect( connectButton, SIGNAL(clicked()), this, SLOT(connectDac()));
  
  resetButton = new  QPushButton("Reset");
  resetButton->setFixedWidth(100);
  connect( resetButton, SIGNAL(clicked()), this, SLOT(resetDac()));

  dacCombo = new QComboBox();
  dacCombo->setFixedWidth(100);
  descriptionLabel = new QLabel();

  layout->addWidget(connectButton,0,0,1,1);
  layout->addWidget(dacCombo,0,1,1,1);
  layout->addWidget(descriptionLabel,0,2,1,1);
  layout->addWidget(resetButton,1,0,1,1);
  
  setLayout(layout);
  signalMapper = NULL;
  shiftsignalMapper = NULL;
  outputsList = new QVector<QLabel*>();
  setButtonList = new QVector<QPushButton*>();
  dacvalueList = new QVector<QLineEdit*>();
  shiftButtonList = new QVector<QPushButton*>();
  dacrvalueList = new QVector<QLineEdit*>();
}
DacControlWidget::~DacControlWidget()
{
  QLOG_DEBUG ( ) << "Deleting DacControlWidget";
}
void
DacControlWidget::setDac(Dac *_dac) {
  dac = _dac;
  connect(dac,SIGNAL(getDescription(QString)),this,SLOT(getDescription(QString)));
  connect(dac,SIGNAL(getOutputs(int,QString)),this,SLOT(getOutputs(int,QString)));
  connect(dac,SIGNAL(getOutputValues(QVector<float>*)),this,SLOT(getOutputValues(QVector<float>*)));
}
void
DacControlWidget::setDacList(QVector<QString>  *_dacList) {
  dacList = _dacList;
  dacCombo->clear();
  for (int i = 0 ; i < dacList->size(); i++)
    dacCombo->addItem(dacList->at(i));
}
void
DacControlWidget::getOutputs(int outputs, QString mode) {
  outputsList->clear();
  dacvalueList->clear();
  setButtonList->clear();
  dacrvalueList->clear();
  shiftButtonList->clear();

  // Create a signal mapper for buttons
  if (signalMapper != NULL) delete signalMapper;
  if (shiftsignalMapper != NULL) delete shiftsignalMapper;
  shiftsignalMapper = new QSignalMapper(this);
  signalMapper = new QSignalMapper(this);
  for (int i = 0 ; i < outputs; i++) {
    QLabel *outputLabel = new QLabel(tr("Output %1").arg(i));
    outputLabel->setFixedWidth(100);
    outputsList->push_back(outputLabel);
    QLineEdit *dacvalue = new QLineEdit("");
    dacvalue->setFixedWidth(100);
    dacvalueList->push_back(dacvalue);
    QLineEdit *dacrvalue = new QLineEdit("");
    dacrvalue->setFixedWidth(100);
    dacrvalueList->push_back(dacrvalue);
    QPushButton *button = new QPushButton(tr("SET %1").arg(mode));
    button->setFixedWidth(150);
    connect( button, SIGNAL(clicked()), signalMapper, SLOT(map()));
    signalMapper->setMapping(button, i);
    setButtonList->push_back(button);
    QPushButton *shiftbutton = new QPushButton(tr("SHIFT %1").arg(mode));
    shiftbutton->setFixedWidth(150);
    connect( shiftbutton, SIGNAL(clicked()), shiftsignalMapper, SLOT(map()));
    shiftsignalMapper->setMapping(shiftbutton, i);
    shiftButtonList->push_back(shiftbutton);
    
    layout->addWidget(outputLabel,i + 2,0,1,1);
    layout->addWidget(dacvalue,i + 2,1,1,1);
    layout->addWidget(button,i + 2,2,1,1);
    layout->addWidget(dacrvalue,i + 2,3,1,1);
    layout->addWidget(shiftbutton,i + 2,4,1,1);
  }
   connect(signalMapper, SIGNAL(mapped(int)),this, SLOT(setDacValue(int)));
   connect(shiftsignalMapper, SIGNAL(mapped(int)),this, SLOT(setDacRValue(int)));
}
void 
DacControlWidget::getDescription(QString description){
  descriptionLabel->setText(tr("Connected to : %1").arg(description));
}
void
DacControlWidget::connectDac() {
  QString newdac;
  newdac = dacCombo->itemText(dacCombo->currentIndex());
  dac->connectDac(newdac);
}
void
DacControlWidget::resetDac() {
  QString newdac;
  newdac = dacCombo->itemText(dacCombo->currentIndex());
  dac->resetDac(newdac);
}

void DacControlWidget::getOutputValues(QVector<float> *dacvalues) {
  QLOG_DEBUG ( ) << "DacControlWidget::getOutputValues";
  for (int i = 0 ; i < dacvalueList->size(); i++) {
    QString valueString;
    valueString.setNum (dacvalues->at(i), 'f',3);
    QLineEdit *dacvalue = dacvalueList->at(i);
    dacvalue->setText(valueString);
  }
}
void
DacControlWidget::setDacValue(int output) {
  // Get value to be set
  QString newdac;
  float value;
  value = dacvalueList->at(output)->text().toFloat();
  newdac = dacCombo->itemText(dacCombo->currentIndex());
  dac->setDacValue(newdac,output, value);
  dac->updateDBValues(newdac);
}
void
DacControlWidget::setDacRValue(int output) {
  // Get value to be set
  QString newdac;
  float value;
  value = dacvalueList->at(output)->text().toFloat() + dacrvalueList->at(output)->text().toFloat();
  newdac = dacCombo->itemText(dacCombo->currentIndex());
  dac->setDacValue(newdac,output, value);
  dac->updateDBValues(newdac);
}

