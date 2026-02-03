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
#include "ComediDacControlWidget.h"

#include <QtSql>
#include <QtWidgets>

/// \ingroup ui
/// Window for Comedi device configuration and control.
class ComediWindow : public QMainWindow 
{
  
  Q_OBJECT
    
public:
  /// Construct a Comedi window.
  /// \param parent Parent window.
  /// \param fl Window flags.
  /// \param comedi Comedi controller instance.
  ComediWindow(QMainWindow* parent = nullptr, Qt::WindowFlags fl = Qt::Window,
               Comedi *comedi = nullptr);
  /// Destructor.
  ~ComediWindow() override;
  
  /// Comedi counter control widget.
  ComediCounterControlWidget *comediWidget;
  /// Comedi DAC control widget.
  ComediDacControlWidget *daccomediWidget;

public slots:
  /// Update database-backed tables.
  void update();
  /// Remove selected entries.
  void remove();
  /// Load configuration into runtime.
  void load();
  /// Set the database path for tables.
  /// \param path Filesystem path to the Comedi DB.
  void setDbPath(QString path);

protected:
  /// Handle window close events.
  /// \param event Close event.
  void closeEvent(QCloseEvent *event) override;
    
private:
  void InitConfig();
  void InitRun();
  void setupTableModel();
  void setupDockWidget();

  QMainWindow *parentWindow;
  QString dbPath;
  QLabel      *comedititle;
  QSqlTableModel *comeditable;
  QTableView *comediview;
  int comedirow;
  QVBoxLayout* vboxlayout;
   
  QPushButton* updateButton;
  QPushButton* removeButton;
  QPushButton* loadButton;

  QVector<QString>* comediList;
  QDockWidget *dockWidget;
  Comedi *comedi;
};
#endif // COMEDIWINDOW_H
