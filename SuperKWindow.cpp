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
 * @file SuperKWindow.cpp
 * @brief Super K window.
 */

#include "SuperKWindow.h"
#include "Utils.h"

SuperKWindow::SuperKWindow( QMainWindow* parent, Qt::WindowFlags fl , SuperK *_superk)
  : QMainWindow( parent, fl )
{
  superk = _superk;
  
  dbPath = superk->path;
   
  QWidget *centralWidget = new QWidget(this);
 
  setCentralWidget(centralWidget);

  vboxlayout = new QVBoxLayout();
  centralWidget->setLayout(vboxlayout);
 
  driverList = new QVector<QString> (); 
  superkWidget = new SuperKControlWidget(driverList);
  superkWidget->setSuperK(superk);
  dockWidget = new QDockWidget(tr("Driver Control"), this);
  dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
  dockWidget->setWidget(superkWidget);
  this->addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

  // 'superk_driver' table model
  drivertitle = new QLabel("Driver settings");
  vboxlayout->addWidget(drivertitle);
  drivertable = new QSqlTableModel(this,QSqlDatabase::database(dbPath));
  driverview = new QTableView;
  driverview->setStyleSheet("QTreeView::item:selected{background-color: palette(highlight); color: palette(highlightedText);};");
  driverview->setModel(drivertable);
  Utils::ConfigureSqlTableView(driverview);
  vboxlayout->addWidget(driverview);

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
  this->setWindowTitle("SuperK");

  
  drivertable->setTable("superk_driver");
  drivertable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  drivertable->setHeaderData(0, Qt::Horizontal, tr("name"));
  drivertable->setHeaderData(1, Qt::Horizontal, tr("comtype"));
  drivertable->setHeaderData(2, Qt::Horizontal, tr("comaddress"));
  drivertable->setHeaderData(3, Qt::Horizontal, tr("comsettings"));
  drivertable->setHeaderData(4, Qt::Horizontal, tr("drvsettings"));
  drivertable->setHeaderData(5, Qt::Horizontal, tr("description"));
  drivertable->setHeaderData(6, Qt::Horizontal, tr("data"));
  driverview->setColumnHidden (4, true);
  driverview->setColumnHidden (6, true);

  InitConfig();
} 

SuperKWindow::~SuperKWindow()
{
  QLOG_DEBUG ( ) << "Deleting SuperKWindow";
  if (drivertable) delete drivertable;
  if (superk) delete superk;
}
void SuperKWindow::setDbPath(QString _path) {

  if (drivertable) delete drivertable;

  // Re_Init object
  
  superk->setDbPath(_path);
  dbPath = superk->path;
  drivertable = new QSqlTableModel(this,QSqlDatabase::database(dbPath));
  driverview->setModel(drivertable);
  Utils::ConfigureSqlTableView(driverview);
  
  drivertable->setTable("superk_driver");
  drivertable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  drivertable->setHeaderData(0, Qt::Horizontal, tr("name"));
  drivertable->setHeaderData(1, Qt::Horizontal, tr("comtype"));
  drivertable->setHeaderData(2, Qt::Horizontal, tr("comaddress"));
  drivertable->setHeaderData(3, Qt::Horizontal, tr("comsettings"));
  drivertable->setHeaderData(4, Qt::Horizontal, tr("drvsettings"));
  drivertable->setHeaderData(5, Qt::Horizontal, tr("description"));
  drivertable->setHeaderData(6, Qt::Horizontal, tr("data"));
  driverview->setColumnHidden (4, true);
  driverview->setColumnHidden (6, true);

  InitConfig();
}
void SuperKWindow::closeEvent(QCloseEvent* event)
{
  event->accept();  
  QLOG_DEBUG ( ) << "Closing SuperKWindow";
  this->hide();
}

void SuperKWindow::InitConfig() {
  
  drivertable->select();
  driverrow = drivertable->rowCount();
  drivertable->insertRow(driverrow);
  driverview->resizeColumnsToContents();
  driverview->resizeRowsToContents();

}

void 
SuperKWindow::update(){
  drivertable->submitAll();
  InitConfig();
}
void 
SuperKWindow::load(){
  InitRun();
}
void 
SuperKWindow::remove(){
  QModelIndexList selected;
  QItemSelectionModel *selmodel;
  selmodel = driverview->selectionModel();
  selected = selmodel->selectedIndexes();
  for (int i = selected.size() - 1 ; i >= 0; i--) 
    drivertable->removeRow(selected.at(i).row());
  drivertable->submitAll();
  InitConfig();
}
void 
SuperKWindow::InitRun(){
  driverList->clear();
  QSqlQuery query(QSqlDatabase::database(dbPath));
  query.exec("select name from superk_driver");
  while (query.next()) {
    QString name = query.value(0).toString();
    driverList->append(name);
  }
  superkWidget->setDriverList(driverList);

}
