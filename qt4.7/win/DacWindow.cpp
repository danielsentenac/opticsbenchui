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

#include "DacWindow.h"

DacWindow::DacWindow( QMainWindow* parent, Qt::WFlags fl , Dac *_dac)
  : QMainWindow( parent, fl )
{
  dac = _dac;
  dbPath = dac->path;
  QLOG_INFO() << "DacWindow::DacWindow> get Db path " << dbPath;
  QWidget *centralWidget = new QWidget(this);
 
  setCentralWidget(centralWidget);

  vboxlayout = new QVBoxLayout();
  centralWidget->setLayout(vboxlayout);

  dacList = new QVector<QString> ();
  dacWidget = new DacControlWidget(dacList);
  dacWidget->setDac(dac);
  dockWidget = new QDockWidget(tr("Dac Control"), this);
  dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
  QScrollArea *scrollableWidget = new  QScrollArea();
  scrollableWidget->setWidget(dacWidget);
  scrollableWidget->setWidgetResizable(true);
  dockWidget->setWidget(scrollableWidget);
  this->addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

   // 'dac_settings' table model
  dactitle = new QLabel("Dac settings");
  vboxlayout->addWidget(dactitle);
  QSqlDatabase db = QSqlDatabase::database(dbPath);
  dactable = new QSqlTableModel(this,db);
  dacview = new QTableView;
  dacview->setModel(dactable);
  vboxlayout->addWidget(dacview);

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
  this->setWindowTitle("Dac");

  dactable->setTable("dac_settings");
  dactable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  dactable->setHeaderData(0, Qt::Horizontal, tr("name"));
  dactable->setHeaderData(1, Qt::Horizontal, tr("settings"));
  dactable->setHeaderData(2, Qt::Horizontal, tr("description"));
  dactable->setHeaderData(3, Qt::Horizontal, tr("dacvalues"));
  dacview->setColumnHidden (3, true);
  InitConfig();

} 
DacWindow::~DacWindow()
{
  QLOG_DEBUG ( ) << "Deleting DacWindow";
  if (dactable) delete dactable;
  if (dac) delete dac;
}
void DacWindow::setDbPath(QString _path) {

  if (dactable) delete dactable;

  // Re_Init object
  
  dac->setDbPath(_path);
  dbPath = dac->path;
  dactable = new QSqlTableModel(this,QSqlDatabase::database(dbPath));
  dacview->setModel(dactable);
  dactable->setTable("dac_settings");
  dactable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  dactable->setHeaderData(0, Qt::Horizontal, tr("name"));
  dactable->setHeaderData(1, Qt::Horizontal, tr("settings"));
  dactable->setHeaderData(2, Qt::Horizontal, tr("description"));
  dactable->setHeaderData(3, Qt::Horizontal, tr("dacvalues"));
  dacview->setColumnHidden (3, true);
  InitConfig();
}
void DacWindow::closeEvent(QCloseEvent* event)
{
  event->accept();  
  QLOG_DEBUG ( ) << "Closing DacWindow";
  this->hide();
}

void DacWindow::InitConfig() {
   // Set where clause
  // QLineEdit shows person lastname that id is given parameter
  //dactable->setFilter(QString("name=*").arg(name));
  dactable->select();
  dacrow = dactable->rowCount();
  dactable->insertRow(dacrow);
  dacview->resizeColumnsToContents();
  dacview->resizeRowsToContents();

}

void 
DacWindow::update(){
  dactable->submitAll();
  InitConfig();
}
void 
DacWindow::load(){
  InitRun();
}
void 
DacWindow::remove(){
  QModelIndexList selected;
  QItemSelectionModel *selmodel;
  selmodel = dacview->selectionModel();
  selected = selmodel->selectedIndexes();
  for (int i = selected.size() - 1 ; i >= 0; i--) 
    dactable->removeRow(selected.at(i).row());
  dactable->submitAll();
  InitConfig();
}
void 
DacWindow::InitRun(){
  dacList->clear();
  QSqlDatabase db = QSqlDatabase::database(dbPath);
  QSqlQuery query(db);
  query.exec("select name from dac_settings");
  while (query.next()) {
    QString name = query.value(0).toString();
    dacList->append(name);
  }
  dacWidget->setDacList(dacList);
}
