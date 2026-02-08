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
 * @file MotorWindow.h
 * @brief Motor window.
 */

#ifndef MOTORWINDOW_H
#define MOTORWINDOW_H

#include "MotorControlWidget.h"

#include <QMessageBox>
#include <QtSql>
#include <QtWidgets>

/// \ingroup ui_motor
/// Window for motor configuration and control.
class MotorWindow : public QMainWindow 
{
  
  Q_OBJECT
    
    public:
  
  /// Construct a motor window.
  /// \param parent Parent window.
  /// \param fl Window flags.
  /// \param _motor Motor controller instance.
  MotorWindow( QMainWindow* parent = 0, Qt::WindowFlags fl = Qt::Window , Motor *_motor = 0);
  /// Destructor.
  virtual ~MotorWindow();

  /// Motor control widget instance.
  MotorControlWidget *motorWidget;

 public slots:
   /// Update database-backed tables.
   void update();
   /// Remove selected entries.
   void remove();
   /// Load configuration into runtime.
   void load();
   /// Set the database path for tables.
   /// \param _path Filesystem path to the motor DB.
   void setDbPath(QString _path);
   /// Refresh actuator table to show latest positions.
   void refreshActuatorTable();

 protected:
  /// Handle window close events.
  /// \param event Close event.
  void closeEvent(QCloseEvent *event);
    
 private:

  QMainWindow *parentWindow;
  QString     dbPath;
  QLabel      *comtitle;
  QSqlTableModel *comtable;
  QTableView *comview;
  QLabel      *drivertitle;
  QSqlTableModel *drivertable;
  QTableView *driverview;
  QLabel      *actuatortitle;
  QSqlTableModel *actuatortable;
  QTableView *actuatorview;
  int comrow,driverrow,actuatorrow;
  QVBoxLayout*  vboxlayout;
   
  QPushButton*  updateButton;
  QPushButton*  removeButton;
  QPushButton*  loadButton;
  
  void InitConfig();
  void InitRun();
  QVector<QString>      *actuatorList;
  QDockWidget *dockWidget;
  Motor *motor;
};
#endif // MOTORWINDOW_H
