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
 * @file RaspiDacControlWidget.h
 * @brief Raspi Dac control widget.
 */
#ifndef RASPIDACCONTROLWIDGET_H
#define RASPIDACCONTROLWIDGET_H

#include "Raspi.h"

#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"


typedef QVector<float> floatVector;

/// \ingroup ui
/// UI widget for Raspberry Pi DAC control.
class RaspiDacControlWidget : public QWidget
{
  Q_OBJECT
    
    public:
  /// Construct a Raspi DAC control widget.
  /// \param _raspiList List of Raspi device names (optional).
  RaspiDacControlWidget(QVector<QString>  *_raspiList = 0);
  /// Destructor.
  ~RaspiDacControlWidget();
    
  /// Attach the Raspi controller.
  /// \param _raspi Raspi controller instance.
  void setRaspi(Raspi *_raspi);
  /// Set the list of available Raspi devices.
  /// \param _raspiList List of Raspi device names.
  void setRaspiList(QVector<QString>  *_raspiList);

  private slots:
  /// Connect to the selected Raspi device.
  void connectRaspi();
  /// Reset the selected device.
  void resetRaspi();
  /// Set a DAC output value.
  /// \param output Output channel index.
  void setRaspiValue(int output);
  /// Set a DAC output raw value.
  /// \param output Output channel index.
  void setRaspiRValue(int output);
  /// Update the description display.
  /// \param description Device description.
  void getDescription(QString description);
  /// Update output count.
  /// \param outputs Number of outputs.
  void getOutputs(int outputs);
  /// Update output value displays.
  /// \param raspivalues Pointer to device-specific values.
  void getOutputValues(void *raspivalues);

 private:
  QPushButton *connectButton; 
  QPushButton *resetButton; 
  QLabel      *raspivalueLabel;
  QLabel      *outputLabel;
  QLabel      *descriptionLabel; 
  QComboBox   *raspiCombo;
  QVector<QLabel*>  *outputsList;
  QVector<QLabel*>  *unitsList;
  QVector<QPushButton*> *setButtonList;
  QVector<QPushButton*> *shiftButtonList;
  QVector<QLineEdit*>   *raspivalueList;
  QVector<QLineEdit*>   *raspirvalueList;
  QVector<QString> *raspiList;
  Raspi  *raspi;
  QGridLayout *layout;
  QSignalMapper *signalMapper, *shiftsignalMapper;
};
#endif
