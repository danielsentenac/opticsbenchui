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
#ifndef DACCOMEDICONTROLWIDGET_H
#define DACCOMEDICONTROLWIDGET_H

#include "Comedi.h"

#include <QtSql>
#include <QtGui>
#include "QsLog.h"


typedef QVector<float> floatVector;

class ComediDacControlWidget : public QWidget
{
  Q_OBJECT
    
    public:
  ComediDacControlWidget(QVector<QString>  *_comediList = 0);
  ~ComediDacControlWidget();
    
  void setComedi(Comedi *_comedi);
  void setComediList(QVector<QString>  *_comediList);

  private slots:
  void connectComedi();
  void resetComedi();
  void setComediValue(int output);
  void getDescription(QString description);
  void getOutputs(int outputs, QString mode);
  void getOutputValues(void *comedivalues);

 private:
  QPushButton *connectButton; 
  QPushButton *resetButton; 
  QLabel      *comedivalueLabel;
  QLabel      *outputLabel;
  QLabel      *descriptionLabel; 
  QComboBox   *comediCombo;
  QVector<QLabel*>  *outputsList;
  QVector<QLabel*>  *unitsList;
  QVector<QPushButton*> *setButtonList;
  QVector<QLineEdit*>   *comedivalueList;
  QVector<QLineEdit*>   *comeditimerList;
  QVector<QString> *comediList;
  Comedi  *comedi;
  QGridLayout *layout;
  QSignalMapper *signalMapper;
};
#endif
