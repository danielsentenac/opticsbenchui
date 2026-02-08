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
 * @file SuperKControlWidget.cpp
 * @brief Super K control widget.
 */

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

  power = new QLineEdit();
  power->setFixedWidth(100);
  powerButton = new QPushButton("Set Power");
  connect(powerButton, SIGNAL(clicked()), this, SLOT(setPower()));
  cur_powerLabel = new QLabel("Power:");
  cur_power = new QLineEdit();
  cur_power->setFixedWidth(100);
  power_units = new QLabel("%");

  nd = new QLineEdit();
  nd->setFixedWidth(100);
  ndButton = new QPushButton("Set Nd");
  connect(ndButton, SIGNAL(clicked()), this, SLOT(setNd()));
  cur_ndLabel = new QLabel("Nd:");
  cur_nd = new QLineEdit();
  cur_nd->setFixedWidth(100);
  nd_units = new QLabel("%");

  bw = new QLineEdit();
  bw->setFixedWidth(100);
  
  cw = new QLineEdit();
  cw->setFixedWidth(100);
  cwButton = new QPushButton("Set Wavelength: \n  and \n Bandwith:");
  cwButton->setFixedHeight(80);
  connect(cwButton, SIGNAL(clicked()), this, SLOT(setWavelengthBandwith()));
  
  cur_bwLabel = new QLabel("Bandwith:");
  cur_bw = new QLineEdit();
  cur_bw->setFixedWidth(100);
  bw_units = new QLabel("nm");

  cur_cwLabel = new QLabel("Wavelength:");
  cur_cw = new QLineEdit();
  cur_cw->setFixedWidth(100);
  cw_units = new QLabel("nm");

  cur_lwpLabel = new QLabel("Lwp:");
  cur_lwp = new QLabel();
  cur_lwp->setFixedWidth(50);
  lwp_units = new QLabel("nm");

  cur_swpLabel = new QLabel("Swp:");
  cur_swp = new QLabel();
  cur_swp->setFixedWidth(50);
  swp_units = new QLabel("nm");

  layout->addWidget(connectButton,0,0,1,1);
  layout->addWidget(driverCombo,0,1,1,1);
  layout->addWidget(descriptionLabel,0,2,1,4);

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
 
  layout->addWidget(cwButton,4,0,2,1);
  layout->addWidget(cw,4,1,1,1);
  layout->addWidget(cur_cwLabel,4,2,1,1,Qt::AlignRight);
  layout->addWidget(cur_cw,4,3,1,1);
  layout->addWidget(cw_units,4,4,1,2);

  layout->addWidget(bw,5,1,1,1);
  layout->addWidget(cur_bwLabel,5,2,1,1,Qt::AlignRight);
  layout->addWidget(cur_bw,5,3,1,1);
  layout->addWidget(bw_units,5,4,1,2);

  QHBoxLayout *lwplayout = new QHBoxLayout();
  lwplayout->addWidget(cur_lwpLabel);
  lwplayout->addWidget(cur_lwp);
  lwplayout->addWidget(lwp_units);

  layout->addLayout(lwplayout,6,1,1,1);

  QHBoxLayout *swplayout = new QHBoxLayout();
  swplayout->addWidget(cur_swpLabel);
  swplayout->addWidget(cur_swp);
  swplayout->addWidget(swp_units);

  layout->addLayout(swplayout,7,1,1,1);

  setLayout(layout);

}
SuperKControlWidget::~SuperKControlWidget()
{
  QLOG_DEBUG ( ) << "Deleting SuperKControlWidget";
}
void
SuperKControlWidget::getPower(int pw) {
  QString powerQString;
  float tmp = (float) pw;
  tmp/=10.;
  powerQString.setNum(tmp);
  cur_power->setText(powerQString);
  power->setText(powerQString);
}
void
SuperKControlWidget::getNd(int n) {
  QString ndQString;
  float tmp = (float) n;
  tmp/=10.;
  ndQString.setNum(tmp);
  cur_nd->setText(ndQString);
  nd->setText(ndQString);
}
void
SuperKControlWidget::getSwp(int swp) {
  swpValue = (float)swp / 10;
  QString swpQString;
  swpQString.setNum(swpValue);
  cur_swp->setText(swpQString);
  updateParam();
}
void
SuperKControlWidget::getLwp(int lwp) {
  lwpValue = (float)lwp / 10;
  QString lwpQString;
  lwpQString.setNum(lwpValue);
  cur_lwp->setText(lwpQString);
  updateParam();
}
void
SuperKControlWidget::setPower() {
  QString driver;
  driver = driverCombo->itemText(driverCombo->currentIndex());
  int powerValue = power->text().toFloat() * 10;
  QLOG_INFO() << "setPower " << powerValue;
  superk->setPower(driver, powerValue);
  timer->start(100);
}
void
SuperKControlWidget::setNd() {
  QString driver;
  driver = driverCombo->itemText(driverCombo->currentIndex());
  int ndValue = nd->text().toFloat() * 10;
  superk->setNd(driver, ndValue);
  timer->start(100);
}
void
SuperKControlWidget::setWavelengthBandwith() {
  setPower();
  setSwp();
  setLwp();
}
void
SuperKControlWidget::setSwp() {
  QString driver;
  driver = driverCombo->itemText(driverCombo->currentIndex());
  swpValue = cw->text().toFloat() * 10 + bw->text().toFloat() * 10 / 2 ;
  QLOG_INFO() << "setSwp " << swpValue ;
  superk->setSwp(driver, (int)swpValue);
  timer->start(100);
}
void
SuperKControlWidget::setLwp() {
  QString driver;
  driver = driverCombo->itemText(driverCombo->currentIndex());
  lwpValue = cw->text().toFloat() * 10 - bw->text().toFloat() * 10 / 2;
  QLOG_INFO() << "setLwp " << lwpValue;
  superk->setLwp(driver, (int)lwpValue);
  timer->start(100);
}
void
SuperKControlWidget::updateParam() {
  QString cwQString, bwQString;
  float bwValue, cwValue;
  bwValue = swpValue - lwpValue;
  cwValue = (swpValue + lwpValue) / 2;
  bwQString.setNum(bwValue);
  cur_bw->setText(bwQString);
  bw->setText(bwQString);
  cwQString.setNum(cwValue);
  cur_cw->setText(cwQString);
  cw->setText(cwQString);
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
