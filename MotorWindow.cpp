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

#include "MotorWindow.h"
#include "Utils.h"

MotorWindow::MotorWindow( QMainWindow* parent, Qt::WindowFlags fl , Motor *_motor)
  : QMainWindow( parent, fl )
{
  motor = _motor;
  
  dbPath = motor->path;
   
  QWidget *centralWidget = new QWidget(this);
 
  setCentralWidget(centralWidget);
  centralWidget->setMinimumSize(20,20);
  vboxlayout = new QVBoxLayout();
  centralWidget->setLayout(vboxlayout);
  
  actuatorList = new QVector<QString> ();
  motorWidget = new MotorControlWidget(actuatorList);
  motorWidget->setMotor(motor);
  dockWidget = new QDockWidget(tr("Actuator Control"), this);
  dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
  QScrollArea *scrollableWidget = new  QScrollArea();
  scrollableWidget->setWidget(motorWidget);
  scrollableWidget->setWidgetResizable(true);
  dockWidget->setWidget(scrollableWidget);
  this->addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

  // 'motor_com' table model
  comtitle = new QLabel("Communication settings");
  vboxlayout->addWidget(comtitle);
  comtable = new QSqlTableModel(this,QSqlDatabase::database(dbPath));
  comview = new QTableView;
  comview->setStyleSheet("QTreeView::item:selected{background-color: palette(highlight); color: palette(highlightedText);};");
  comview->setModel(comtable);
  Utils::ConfigureSqlTableView(comview);
  vboxlayout->addWidget(comview);

   // 'motor_driver' table model
  drivertitle = new QLabel("Driver settings");
  vboxlayout->addWidget(drivertitle);
  drivertable = new QSqlTableModel(this,QSqlDatabase::database(dbPath));
  driverview = new QTableView;
  driverview->setStyleSheet("QTreeView::item:selected{background-color: palette(highlight); color: palette(highlightedText);};");
  driverview->setModel(drivertable);
  Utils::ConfigureSqlTableView(driverview);
  vboxlayout->addWidget(driverview);

  // 'motor_actuator' table model
  actuatortitle = new QLabel("Actuator settings");
  vboxlayout->addWidget(actuatortitle);
  actuatortable = new QSqlTableModel(this,QSqlDatabase::database(dbPath));
  actuatorview = new QTableView;
  actuatorview->setStyleSheet("QTreeView::item:selected{background-color: palette(highlight); color: palette(highlightedText);};");
  actuatorview->setModel(actuatortable);
  Utils::ConfigureSqlTableView(actuatorview);
  vboxlayout->addWidget(actuatorview);

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
  this->setWindowTitle("Motor");

  
  comtable->setTable("motor_com");
  comtable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  comtable->setHeaderData(0, Qt::Horizontal, tr("name"));
  comtable->setHeaderData(1, Qt::Horizontal, tr("type"));
  comtable->setHeaderData(2, Qt::Horizontal, tr("address"));
  comtable->setHeaderData(3, Qt::Horizontal, tr("settings"));
  
  drivertable->setTable("motor_driver");
  drivertable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  drivertable->setHeaderData(0, Qt::Horizontal, tr("name"));
  drivertable->setHeaderData(1, Qt::Horizontal, tr("com"));
  drivertable->setHeaderData(2, Qt::Horizontal, tr("type"));
  drivertable->setHeaderData(3, Qt::Horizontal, tr("settings"));
 

  actuatortable->setTable("motor_actuator");
  actuatortable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  actuatortable->setHeaderData(0, Qt::Horizontal, tr("name"));
  actuatortable->setHeaderData(1, Qt::Horizontal, tr("driver"));
  actuatortable->setHeaderData(2, Qt::Horizontal, tr("description"));
  actuatortable->setHeaderData(3, Qt::Horizontal, tr("settings"));
  actuatortable->setHeaderData(4, Qt::Horizontal, tr("position"));
  //actuatorview->setColumnHidden (4, true);

  
  InitConfig();
} 

