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
 * @file SuperKWindow.h
 * @brief Super K window.
 */

#ifndef SUPERKWINDOW_H
#define SUPERKWINDOW_H

#include "SuperKControlWidget.h"

#include <QMessageBox>
#include <QtSql>
#include <QtWidgets>

/// \ingroup ui_lasers
/// Window for SuperK configuration and control.
class SuperKWindow : public QMainWindow 
{
  
  Q_OBJECT
    
    public:
  
  /// Construct a SuperK window.
  /// \param parent Parent window.
  /// \param fl Window flags.
  /// \param _superk SuperK controller instance.
  SuperKWindow( QMainWindow* parent = 0, Qt::WindowFlags fl = Qt::Window , SuperK *_superk = 0);
  /// Destructor.
  virtual ~SuperKWindow();

  /// SuperK control widget instance.
  SuperKControlWidget *superkWidget;

 public slots:
   /// Update database-backed tables.
   void update();
   /// Remove selected entries.
   void remove();
   /// Load configuration into runtime.
   void load();
   /// Set the database path for tables.
   /// \param _path Filesystem path to the SuperK DB.
   void setDbPath(QString _path);

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
  int comrow,driverrow;
  QVBoxLayout*  vboxlayout;
   
  QPushButton*  updateButton;
  QPushButton*  removeButton;
  QPushButton*  loadButton;
  
  void InitConfig();
  void InitRun();
  QVector<QString>  *driverList;
  QDockWidget *dockWidget;
  SuperK *superk;
};
#endif // SUPERKWINDOW_H
