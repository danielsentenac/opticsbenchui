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

#ifndef DACWINDOW_H
#define DACWINDOW_H

#include "DacControlWidget.h"

#include <QtSql>
#include <QtWidgets>

class DacWindow : public QMainWindow 
{
  
  Q_OBJECT
    
    public:
  
  DacWindow( QMainWindow* parent = 0, Qt::WindowFlags fl = Qt::Window , Dac *_dac = 0);
  virtual ~DacWindow();

  DacControlWidget *dacWidget;

 public slots:
   void update();
   void remove();
   void load();
   void setDbPath(QString _path);

 protected:
  void closeEvent(QCloseEvent *event);
    
 private:

  QMainWindow *parentWindow;
  QString dbPath;
  QLabel      *dactitle;
  QSqlTableModel *dactable;
  QTableView *dacview;
  int dacrow;
  QVBoxLayout*                vboxlayout;
   
  QPushButton*                updateButton;
  QPushButton*                removeButton;
  QPushButton*                loadButton;
  
  void InitConfig();
  void InitRun();
  QVector<QString>      *dacList;
  QDockWidget *dockWidget;
  Dac *dac;
};
#endif // DACWINDOW_H
