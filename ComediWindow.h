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

#ifndef COMEDIWINDOW_H
#define COMEDIWINDOW_H

#include "ComediCounterControlWidget.h"

#include <QtSql>
#include <QtGui>

class ComediWindow : public QMainWindow 
{
  
  Q_OBJECT
    
    public:
  
  ComediWindow( QMainWindow* parent = 0, Qt::WFlags fl = Qt::Window , Comedi *_comedi = 0);
  virtual ~ComediWindow();

  ComediCounterControlWidget *comediWidget;

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
  QLabel      *comedititle;
  QSqlTableModel *comeditable;
  QTableView *comediview;
  int comedirow;
  QVBoxLayout*                vboxlayout;
   
  QPushButton*                updateButton;
  QPushButton*                removeButton;
  QPushButton*                loadButton;
  
  void InitConfig();
  void InitRun();
  QVector<QString>      *comediList;
  QDockWidget *dockWidget;
  Comedi *comedi;
};
#endif // COMEDIWINDOW_H
