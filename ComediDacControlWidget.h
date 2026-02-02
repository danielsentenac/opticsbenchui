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
#ifndef COMEDIDACCONTROLWIDGET_H
#define COMEDIDACCONTROLWIDGET_H

#include "Comedi.h"

#include <QtSql>
#include <QtWidgets>
#include "QsLog.h"


typedef QVector<float> floatVector;

class ComediDacControlWidget : public QWidget
{
  Q_OBJECT
    
public:
  explicit ComediDacControlWidget(QVector<QString>* comediList = nullptr);
  ~ComediDacControlWidget() override;
    
  void setComedi(Comedi* comedi);
  void setComediList(QVector<QString>* comediList);

private slots:
  void connectComedi();
  void resetComedi();
  void setComediValue(int output);
  void setComediRValue(int output);
  void getDescription(QString description);
  void getOutputs(int outputs, QString mode);
  void getOutputValues(void *comedivalues);

private:
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