MotorWindow::~MotorWindow()
{
  QLOG_DEBUG ( ) << "Deleting MotorWindow";
  if (comtable) delete comtable;
  if (drivertable) delete drivertable;
  if (actuatortable) delete actuatortable;
  if (motor) delete motor;
}
void MotorWindow::setDbPath(QString _path) {

  if (comtable) delete comtable;
  if (drivertable) delete drivertable;
  if (actuatortable) delete actuatortable;

  // Re_Init object
  
  motor->setDbPath(_path);
  dbPath = motor->path;
  comtable = new QSqlTableModel(this,QSqlDatabase::database(dbPath));
  comview->setModel(comtable);
  Utils::ConfigureSqlTableView(comview);
  drivertable = new QSqlTableModel(this,QSqlDatabase::database(dbPath));
  driverview->setModel(drivertable);
  Utils::ConfigureSqlTableView(driverview);
  actuatortable = new QSqlTableModel(this,QSqlDatabase::database(dbPath));
  actuatorview->setModel(actuatortable);
  Utils::ConfigureSqlTableView(actuatorview);
  
  comtable->setTable("motor_com");
  comtable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  comtable->setHeaderData(0, Qt::Horizontal, tr("name"));
  comtable->setHeaderData(1, Qt::Horizontal, tr("type"));
  comtable->setHeaderData(2, Qt::Horizontal, tr("address"));
  comtable->setHeaderData(3, Qt::Horizontal, tr("settings"));
  
  drivertable->setTable("motor_driver");
  drivertable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  drivertable->setHeaderData(0, Qt::Horizontal, tr("name"));
  drivertable->setHeaderData(1, Qt::Horizontal, tr("com"));
  drivertable->setHeaderData(2, Qt::Horizontal, tr("type"));
  drivertable->setHeaderData(3, Qt::Horizontal, tr("settings"));
 

  actuatortable->setTable("motor_actuator");
  actuatortable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  actuatortable->setHeaderData(0, Qt::Horizontal, tr("name"));
  actuatortable->setHeaderData(1, Qt::Horizontal, tr("driver"));
  actuatortable->setHeaderData(2, Qt::Horizontal, tr("description"));
  actuatortable->setHeaderData(3, Qt::Horizontal, tr("settings"));
  actuatortable->setHeaderData(4, Qt::Horizontal, tr("position"));
  actuatorview->setColumnHidden (4, true);
  InitConfig();
}
void MotorWindow::closeEvent(QCloseEvent* event)
{
  event->accept();  
  QLOG_DEBUG ( ) << "Closing MotorWindow";
  this->hide();
}

void MotorWindow::InitConfig() {
   // Set where clause
  // QLineEdit shows person lastname that id is given parameter
  //comtable->setFilter(QString("name=*").arg(name));
  comtable->select();
  comrow = comtable->rowCount();
  comtable->insertRow(comrow);
  comview->resizeColumnsToContents();
  comview->resizeRowsToContents();
  // Set where clause
  // QLineEdit shows person lastname that id is given parameter
  //drivertable->setFilter(QString("name=*").arg(name));
  drivertable->select();
  driverrow = drivertable->rowCount();
  drivertable->insertRow(driverrow);
  driverview->resizeColumnsToContents();
  driverview->resizeRowsToContents();
  // Set where clause
  // QLineEdit shows person lastname that id is given parameter
  //actuatortable->setFilter(QString("name=*").arg(name));
  actuatortable->select();
  actuatorrow = actuatortable->rowCount();
  actuatortable->insertRow(actuatorrow);
  actuatorview->resizeColumnsToContents();
  actuatorview->resizeRowsToContents();

}

void 
MotorWindow::update(){
  comtable->submitAll();
  drivertable->submitAll();
  actuatortable->submitAll();
  InitConfig();
}
void 
MotorWindow::load(){
  InitRun();
}
void 
MotorWindow::remove(){
  QModelIndexList selected;
  QItemSelectionModel *selmodel;
  selmodel = comview->selectionModel();
  selected = selmodel->selectedIndexes();
  for (int i = selected.size() - 1 ; i >= 0; i--) 
    comtable->removeRow(selected.at(i).row());
  comtable->submitAll();
  selmodel = driverview->selectionModel();
  selected = selmodel->selectedIndexes();
  for (int i = selected.size() - 1 ; i >= 0; i--) 
    drivertable->removeRow(selected.at(i).row());
  drivertable->submitAll();
  selmodel = actuatorview->selectionModel();
  selected = selmodel->selectedIndexes();
  for (int i = selected.size() - 1 ; i >= 0; i--) 
    actuatortable->removeRow(selected.at(i).row());
  actuatortable->submitAll();
  InitConfig();
}
void 
MotorWindow::InitRun(){
  actuatorList->clear();
  QSqlQuery query(QSqlDatabase::database(dbPath));
  query.exec("select name from motor_actuator");
  while (query.next()) {
    QString name = query.value(0).toString();
    actuatorList->append(name);
  }
  motorWidget->setActuatorList(actuatorList);
}
