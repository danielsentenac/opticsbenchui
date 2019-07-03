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
#ifndef DACCONTROLWIDGET_H
#define DACCONTROLWIDGET_H

#include "Dac.h"

#include <QtSql>
#include <QtGui>
#include "QsLog.h"


typedef QVector<float> floatVector;

class DacControlWidget : public QWidget
{
  Q_OBJECT
    
    public:
  DacControlWidget(QVector<QString>  *_dacList = 0);
  ~DacControlWidget();
    
  void setDac(Dac *_dac);
  void setDacList(QVector<QString>  *_dacList);

  private slots:
  void connectDac();
  void resetDac();
  void setDacValue(int output);
  void setDacRValue(int output);
  void getDescription(QString description);
  void getOutputs(int outputs, QString mode);
  void getOutputValues(QVector<float> *dacvalues);

 private:
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
  Dac  *dac;
  QGridLayout *layout;
  QSignalMapper *signalMapper, *shiftsignalMapper;
};
#endif
