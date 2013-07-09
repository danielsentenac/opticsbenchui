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

#include "ComediWindow.h"

ComediWindow::ComediWindow( QMainWindow* parent, Qt::WFlags fl , Comedi *_comedi)
  : QMainWindow( parent, fl )
{
  comedi = _comedi;
  dbPath = comedi->path;
  QLOG_INFO() << "ComediWindow::ComediWindow> get Db path " << dbPath;
  QWidget *centralWidget = new QWidget(this);
 
  setCentralWidget(centralWidget);

  vboxlayout = new QVBoxLayout();
  centralWidget->setLayout(vboxlayout);

  comediList = new QVector<QString> ();
  if ( comedi->comeditype == "COMEDICOUNTER" ) {
     comediWidget = new ComediCounterControlWidget(comediList);
     comediWidget->setComedi(comedi);
     dockWidget = new QDockWidget(tr("Comedi Control"), this);
     dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
     QScrollArea *scrollableWidget = new  QScrollArea();
     scrollableWidget->setWidget(comediWidget);
     scrollableWidget->setWidgetResizable(true);
     dockWidget->setWidget(scrollableWidget);
     this->addDockWidget(Qt::BottomDockWidgetArea, dockWidget);
  }
  else if ( comedi->comeditype == "COMEDIDAC" ) {
     daccomediWidget = new ComediDacControlWidget(comediList);
     daccomediWidget->setComedi(comedi);
     dockWidget = new QDockWidget(tr("Comedi Control"), this);
     dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
     QScrollArea *scrollableWidget = new  QScrollArea();
     scrollableWidget->setWidget(daccomediWidget);
     scrollableWidget->setWidgetResizable(true);
     dockWidget->setWidget(scrollableWidget);
     this->addDockWidget(Qt::BottomDockWidgetArea, dockWidget);
  }

   // 'comedi_settings' table model
  comedititle = new QLabel("Comedi settings");
  vboxlayout->addWidget(comedititle);
  QSqlDatabase db = QSqlDatabase::database(dbPath);
  comeditable = new QSqlTableModel(this,db);
  comediview = new QTableView;
  comediview->setModel(comeditable);
  vboxlayout->addWidget(comediview);

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
  this->setWindowTitle("Comedi");

  comeditable->setTable("comedi_settings");
  comeditable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  comeditable->setHeaderData(0, Qt::Horizontal, tr("name"));
  comeditable->setHeaderData(1, Qt::Horizontal, tr("settings"));
  comeditable->setHeaderData(2, Qt::Horizontal, tr("description"));
  comeditable->setHeaderData(3, Qt::Horizontal, tr("comedivalues"));
  comediview->setColumnHidden (3, true);
  InitConfig();

} 
ComediWindow::~ComediWindow()
{
  QLOG_DEBUG ( ) << "Deleting ComediWindow";
  if (comeditable) delete comeditable;
  if (comedi) delete comedi;
}
void ComediWindow::setDbPath(QString _path) {

  if (comeditable) delete comeditable;

  // Re_Init object
  
  comedi->setDbPath(_path);
  dbPath = comedi->path;
  comeditable = new QSqlTableModel(this,QSqlDatabase::database(dbPath));
  comediview->setModel(comeditable);
  comeditable->setTable("comedi_settings");
  comeditable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  comeditable->setHeaderData(0, Qt::Horizontal, tr("name"));
  comeditable->setHeaderData(1, Qt::Horizontal, tr("settings"));
  comeditable->setHeaderData(2, Qt::Horizontal, tr("description"));
  comeditable->setHeaderData(3, Qt::Horizontal, tr("comedivalues"));
  comediview->setColumnHidden (3, true);
  InitConfig();
}
void ComediWindow::closeEvent(QCloseEvent* event)
{
  event->accept();  
  QLOG_DEBUG ( ) << "Closing ComediWindow";
  this->hide();
}

void ComediWindow::InitConfig() {
   // Set where clause
  // QLineEdit shows person lastname that id is given parameter
  //comeditable->setFilter(QString("name=*").arg(name));
  comeditable->select();
  comedirow = comeditable->rowCount();
  comeditable->insertRow(comedirow);
  comediview->resizeColumnsToContents();
  comediview->resizeRowsToContents();

}

void 
ComediWindow::update(){
  comeditable->submitAll();
  InitConfig();
}
void 
ComediWindow::load(){
  InitRun();
}
void 
ComediWindow::remove(){
  QModelIndexList selected;
  QItemSelectionModel *selmodel;
  selmodel = comediview->selectionModel();
  selected = selmodel->selectedIndexes();
  for (int i = selected.size() - 1 ; i >= 0; i--) 
    comeditable->removeRow(selected.at(i).row());
  comeditable->submitAll();
  InitConfig();
}
void 
ComediWindow::InitRun(){
  comediList->clear();
  QSqlDatabase db = QSqlDatabase::database(dbPath);
  QSqlQuery query(db);
  query.exec("select name from comedi_settings");
  while (query.next()) {
    QString name = query.value(0).toString();
    comediList->append(name);
  }
  if ( comedi->comeditype == "COMEDICOUNTER" )
    comediWidget->setComediList(comediList);
  else if ( comedi->comeditype == "COMEDIDAC" )
    daccomediWidget->setComediList(comediList);
}
