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
 * @file DacControlWidget.h
 * @brief Dac control widget.
 */
#ifndef DACCONTROLWIDGET_H
#define DACCONTROLWIDGET_H

#include "Dac.h"

#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"


typedef QVector<float> floatVector;

/// \ingroup ui_dac
/// UI widget for DAC control.
class DacControlWidget : public QWidget
{
  Q_OBJECT
    
public:
  /// Construct a DAC control widget.
  /// \param dacList List of DAC names (optional).
  explicit DacControlWidget(QVector<QString>* dacList = nullptr);
  /// Destructor.
  ~DacControlWidget() override;
    
  /// Attach the DAC controller.
  /// \param dac DAC controller instance.
  void setDac(Dac* dac);
  /// Set the list of available DAC devices.
  /// \param dacList List of DAC names.
  void setDacList(QVector<QString>* dacList);

private slots:
  /// Connect to the selected DAC.
  void connectDac();
  /// Reset the selected DAC.
  void resetDac();
  /// Set a DAC output value.
  /// \param output Output channel index.
  void setDacValue(int output);
  /// Set a DAC output raw value.
  /// \param output Output channel index.
  void setDacRValue(int output);
  /// Update the description display.
  /// \param description Device description.
  void getDescription(QString description);
  /// Update output count and mode.
  /// \param outputs Number of outputs.
  /// \param mode Mode or unit string.
  void getOutputs(int outputs, QString mode);
  /// Update output value displays.
  /// \param dacvalues Pointer to output values.
  void getOutputValues(QVector<float> *dacvalues);

private:
  /// Clear output UI widgets.
  void clearOutputs();

  QPushButton *connectButton; 
  QPushButton *resetButton; 
  QLabel      *dacvalueLabel;
  QLabel      *outputLabel;
  QLabel      *descriptionLabel; 
  QComboBox   *dacCombo;
  QVector<QLabel*>  *outputsList;
  QVector<QPushButton*> *setButtonList;
  QVector<QPushButton*> *shiftButtonList;
  QVector<QLineEdit*>   *dacvalueList;
  QVector<QLineEdit*>   *dacrvalueList;
  QVector<QString> *dacList;
  Dac* dac;
  QGridLayout *layout;
  QSignalMapper *signalMapper, *shiftsignalMapper;
};
#endif
