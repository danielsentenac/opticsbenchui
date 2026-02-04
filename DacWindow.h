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
 * @file DacWindow.h
 * @brief Dac window.
 */

#ifndef DACWINDOW_H
#define DACWINDOW_H

#include "DacControlWidget.h"

#include <QtSql>
#include <QtWidgets>

/// \ingroup ui
/// Window for DAC configuration and control.
class DacWindow : public QMainWindow 
{
  
  Q_OBJECT
    
public:
  /// Construct a DAC window.
  /// \param parent Parent window.
  /// \param fl Window flags.
  /// \param dac DAC controller instance.
  DacWindow(QMainWindow* parent = nullptr, Qt::WindowFlags fl = Qt::Window,
            Dac *dac = nullptr);
  /// Destructor.
  ~DacWindow() override;
  
  /// DAC control widget instance.
  DacControlWidget *dacWidget;

public slots:
  /// Update database-backed tables.
  void update();
  /// Remove selected entries.
  void remove();
  /// Load configuration into runtime.
  void load();
  /// Set the database path for tables.
  /// \param path Filesystem path to the DAC DB.
  void setDbPath(QString path);

protected:
  /// Handle window close events.
  /// \param event Close event.
  void closeEvent(QCloseEvent *event) override;
    
private:
  void InitConfig();
  void InitRun();
  void setupTableModel();

  QMainWindow *parentWindow;
  QString dbPath;
  QLabel      *dactitle;
  QSqlTableModel *dactable;
  QTableView *dacview;
  int dacrow;
  QVBoxLayout* vboxlayout;
   
  QPushButton* updateButton;
  QPushButton* removeButton;
  QPushButton* loadButton;

  QVector<QString>* dacList;
  QDockWidget *dockWidget;
  Dac *dac;
};
#endif // DACWINDOW_H
