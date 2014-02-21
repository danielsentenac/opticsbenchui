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

#include "SuperKControlWidget.h"

SuperKControlWidget::SuperKControlWidget(QVector<QString>  *_driverList)
 
{
  driverList = _driverList;
  
  QGridLayout *layout = new QGridLayout(this);
  connectButton = new  QPushButton("Connect");
  connect(connectButton, SIGNAL(clicked()), this, SLOT(connectSuperK()));
  driverCombo = new QComboBox();
  driverCombo->setFixedWidth(100);
  descriptionLabel = new QLabel();

  power = new QSpinBox();
  power->setRange(0,1000);
  power->setFixedWidth(100);
  powerButton = new QPushButton("Set Power");
  connect(powerButton, SIGNAL(clicked()), this, SLOT(setPower()));
  cur_powerLabel = new QLabel("Power:");
  cur_power = new QLineEdit();
  cur_power->setFixedWidth(100);
  power_units = new QLabel("%");

  nd = new QSpinBox();
  nd->setRange(0,100000);
  nd->setFixedWidth(100);
  ndButton = new QPushButton("Set Nd");
  connect(ndButton, SIGNAL(clicked()), this, SLOT(setNd()));
  cur_ndLabel = new QLabel("Nd:");
  cur_nd = new QLineEdit();
  cur_nd->setFixedWidth(100);
  nd_units = new QLabel("%");

  swp = new QSpinBox();
  swp->setRange(0,100000);
  swp->setFixedWidth(100);
  swpButton = new QPushButton("Set Swp");
  connect(swpButton, SIGNAL(clicked()), this, SLOT(setSwp()));
  cur_swpLabel = new QLabel("Swp:");
  cur_swp = new QLineEdit();
  cur_swp->setFixedWidth(100); 
  swp_units = new QLabel("nm"); 

  lwp = new QSpinBox();
  lwp->setRange(0,100000);
  lwp->setFixedWidth(100);
  lwpButton = new QPushButton("Set Lwp");
  connect(lwpButton, SIGNAL(clicked()), this, SLOT(setLwp()));
  cur_lwpLabel = new QLabel("Lwp:");
  cur_lwp = new QLineEdit();
  cur_lwp->setFixedWidth(100);
  lwp_units = new QLabel("nm");

  cur_cwLabel = new QLabel("Central Wavelength:");
  cur_cw = new QLabel();
  cur_cw->setFixedWidth(100);
  cw_units = new QLabel("nm");

  cur_bwLabel = new QLabel("Bandwidth:");
  cur_bw = new QLabel();
  cur_bw->setFixedWidth(100);
  bw_units = new QLabel("nm");

  layout->addWidget(connectButton,0,0,1,1);
  layout->addWidget(driverCombo,0,1,1,1);
  layout->addWidget(descriptionLabel,0,2,1,4);

  layout->addWidget(cur_cwLabel,1,0,1,1);
  layout->addWidget(cur_cw,1,1,1,1,Qt::AlignCenter);
  layout->addWidget(cw_units,1,1,1,1,Qt::AlignRight);

  layout->addWidget(cur_bwLabel,1,2,1,1);
  layout->addWidget(cur_bw,1,3,1,1,Qt::AlignCenter);
  layout->addWidget(bw_units,1,3,1,1,Qt::AlignRight);

  layout->addWidget(powerButton,2,0,1,1);
  layout->addWidget(power,2,1,1,1);
  layout->addWidget(cur_powerLabel,2,2,1,1,Qt::AlignRight);
  layout->addWidget(cur_power,2,3,1,1);
  layout->addWidget(power_units,2,4,1,2);

  layout->addWidget(ndButton,3,0,1,1);
  layout->addWidget(nd,3,1,1,1);
  layout->addWidget(cur_ndLabel,3,2,1,1,Qt::AlignRight);
  layout->addWidget(cur_nd,3,3,1,1);
  layout->addWidget(nd_units,3,4,1,2);
 
  layout->addWidget(swpButton,4,0,1,1);
  layout->addWidget(swp,4,1,1,1);
  layout->addWidget(cur_swpLabel,4,2,1,1,Qt::AlignRight);
  layout->addWidget(cur_swp,4,3,1,1);
  layout->addWidget(swp_units,4,4,1,2);

  layout->addWidget(lwpButton,5,0,1,1);
  layout->addWidget(lwp,5,1,1,1);
  layout->addWidget(cur_lwpLabel,5,2,1,1,Qt::AlignRight);
  layout->addWidget(cur_lwp,5,3,1,1);
  layout->addWidget(lwp_units,5,4,1,2);

  setLayout(layout);

}
SuperKControlWidget::~SuperKControlWidget()
{
  QLOG_DEBUG ( ) << "Deleting SuperKControlWidget";
}
void
SuperKControlWidget::getPower(int power) {
  QString powerQString;
  float tmpF = (float) power;
  tmpF/=10.;
  powerQString.setNum(tmpF);
  cur_power->setText(powerQString);
}
void
SuperKControlWidget::getNd(int nd) {
  QString ndQString;
  float tmpF = (float) nd;
  tmpF/=10.;
  ndQString.setNum(tmpF);
  cur_nd->setText(ndQString);
}
void
SuperKControlWidget::getSwp(int swp) {
  QString swpQString;
  float tmpF = (float) swp;
  tmpF/=10.;
  swpQString.setNum(tmpF);
  cur_swp->setText(swpQString);
  updateParam();
}
void
SuperKControlWidget::getLwp(int lwp) {
  QString lwpQString;
  float tmpF = (float) lwp;
  tmpF/=10.; 
  lwpQString.setNum(tmpF);
  cur_lwp->setText(lwpQString);
  updateParam();
}
void
SuperKControlWidget::setPower() {
  QString driver;
  driver = driverCombo->itemText(driverCombo->currentIndex());
  int powerValue = power->text().toInt();
  superk->setPower(driver, powerValue);
  timer->start(100);
}
void
SuperKControlWidget::setNd() {
  QString driver;
  driver = driverCombo->itemText(driverCombo->currentIndex());
  int ndValue = nd->text().toInt();
  superk->setNd(driver, ndValue);
  timer->start(100);
}
void
SuperKControlWidget::setSwp() {
  QString driver;
  driver = driverCombo->itemText(driverCombo->currentIndex());
  int swpValue = swp->text().toInt();
  superk->setSwp(driver, swpValue);
  timer->start(100);
}
void
SuperKControlWidget::setLwp() {
  QString driver;
  driver = driverCombo->itemText(driverCombo->currentIndex());
  int lwpValue = lwp->text().toInt();
  superk->setLwp(driver, lwpValue);
  timer->start(100);
}
void
SuperKControlWidget::updateParam() {
  QString cwQString, bwQString;
  int cw = (cur_swp->text().toInt() - cur_lwp->text().toInt()) / 2;
  int bw = (cur_swp->text().toInt() - cur_lwp->text().toInt());
  cwQString.setNum(cw);
  cur_cw->setText(cwQString);
  bwQString.setNum(bw);
  cur_bw->setText(bwQString);

}
void
SuperKControlWidget::setSuperK(SuperK *_superk) {
  superk = _superk;
  connect(superk,SIGNAL(getPower(int)),this,SLOT(getPower(int)));
  connect(superk,SIGNAL(getNd(int)),this,SLOT(getNd(int)));
  connect(superk,SIGNAL(getSwp(int)),this,SLOT(getSwp(int)));
  connect(superk,SIGNAL(getLwp(int)),this,SLOT(getLwp(int)));
  connect(superk,SIGNAL(getDescription(QString)),this,SLOT(getDescription(QString)));
  connect(superk,SIGNAL(stopTimer()),this,SLOT(stopTimer()));
  //
  // Launch a timer to get the current position of the driver every second
  //
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), superk, SLOT(operationComplete()));
}
void
SuperKControlWidget::setDriverList(QVector<QString>  *_driverList) {
  driverList = _driverList;
  driverCombo->clear();
  for (int i = 0 ; i < driverList->size(); i++)
    driverCombo->addItem(driverList->at(i));
}
void
SuperKControlWidget::connectSuperK() {
  QString driver;
  driver = driverCombo->itemText(driverCombo->currentIndex());
  superk->connectSuperK(driver);
}
void 
SuperKControlWidget::getDescription(QString description) {
  descriptionLabel->setText(tr("Connected to : %1").arg(description));
}
void 
SuperKControlWidget::stopTimer() {
  timer->stop();
}
