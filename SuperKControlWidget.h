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
 * @file SuperKControlWidget.h
 * @brief Super K control widget.
 */

#ifndef SUPERKCONTROLWIDGET_H
#define SUPERKCONTROLWIDGET_H

#include "SuperK.h"

#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"

/// \ingroup ui
/// UI widget for SuperK laser control.
class SuperKControlWidget : public QWidget
{
  Q_OBJECT
    
    public:
  /// Construct a SuperK control widget.
  /// \param _driverList List of driver names (optional).
  SuperKControlWidget(QVector<QString>  *_driverList = 0);
  /// Destructor.
  ~SuperKControlWidget();

  /// Attach the SuperK controller.
  /// \param _superk SuperK controller instance.
  void setSuperK(SuperK *_superk);
  /// Set the list of available drivers.
  /// \param _driverList List of driver names.
  void setDriverList(QVector<QString>  *_driverList);
  
  private slots:
  /// Connect to the selected SuperK driver.
  void connectSuperK();
  /// Update the power readback.
  /// \param power Power value.
  void getPower(int power);
  /// Set the power value.
  void setPower();
  /// Update the ND readback.
  /// \param nd ND value.
  void getNd(int nd);
  /// Set the ND value.
  void setNd();
  /// Update the SWP readback.
  /// \param swp SWP value.
  void getSwp(int swp);
  /// Set the SWP value.
  void setSwp();
  /// Set wavelength/bandwidth parameters.
  void setWavelengthBandwith();
  /// Update the LWP readback.
  /// \param lwp LWP value.
  void getLwp(int lwp);
  /// Set the LWP value.
  void setLwp();
  /// Update the description display.
  /// \param description Device description.
  void getDescription(QString description);
  /// Stop periodic updates.
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
