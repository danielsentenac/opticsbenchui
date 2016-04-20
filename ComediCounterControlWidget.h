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
#ifndef COMEDICONTROLWIDGET_H
#define COMEDICONTROLWIDGET_H

#include <QtSql>
#include <QtGui>
#include "QsLog.h"
#include <qtconcurrentrun.h>
#include "ComediCounterPlot.h"

using namespace QtConcurrent;


typedef QVector<float> floatVector;

class ComediCounterControlWidget : public QWidget
{
  Q_OBJECT
    
    public:
  ComediCounterControlWidget(QVector<QString>  *_comediList = 0);
  ~ComediCounterControlWidget();
    
  void setComedi(Comedi *_comedi);
  void setComediList(QVector<QString>  *_comediList);

  bool stopFuture;
  QVector<QPushButton*> *setButtonList;

  private slots:
  void connectComedi();
  void resetComedi();
  void setComediValue(int output);
  void startCounting(int output);
  void startPlot(int output) ;
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
