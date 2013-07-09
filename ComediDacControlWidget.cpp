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
#include "ComediDacControlWidget.h"

ComediDacControlWidget::ComediDacControlWidget(QVector<QString>  *_comediList)
{

  comediList = _comediList;
 
  layout = new QGridLayout(this);
  connectButton = new  QPushButton("Connect");
  connectButton->setFixedWidth(100);
  connect( connectButton, SIGNAL(clicked()), this, SLOT(connectComedi()));
  
  resetButton = new  QPushButton("Reset");
  resetButton->setFixedWidth(100);
  connect( resetButton, SIGNAL(clicked()), this, SLOT(resetComedi()));

  comediCombo = new QComboBox();
  comediCombo->setFixedWidth(100);
  descriptionLabel = new QLabel();

  layout->addWidget(connectButton,0,0,1,1);
  layout->addWidget(comediCombo,0,1,1,1);
  layout->addWidget(descriptionLabel,0,2,1,1);
  layout->addWidget(resetButton,1,0,1,1);
  
  setLayout(layout);
  signalMapper = NULL;
  outputsList = new QVector<QLabel*>();
  unitsList = new QVector<QLabel*>();
  setButtonList = new QVector<QPushButton*>();
  comedivalueList = new QVector<QLineEdit*>();
  comeditimerList = new QVector<QLineEdit*>();
}
ComediDacControlWidget::~ComediDacControlWidget()
{
  QLOG_DEBUG ( ) << "Deleting ComediDacControlWidget";
}
void
ComediDacControlWidget::setComedi(Comedi *_comedi) {
  comedi = _comedi;
  connect(comedi,SIGNAL(getDescription(QString)),this,SLOT(getDescription(QString)));
  connect(comedi,SIGNAL(getOutputs(int,QString)),this,SLOT(getOutputs(int,QString)));
  connect(comedi,SIGNAL(getOutputValues(void*)),this,SLOT(getOutputValues(void*)));
}
void
ComediDacControlWidget::setComediList(QVector<QString>  *_comediList) {
  comediList = _comediList;
  comediCombo->clear();
  for (int i = 0 ; i < comediList->size(); i++)
    comediCombo->addItem(comediList->at(i));
}
void
ComediDacControlWidget::getOutputs(int outputs, QString mode) {
  outputsList->clear();
  comedivalueList->clear();
  setButtonList->clear();
  // Create a signal mapper for buttons
  if (signalMapper != NULL) delete signalMapper;
  signalMapper = new QSignalMapper(this);
  for (int i = 0 ; i < outputs; i++) {
    QLabel *outputLabel = new QLabel(tr("Channel %1").arg(i));
    outputLabel->setFixedWidth(100);
    QLabel *unitsLabel = new QLabel("volts");
    outputsList->push_back(outputLabel);
    unitsList->push_back(unitsLabel);
    QLineEdit *comedivalue = new QLineEdit("");
    comedivalue->setFixedWidth(100);
    comedivalueList->push_back(comedivalue);
    QPushButton *button = new QPushButton(tr("Set %1").arg(mode));
    button->setFixedWidth(150);
    connect( button, SIGNAL(clicked()), signalMapper, SLOT(map()));
    signalMapper->setMapping(button, i);
    setButtonList->push_back(button);

    layout->addWidget(outputLabel,i + 2,0,1,1);
    layout->addWidget(comedivalue,i + 2,1,1,1);
    layout->addWidget(unitsLabel,i + 2,2,1,1);
    layout->addWidget(button,i + 2,3,1,1);
  }
   connect(signalMapper, SIGNAL(mapped(int)),this, SLOT(setComediValue(int)));
}
void 
ComediDacControlWidget::getDescription(QString description){
  descriptionLabel->setText(tr("Connected to : %1").arg(description));
}
void
ComediDacControlWidget::connectComedi() {
  QString newcomedi;
  newcomedi = comediCombo->itemText(comediCombo->currentIndex());
  comedi->connectComedi(newcomedi);
}
void
ComediDacControlWidget::resetComedi() {
  QString newcomedi;
  newcomedi = comediCombo->itemText(comediCombo->currentIndex());
  comedi->resetComedi(newcomedi);
}

void 
ComediDacControlWidget::getOutputValues(void *comedivalues) {
  // Cast in expected type
  QVector<double>* vfcomedivalues;
  vfcomedivalues = (QVector<double>*) comedivalues;

  QLOG_DEBUG ( ) << "ComediCounterControlWidget::getOutputValues";
  for (int i = 0 ; i < comedivalueList->size(); i++) {
    QString valueString;
    valueString.setNum (vfcomedivalues->at(i), 'f',4);
    QLineEdit *comedivalue = comedivalueList->at(i);
    comedivalue->setText(valueString);
  }
}
void
ComediDacControlWidget::setComediValue(int output) {
  // Get value to be set
  QString newcomedi;
  double value;
  value = comedivalueList->at(output)->text().toDouble();
  newcomedi = comediCombo->itemText(comediCombo->currentIndex());
  comedi->setComediValue(newcomedi,output, (void*)&value);
  comedi->updateDBValues(newcomedi);
}
