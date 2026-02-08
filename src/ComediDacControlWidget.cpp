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
 * @file ComediDacControlWidget.cpp
 * @brief Comedi Dac control widget.
 */
#include "ComediDacControlWidget.h"

ComediDacControlWidget::ComediDacControlWidget(QVector<QString>* comediList)
    : connectButton(new QPushButton("Connect")),
      resetButton(new QPushButton("Reset")),
      comedivalueLabel(nullptr),
      outputLabel(nullptr),
      descriptionLabel(new QLabel()),
      comediCombo(new QComboBox()),
      outputsList(new QVector<QLabel*>()),
      unitsList(new QVector<QLabel*>()),
      setButtonList(new QVector<QPushButton*>()),
      shiftButtonList(new QVector<QPushButton*>()),
      comedivalueList(new QVector<QLineEdit*>()),
      comedirvalueList(new QVector<QLineEdit*>()),
      comediList(comediList),
      comedi(nullptr),
      layout(new QGridLayout(this)),
      signalMapper(nullptr),
      shiftsignalMapper(nullptr) {
  connectButton->setFixedWidth(100);
  connect(connectButton, SIGNAL(clicked()), this, SLOT(connectComedi()));

  resetButton->setFixedWidth(100);
  connect(resetButton, SIGNAL(clicked()), this, SLOT(resetComedi()));

  comediCombo->setFixedWidth(100);

  layout->addWidget(connectButton, 0, 0, 1, 1);
  layout->addWidget(comediCombo, 0, 1, 1, 1);
  layout->addWidget(descriptionLabel, 0, 2, 1, 5);
  layout->addWidget(resetButton, 1, 0, 1, 1);

  setLayout(layout);
}

ComediDacControlWidget::~ComediDacControlWidget() {
  QLOG_DEBUG() << "Deleting ComediDacControlWidget";
  delete signalMapper;
  delete shiftsignalMapper;
  delete outputsList;
  delete unitsList;
  delete setButtonList;
  delete shiftButtonList;
  delete comedivalueList;
  delete comedirvalueList;
}

void ComediDacControlWidget::setComedi(Comedi* comedi) {
  this->comedi = comedi;
  connect(comedi, SIGNAL(getDescription(QString)), this,
          SLOT(getDescription(QString)));
  connect(comedi, SIGNAL(getOutputs(int,QString)), this,
          SLOT(getOutputs(int,QString)));
  connect(comedi, SIGNAL(getOutputValues(void*)), this,
          SLOT(getOutputValues(void*)));
}

void ComediDacControlWidget::setComediList(QVector<QString>* comediList) {
  this->comediList = comediList;
  comediCombo->clear();
  if (!comediList) {
    return;
  }
  for (int i = 0; i < comediList->size(); i++) {
    comediCombo->addItem(comediList->at(i));
  }
}

void ComediDacControlWidget::clearOutputs() {
  outputsList->clear();
  comedivalueList->clear();
  setButtonList->clear();
  comedirvalueList->clear();
  shiftButtonList->clear();
  unitsList->clear();
}

void ComediDacControlWidget::getOutputs(int outputs, QString mode) {
  clearOutputs();

  delete signalMapper;
  delete shiftsignalMapper;
  shiftsignalMapper = new QSignalMapper(this);
  signalMapper = new QSignalMapper(this);
  for (int i = 0; i < outputs; i++) {
    QLabel *outputLabel = new QLabel(tr("Channel %1").arg(i));
    outputLabel->setFixedWidth(100);
    QLabel *unitsLabel = new QLabel("volts");
    unitsLabel->setFixedWidth(30);
    outputsList->push_back(outputLabel);
    unitsList->push_back(unitsLabel);
    QLineEdit *comedivalue = new QLineEdit("");
    comedivalue->setFixedWidth(100);
    comedivalueList->push_back(comedivalue);
    QLineEdit *comedirvalue = new QLineEdit("");
    comedirvalue->setFixedWidth(100);
    comedirvalueList->push_back(comedirvalue);
    QPushButton *button = new QPushButton(tr("Set %1").arg(mode));
    button->setFixedWidth(150);
    connect(button, SIGNAL(clicked()), signalMapper, SLOT(map()));
    signalMapper->setMapping(button, i);
    setButtonList->push_back(button);
    QPushButton *shiftbutton = new QPushButton(tr("SHIFT %1").arg(mode));
    shiftbutton->setFixedWidth(150);
    connect(shiftbutton, SIGNAL(clicked()), shiftsignalMapper, SLOT(map()));
    shiftsignalMapper->setMapping(shiftbutton, i);
    shiftButtonList->push_back(shiftbutton);

    layout->addWidget(outputLabel, i + 2, 0, 1, 1);
    layout->addWidget(comedivalue, i + 2, 1, 1, 1);
    layout->addWidget(unitsLabel, i + 2, 2, 1, 1);
    layout->addWidget(button, i + 2, 3, 1, 1);
    layout->addWidget(comedirvalue, i + 2, 4, 1, 1);
    layout->addWidget(shiftbutton, i + 2, 5, 1, 1);
  }
  connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(setComediValue(int)));
  connect(shiftsignalMapper, SIGNAL(mapped(int)), this, SLOT(setComediRValue(int)));
}

void ComediDacControlWidget::getDescription(QString description) {
  descriptionLabel->setText(tr("Connected to : %1").arg(description));
}

void ComediDacControlWidget::connectComedi() {
  QString newcomedi = comediCombo->itemText(comediCombo->currentIndex());
  comedi->connectComedi(newcomedi);
}

void ComediDacControlWidget::resetComedi() {
  QString newcomedi = comediCombo->itemText(comediCombo->currentIndex());
  comedi->resetComedi(newcomedi);
}

void ComediDacControlWidget::getOutputValues(void *comedivalues) {
  QVector<double>* vfcomedivalues = (QVector<double>*) comedivalues;

  QLOG_DEBUG() << "ComediCounterControlWidget::getOutputValues";
  for (int i = 0; i < comedivalueList->size(); i++) {
    QString valueString;
    valueString.setNum(vfcomedivalues->at(i), 'f', 4);
    QLineEdit *comedivalue = comedivalueList->at(i);
    comedivalue->setText(valueString);
  }
}

void ComediDacControlWidget::setComediValue(int output) {
  double value = comedivalueList->at(output)->text().toDouble();
  QString newcomedi = comediCombo->itemText(comediCombo->currentIndex());
  comedi->setComediValue(newcomedi, output, (void*)&value);
  comedi->updateDBValues(newcomedi);
}

void ComediDacControlWidget::setComediRValue(int output) {
  double value = comedivalueList->at(output)->text().toDouble() +
                 comedirvalueList->at(output)->text().toDouble();
  QString newcomedi = comediCombo->itemText(comediCombo->currentIndex());
  comedi->setComediValue(newcomedi, output, (void*)&value);
  comedi->updateDBValues(newcomedi);
}
