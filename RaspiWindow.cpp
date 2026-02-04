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
 * @file RaspiWindow.cpp
 * @brief Raspi window.
 */

#include "RaspiWindow.h"
#include "Utils.h"

RaspiWindow::RaspiWindow( QMainWindow* parent, Qt::WindowFlags fl , Raspi *_raspi)
  : QMainWindow( parent, fl )
{
  raspiWidget = NULL;
  raspi = _raspi;
  dbPath = raspi->path;
  QLOG_INFO() << "RaspiWindow::RaspiWindow> get Db path " << dbPath;
  QWidget *centralWidget = new QWidget(this);
  centralWidget->setMinimumSize(20,20);
  setCentralWidget(centralWidget);

  vboxlayout = new QVBoxLayout();
  centralWidget->setLayout(vboxlayout);

  raspiList = new QVector<QString> ();
  if ( raspi->raspitype == "RASPIDAC" ) {
     raspiWidget = new RaspiDacControlWidget(raspiList);
     raspiWidget->setRaspi(raspi);
     dockWidget = new QDockWidget(tr("Raspi Control"), this);
     dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
     QScrollArea *scrollableWidget = new  QScrollArea();
     scrollableWidget->setWidget(raspiWidget);
     scrollableWidget->setWidgetResizable(true);
     dockWidget->setWidget(scrollableWidget);
     this->addDockWidget(Qt::BottomDockWidgetArea, dockWidget);
  }

   // 'raspi_settings' table model
  raspititle = new QLabel("Raspi settings");
  vboxlayout->addWidget(raspititle);
  QSqlDatabase db = QSqlDatabase::database(dbPath);
  raspitable = new QSqlTableModel(this,db);
  raspiview = new QTableView;
  raspiview->setStyleSheet("QTreeView::item:selected{background-color: palette(highlight); color: palette(highlightedText);};");
  raspiview->setModel(raspitable);
  Utils::ConfigureSqlTableView(raspiview);
  vboxlayout->addWidget(raspiview);

  updateButton = new QPushButton("Update",this);
  updateButton->setFixedHeight(30);
  QObject::connect(updateButton, SIGNAL(clicked()), this, SLOT(update()));
  vboxlayout->addWidget(updateButton);
  removeButton = new QPushButton("Delete",this);
  removeButton->setFixedHeight(30);
  QObject::connect(removeButton, SIGNAL(clicked()), this, SLOT(remove()));
  vboxlayout->addWidget(removeButton);
  loadButton = new QPushButton("Load",this);
  loadButton->setFixedHeight(30);
  QObject::connect(loadButton, SIGNAL(clicked()), this, SLOT(load()));
  vboxlayout->addWidget(loadButton);
  this->setWindowTitle("Raspi");

  raspitable->setTable("raspi_settings");
  raspitable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  raspitable->setHeaderData(0, Qt::Horizontal, tr("name"));
  raspitable->setHeaderData(1, Qt::Horizontal, tr("settings"));
  raspitable->setHeaderData(2, Qt::Horizontal, tr("description"));
  raspitable->setHeaderData(3, Qt::Horizontal, tr("raspivalues"));
  raspiview->setColumnHidden (3, true);
  InitConfig();

} 
RaspiWindow::~RaspiWindow()
{
  QLOG_DEBUG ( ) << "Deleting RaspiWindow";
  if (raspitable) delete raspitable;
  if (raspiWidget) delete raspiWidget;
}
void RaspiWindow::setDbPath(QString _path) {

  if (raspitable) delete raspitable;

  // Re_Init object
  
  raspi->setDbPath(_path);
  dbPath = raspi->path;
  raspitable = new QSqlTableModel(this,QSqlDatabase::database(dbPath));
  raspiview->setModel(raspitable);
  Utils::ConfigureSqlTableView(raspiview);
  raspitable->setTable("raspi_settings");
  raspitable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  raspitable->setHeaderData(0, Qt::Horizontal, tr("name"));
  raspitable->setHeaderData(1, Qt::Horizontal, tr("settings"));
  raspitable->setHeaderData(2, Qt::Horizontal, tr("description"));
  raspitable->setHeaderData(3, Qt::Horizontal, tr("raspivalues"));
  raspiview->setColumnHidden (3, true);
  InitConfig();
}
void RaspiWindow::closeEvent(QCloseEvent* event)
{
  event->accept();  
  QLOG_DEBUG ( ) << "Closing RaspiWindow";
  this->hide();
}

void RaspiWindow::InitConfig() {
   // Set where clause
  // QLineEdit shows person lastname that id is given parameter
  //raspitable->setFilter(QString("name=*").arg(name));
  raspitable->select();
  raspirow = raspitable->rowCount();
  raspitable->insertRow(raspirow);
  raspiview->resizeColumnsToContents();
  raspiview->resizeRowsToContents();

}

void 
RaspiWindow::update(){
  raspitable->submitAll();
  InitConfig();
}
void 
RaspiWindow::load(){
  InitRun();
}
void 
RaspiWindow::remove(){
  QModelIndexList selected;
  QItemSelectionModel *selmodel;
  selmodel = raspiview->selectionModel();
  selected = selmodel->selectedIndexes();
  for (int i = selected.size() - 1 ; i >= 0; i--) 
    raspitable->removeRow(selected.at(i).row());
  raspitable->submitAll();
  InitConfig();
}
void 
RaspiWindow::InitRun(){
  raspiList->clear();
  QSqlDatabase db = QSqlDatabase::database(dbPath);
  QSqlQuery query(db);
  query.exec("select name from raspi_settings");
  while (query.next()) {
    QString name = query.value(0).toString();
    raspiList->append(name);
  }
  if ( raspi->raspitype == "RASPICOUNTER" )
    raspiWidget->setRaspiList(raspiList);
  else if ( raspi->raspitype == "RASPIDAC" )
    raspiWidget->setRaspiList(raspiList);
}
