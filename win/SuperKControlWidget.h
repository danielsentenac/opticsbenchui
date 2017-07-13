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

#ifndef SUPERKCONTROLWIDGET_H
#define SUPERKCONTROLWIDGET_H

#include "SuperK.h"

#include <QtSql>
#include <QtGui>
#include "QsLog.h"

class SuperKControlWidget : public QWidget
{
  Q_OBJECT
    
    public:
  SuperKControlWidget(QVector<QString>  *_driverList = 0);
  ~SuperKControlWidget();

  void setSuperK(SuperK *_superk);
  void setDriverList(QVector<QString>  *_driverList);
  
  private slots:
  void connectSuperK();
  void getPower(int power);
  void setPower();
  void getNd(int nd);
  void setNd();
  void getSwp(int swp);
  void setSwp();
  void setWavelengthBandwith();
  void getLwp(int lwp);
  void setLwp();
  void getDescription(QString description);
  void stopTimer();

 private:

  QTimer      *timer;
  QPushButton *connectButton; 
  QComboBox   *driverCombo;
  QLabel      *descriptionLabel;
  QVector<QString> *driverList;
  QLineEdit    *power;
  QPushButton *powerButton;
  QLabel      *cur_powerLabel;
  QLineEdit   *cur_power;
  QLabel      *power_units;
  QLineEdit    *nd;
  QPushButton *ndButton;
  QLabel      *cur_ndLabel;
  QLineEdit   *cur_nd;
  QLabel      *nd_units;
  QLineEdit    *bw;
  QPushButton *cwButton;
  QLabel      *cur_swpLabel;
  QLabel      *cur_swp;
  QLabel      *swp_units;
  float        swpValue;
  QLineEdit    *cw;
  QLabel      *cur_lwpLabel;
  QLabel      *cur_lwp;
  QLabel      *lwp_units;
  float        lwpValue;
  QLabel      *cur_cwLabel;
  QLineEdit   *cur_cw;
  QLabel      *cw_units;
  QLabel      *cur_bwLabel;
  QLineEdit   *cur_bw;
  QLabel      *bw_units;
  
  SuperK *superk;

  void updateParam();
};
#endif
