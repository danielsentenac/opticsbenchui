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
 * @file ComediDacControlWidget.h
 * @brief Comedi Dac control widget.
 */
#ifndef COMEDIDACCONTROLWIDGET_H
#define COMEDIDACCONTROLWIDGET_H

#include "Comedi.h"

#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"


typedef QVector<float> floatVector;

/// \ingroup ui_dac
/// UI widget for Comedi DAC control.
class ComediDacControlWidget : public QWidget
{
  Q_OBJECT
    
public:
  /// Construct a Comedi DAC control widget.
  /// \param comediList List of Comedi device names (optional).
  explicit ComediDacControlWidget(QVector<QString>* comediList = nullptr);
  /// Destructor.
  ~ComediDacControlWidget() override;
    
  /// Attach the Comedi controller.
  /// \param comedi Comedi controller instance.
  void setComedi(Comedi* comedi);
  /// Set the list of available Comedi devices.
  /// \param comediList List of Comedi device names.
  void setComediList(QVector<QString>* comediList);

private slots:
  /// Connect to the selected Comedi device.
  void connectComedi();
  /// Reset the selected device.
  void resetComedi();
  /// Set a DAC output value.
  /// \param output Output channel index.
  void setComediValue(int output);
  /// Set a DAC output raw value.
  /// \param output Output channel index.
  void setComediRValue(int output);
  /// Update the description display.
  /// \param description Device description.
  void getDescription(QString description);
  /// Update output count and mode.
  /// \param outputs Number of outputs.
  /// \param mode Mode or unit string.
  void getOutputs(int outputs, QString mode);
  /// Update output value displays.
  /// \param comedivalues Pointer to device-specific values.
  void getOutputValues(void *comedivalues);

private:
  /// Clear output UI widgets.
  void clearOutputs();

  QPushButton *connectButton; 
  QPushButton *resetButton; 
  QLabel      *comedivalueLabel;
  QLabel      *outputLabel;
  QLabel      *descriptionLabel; 
  QComboBox   *comediCombo;
  QVector<QLabel*>  *outputsList;
  QVector<QLabel*>  *unitsList;
  QVector<QPushButton*> *setButtonList;
  QVector<QPushButton*> *shiftButtonList;
  QVector<QLineEdit*>   *comedivalueList;
  QVector<QLineEdit*>   *comedirvalueList;
  QVector<QString> *comediList;
  Comedi  *comedi;
  QGridLayout *layout;
  QSignalMapper *signalMapper, *shiftsignalMapper;
};
#endif
