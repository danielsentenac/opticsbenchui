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
 * @file RaspiWindow.h
 * @brief Raspi window.
 */
#ifndef RASPIWINDOW_H
#define RASPIWINDOW_H

#include "RaspiDacControlWidget.h"

#include <QtSql>
#include <QtWidgets>

/// \ingroup ui
/// Window for Raspi DAC configuration and control.
class RaspiWindow : public QMainWindow 
{
  
  Q_OBJECT
    
    public:
  
  /// Construct a Raspi window.
  /// \param parent Parent window.
  /// \param fl Window flags.
  /// \param _raspi Raspi controller instance.
  RaspiWindow( QMainWindow* parent = 0, Qt::WindowFlags fl = Qt::Window , Raspi *_raspi = 0);
  /// Destructor.
  virtual ~RaspiWindow();

  /// Raspi DAC control widget instance.
  RaspiDacControlWidget *raspiWidget;

 public slots:
   /// Update database-backed tables.
   void update();
   /// Remove selected entries.
   void remove();
   /// Load configuration into runtime.
   void load();
   /// Set the database path for tables.
   /// \param _path Filesystem path to the Raspi DB.
   void setDbPath(QString _path);

 protected:
  /// Handle window close events.
  /// \param event Close event.
  void closeEvent(QCloseEvent *event);
    
 private:

  QMainWindow *parentWindow;
  QString dbPath;
  QLabel      *raspititle;
  QSqlTableModel *raspitable;
  QTableView *raspiview;
  int raspirow;
  QVBoxLayout*                vboxlayout;
   
  QPushButton*                updateButton;
  QPushButton*                removeButton;
  QPushButton*                loadButton;
  
  void InitConfig();
  void InitRun();
  QVector<QString>      *raspiList;
  QDockWidget *dockWidget;
  Raspi *raspi;
};
#endif // RASPIWINDOW_H
