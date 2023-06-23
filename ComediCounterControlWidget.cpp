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
#include "ComediCounterControlWidget.h"

ComediCounterControlWidget::ComediCounterControlWidget(QVector<QString>  *_comediList)
 
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
  
  vBox = new QWidget(this);
  vBox->setWindowTitle( "ComediCounter Plot" );
  boxlayout = new QVBoxLayout( this );
  vBox->setMinimumSize( 600, 400 );
  vBox->setMaximumSize( 600, 400 );
  plot = NULL;
  boxlayout->addWidget( plot );
  layout->addWidget(connectButton,0,0,1,1);
  layout->addWidget(comediCombo,0,1,1,1);
  layout->addWidget(descriptionLabel,0,2,1,1);
  layout->addWidget(resetButton,1,0,1,1);
  layout->addLayout(boxlayout,2,0,1,5);
  setLayout(layout);
  signalMapper = NULL;
  outputsList = new QVector<QLabel*>();
  unitsList = new QVector<QLabel*>();
  setButtonList = new QVector<QPushButton*>();
  comedivalueList = new QVector<QLineEdit*>();
  comeditimerList = new QVector<QLineEdit*>();
}
ComediCounterControlWidget::~ComediCounterControlWidget()
{
  QLOG_INFO ( ) << "Deleting ComediCounterControlWidget";
  stopFuture = true;
  future.waitForFinished();
  if (plot != NULL) {delete plot; plot = NULL;}
  while(!boxlayout->isEmpty()) {
      delete boxlayout->takeAt(0);
  }
  delete vBox;
  if (comedi) delete comedi;
}
void
ComediCounterControlWidget::setComedi(Comedi *_comedi) {
  comedi = _comedi;
  connect(comedi,SIGNAL(getDescription(QString)),this,SLOT(getDescription(QString)));
  connect(comedi,SIGNAL(getOutputs(int,QString)),this,SLOT(getOutputs(int,QString)));
  connect(comedi,SIGNAL(getOutputValues(void*)),this,SLOT(getOutputValues(void*)));
}
void
ComediCounterControlWidget::setComediList(QVector<QString>  *_comediList) {
  comediList = _comediList;
  comediCombo->clear();
  for (int i = 0 ; i < comediList->size(); i++)
    comediCombo->addItem(comediList->at(i));
}
void
ComediCounterControlWidget::getOutputs(int outputs, QString mode) {
  outputsList->clear();
  comedivalueList->clear();
  setButtonList->clear();
  // Create a signal mapper for buttons
  if (signalMapper != NULL) delete signalMapper;
  signalMapper = new QSignalMapper(this);
  for (int i = 0 ; i < outputs; i++) {
    QLabel *outputLabel = new QLabel(tr("Output %1").arg(i));
    outputLabel->setFixedWidth(100);
    QLabel *unitsLabel = new QLabel("microsecs");
    outputsList->push_back(outputLabel);
    unitsList->push_back(unitsLabel);
    QLineEdit *comedivalue = new QLineEdit("");
    comedivalue->setFixedWidth(100);
    comedivalueList->push_back(comedivalue);
    QPushButton *button = new QPushButton(tr("Start %1").arg(mode));
    button->setCheckable(true);
    button->setFixedWidth(150);
    connect( button, SIGNAL(clicked()), signalMapper, SLOT(map()));
    signalMapper->setMapping(button, i);
    setButtonList->push_back(button);
    QLineEdit *comeditimer = new QLineEdit("");
    comeditimer->setFixedWidth(100);
    comeditimerList->push_back(comeditimer);

    layout->addWidget(outputLabel,i + 3,0,1,1);
    layout->addWidget(comedivalue,i + 3,1,1,1);
    layout->addWidget(button,i + 3,2,1,1);
    layout->addWidget(comeditimer,i + 3,3,1,1);
    layout->addWidget(unitsLabel,i + 3,4,1,1);
  }
   connect(signalMapper, SIGNAL(mapped(int)),this, SLOT(startCounting(int)));

}
void 
ComediCounterControlWidget::getDescription(QString description){
  descriptionLabel->setText(tr("Connected to : %1").arg(description));
}
void
ComediCounterControlWidget::connectComedi() {
  QString newcomedi;
  newcomedi = comediCombo->itemText(comediCombo->currentIndex());
  comedi->connectComedi(newcomedi);
}
void
ComediCounterControlWidget::resetComedi() {
  QString newcomedi;
  newcomedi = comediCombo->itemText(comediCombo->currentIndex());
  comedi->resetComedi(newcomedi);
}

void ComediCounterControlWidget::getOutputValues(void *comedivalues) {
  // Cast in expected type
  QVector<double>* vfcomedivalues;
  vfcomedivalues = (QVector<double>*) comedivalues;

  QLOG_DEBUG ( ) << "ComediCounterControlWidget::getOutputValues";
  for (int i = 0 ; i < comedivalueList->size(); i++) {
    QString valueString;
    valueString.setNum (vfcomedivalues->at(i), 'f',3);
    QLineEdit *comedivalue = comedivalueList->at(i);
    comedivalue->setText(valueString);
  }
}
void
ComediCounterControlWidget::setComediValue(int output) {
  // Get value to be set
  QString newcomedi;
  int value;
  value = comeditimerList->at(output)->text().toInt();
  newcomedi = comediCombo->itemText(comediCombo->currentIndex());
  comedi->setComediValue(newcomedi,output, (void*)&value);
  comedi->updateDBValues(newcomedi);
}
void
ComediCounterControlWidget::setComediContinuousValue(int output) {
  QPushButton *button = setButtonList->at(output);
  while ( button->isChecked() && !stopFuture)
     setComediValue(output);
}
void
ComediCounterControlWidget::startCounting(int output) {
  QPushButton *button = setButtonList->at(output);
  if (button->isChecked()) {
   for (int i = 0; i < setButtonList->size();i++) {
     if ( i != output) {
       QPushButton *button = setButtonList->at(i);
       button->setChecked(false);
       stopFuture = true;
       future.waitForFinished();
       if (plot != NULL) {delete plot; plot = NULL;}
       while(!boxlayout->isEmpty()) {
        delete boxlayout->takeAt(0);
       }
     }
  }
   stopFuture = false;
   startPlot(output);
   future = QtConcurrent::run(this,&ComediCounterControlWidget::setComediContinuousValue, output);
  }
}
void 
ComediCounterControlWidget::startPlot(int output) {
  plot = new ComediCounterPlot(vBox, comedi, output );
  plot->setTitle( tr("Comedi Counter"));
  const int margin = 5;
  plot->setContentsMargins( margin, margin, margin, margin );
  boxlayout->addWidget( plot );
}

