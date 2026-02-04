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
 * @file ComediCounterControlWidget.h
 * @brief Comedi Counter control widget.
 */
#ifndef COMEDICOUNTERCONTROLWIDGET_H
#define COMEDICOUNTERCONTROLWIDGET_H

#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"
#include <qtconcurrentrun.h>
#include "Comedi.h"
#include "ComediCounterPlot.h"

typedef QVector<float> floatVector;

/// \ingroup ui
/// UI widget for Comedi counter control and plotting.
class ComediCounterControlWidget : public QWidget
{
  Q_OBJECT
    
public:
  /// Construct a Comedi counter control widget.
  /// \param comediList List of Comedi device names (optional).
  explicit ComediCounterControlWidget(QVector<QString>* comediList = nullptr);
  /// Destructor.
  ~ComediCounterControlWidget() override;
    
  /// Attach the Comedi controller.
  /// \param comedi Comedi controller instance.
  void setComedi(Comedi* comedi);
  /// Set the list of available Comedi devices.
  /// \param comediList List of Comedi device names.
  void setComediList(QVector<QString>* comediList);

  /// Flag to request stopping the background future.
  bool stopFuture;
  /// List of per-output set buttons.
  QVector<QPushButton*> *setButtonList;

private slots:
  /// Connect to the selected Comedi device.
  void connectComedi();
  /// Reset the selected device.
  void resetComedi();
  /// Set a counter value.
  /// \param output Output channel index.
  void setComediValue(int output);
  /// Start continuous counting.
  /// \param output Output channel index.
  void setComediContinuousValue(int output);
  /// Start counting on a channel.
  /// \param output Output channel index.
  void startCounting(int output);
  /// Start plotting for a channel.
  /// \param output Output channel index.
  void startPlot(int output) ;
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
  /// Clear the plot display.
  void clearPlot();

  QPushButton *connectButton; 
  QPushButton *resetButton; 
  QLabel      *comedivalueLabel;
  QLabel      *outputLabel;
  QLabel      *descriptionLabel; 
  QComboBox   *comediCombo;
  QVector<QLabel*>  *outputsList;
  QVector<QLabel*>  *unitsList;
  QVector<QLineEdit*>   *comedivalueList;
  QVector<QLineEdit*>   *comeditimerList;
  QVector<QString> *comediList;
  Comedi  *comedi;
  QGridLayout *layout;
  QSignalMapper *signalMapper;
  ComediCounterPlot *plot;
  QVBoxLayout *boxlayout;
  QWidget *vBox;
  QFuture<void> future;
};
#endif
