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
#ifndef DACRASPICONTROLWIDGET_H
#define DACRASPICONTROLWIDGET_H

#include "RaspiDac.h"

#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"


typedef QVector<float> floatVector;

class RaspiDacControlWidget : public QWidget
{
  Q_OBJECT
    
    public:
  RaspiDacControlWidget(QVector<QString>  *_raspiList = 0);
  ~RaspiDacControlWidget();
    
  void setRaspi(RaspiDac *_raspi);
  void setRaspiList(QVector<QString>  *_raspiList);

  private slots:
  void connectRaspi();
  void resetRaspi();
  void setRaspiValue(int output);
  void setRaspiRValue(int output);
  void getDescription(QString description);
  void getOutputs(int outputs);
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
  RaspiDac  *raspi;
  QGridLayout *layout;
  QSignalMapper *signalMapper, *shiftsignalMapper;
};
#endif
