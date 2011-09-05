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

#include "AcquisitionWidget.h"


AcquisitionWidget::AcquisitionWidget(QString _appDirPath)
{
  path = _appDirPath;
  // Connect database
  dbConnexion();
 
  gridlayout = new QGridLayout();
  this->setLayout(gridlayout);

  // 'acquisition_settings' table model
  acquisitiontitle = new QLabel("Acquisition sequence");
  gridlayout->addWidget(acquisitiontitle,0,0,1,1);
  acquisitiontable = new QSqlTableModel(this,QSqlDatabase::database(path));
  acquisitionview = new QTableView;
  acquisitionview->setModel(acquisitiontable);
  acquisitionview->verticalHeader()->hide();

  gridlayout->addWidget(acquisitionview,1,0,1,5);
  
  updateButton = new QPushButton("Update",this);
  updateButton->setFixedHeight(30);
  updateButton->setFixedWidth(100);
  QObject::connect(updateButton, SIGNAL(clicked()), this, SLOT(update()));
  gridlayout->addWidget(updateButton,2,0,1,1);
  
  removeButton = new QPushButton("Delete",this);
  removeButton->setFixedHeight(30);
  removeButton->setFixedWidth(100);
  QObject::connect(removeButton, SIGNAL(clicked()), this, SLOT(remove()));
  gridlayout->addWidget(removeButton,2,1,1,1);
  
  fileLabel = new QLabel("Save in File Name :");
  gridlayout->addWidget(fileLabel,3,2,1,1);
  filename  = new QLineEdit("Scan");
  filename->setFixedWidth(300);
  gridlayout->addWidget(filename,4,2,1,1);
  filenumberLabel = new QLabel("File Number :");
  gridlayout->addWidget(filenumberLabel,4,3,1,1);
  filenumber = new QLineEdit("0");
  filenumber->setFixedWidth(30);
  gridlayout->addWidget(filenumber,4,4,1,1);


  runButton = new QPushButton("Run",this);
  runButton->setFixedHeight(30);
  runButton->setFixedWidth(100);
  QObject::connect(runButton, SIGNAL(clicked()), this, SLOT(run()));
  gridlayout->addWidget(runButton,5,0,1,1);

  stopButton = new QPushButton("Stop",this);
  stopButton->setFixedHeight(30);
  stopButton->setFixedWidth(100);
  QObject::connect(stopButton, SIGNAL(clicked()), this, SLOT(stop()));
  gridlayout->addWidget(stopButton,5,1,1,1);
  
  acquisitiontable->setTable("acquisition_sequence");
  acquisitiontable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  acquisitiontable->setHeaderData(0, Qt::Horizontal, tr("record"));
  acquisitiontable->setHeaderData(1, Qt::Horizontal, tr("type"));
  acquisitiontable->setHeaderData(2, Qt::Horizontal, tr("instrument"));
  acquisitiontable->setHeaderData(3, Qt::Horizontal, tr("settings"));
  acquisitiontable->setHeaderData(4, Qt::Horizontal, tr("scanplan"));
  acquisitiontable->setHeaderData(5, Qt::Horizontal, tr("status"));
  acquisitiontable->setHeaderData(6, Qt::Horizontal, tr("acquiring"));
  
  InitConfig();

  acquisition = new AcquisitionThread();
  connect(acquisition,SIGNAL(getPosition(QString)),this,SLOT(getPosition(QString)));
  connect(acquisition,SIGNAL(getAcquiring(int)),this,SLOT(getAcquiring(int)));
  connect(acquisition,SIGNAL(getFilenumber(int)),this,SLOT(getFilenumber(int)));
  connect(acquisition,SIGNAL(getDacStatus(bool)),this,SLOT(getDacStatus(bool)));
  connect(acquisition,SIGNAL(getMotorStatus(bool)),this,SLOT(getMotorStatus(bool)));
  connect(acquisition,SIGNAL(getCameraStatus(bool)),this,SLOT(getCameraStatus(bool)));
  connect(acquisition,SIGNAL(getTreatmentStatus(bool)),this,SLOT(getTreatmentStatus(bool)));
  connect(acquisition,SIGNAL(showWarning(QString)),this,SLOT(showAcquisitionWarning(QString)));

}
AcquisitionWidget::~AcquisitionWidget()
{
  if (acquisition) delete acquisition;
  {
  QSqlDatabase db = QSqlDatabase::database(path);  
  db.close();  
  if (acquisitiontable) delete acquisitiontable; 
  }
  QSqlDatabase::removeDatabase(path);
  QLOG_DEBUG ( ) << "Deleting AcquisitionWidget";
}
void AcquisitionWidget::setDbPath(QString _path) {
  
  // Close Db
  {
    QSqlDatabase db = QSqlDatabase::database(path);
    db.close();
    if (acquisitiontable) delete acquisitiontable;
  }
  QSqlDatabase::removeDatabase(path); 
  
  //if (acquisition) delete acquisition;
  
  // Re_Init object
  path = _path;
  // Connect database
  dbConnexion();
  acquisitiontable = new QSqlTableModel(this,QSqlDatabase::database(path));
  acquisitionview->setModel(acquisitiontable);
  acquisitionview->verticalHeader()->hide();
  acquisitiontable->setTable("acquisition_sequence");
  acquisitiontable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  acquisitiontable->setHeaderData(0, Qt::Horizontal, tr("record"));
  acquisitiontable->setHeaderData(1, Qt::Horizontal, tr("type"));
  acquisitiontable->setHeaderData(2, Qt::Horizontal, tr("instrument"));
  acquisitiontable->setHeaderData(3, Qt::Horizontal, tr("settings"));
  acquisitiontable->setHeaderData(4, Qt::Horizontal, tr("scanplan"));
  acquisitiontable->setHeaderData(5, Qt::Horizontal, tr("status"));
  acquisitiontable->setHeaderData(6, Qt::Horizontal, tr("acquiring"));
  InitConfig();

}
void 
AcquisitionWidget::setCamera(QVector<Camera*> _cameraList){
  cameraList = _cameraList;
}
void 
AcquisitionWidget::setMotor(Motor* _motor){
  motor = _motor;
}
void 
AcquisitionWidget::setDac(Dac* _dac){
  dac = _dac;
}

void AcquisitionWidget::InitConfig() {
   // Set where clause
  // QLineEdit shows person lastname that id is given parameter
  //acquisitiontable->setFilter(QString("name=*").arg(name));
  acquisitiontable->setSort(0,Qt::AscendingOrder);
  acquisitiontable->select();
  acquisitionrow = acquisitiontable->rowCount();
  acquisitiontable->insertRow(acquisitionrow);
  acquisitionview->resizeColumnsToContents();
  acquisitionview->resizeRowsToContents();

}

void 
AcquisitionWidget::update(){
  acquisitiontable->submitAll();
  InitConfig();
}

void 
AcquisitionWidget::remove(){
  QModelIndexList selected;
  QItemSelectionModel *selmodel;
  selmodel = acquisitionview->selectionModel();
  selected = selmodel->selectedIndexes();
  for (int i = selected.size() - 1 ; i >= 0; i--) 
    acquisitiontable->removeRow(selected.at(i).row());
  acquisitiontable->submitAll();
  InitConfig();
}
void 
AcquisitionWidget::run(){
  this->stop();
  QSqlQuery query(QSqlDatabase::database(path));
  query.exec("update acquisition_sequence set status = ''");
  query.exec("update acquisition_sequence set acquiring = ''");
  query.exec("select record,type,instrument,settings,scanplan from acquisition_sequence "
	     "order by record");
  
  // Store acquisition sequence
  sequenceList.clear();
  cur_record = 0;
  while (query.next()) {
    int seq_record = query.value(0).toInt();
    if ( seq_record < 0 ) continue;
    AcquisitionSequence *sequence = new AcquisitionSequence();
    sequence->record = cur_record++;
    sequence->seq_record = seq_record;
    sequence->instrumentType = query.value(1).toString();
    sequence->instrumentName = query.value(2).toString();
    sequence->settings = query.value(3).toString();
    sequence->scanplan = query.value(4).toString();
    sequenceList.append(sequence);
  }
  
  // Prepare sequence
  for (int i = 0 ; i < sequenceList.size(); i++) {
    AcquisitionSequence *sequence = sequenceList.at(i);
    sequence->prepare();
    }
  // Set File name and number from widget
  acquisition->setFile(filename->text(),filenumber->text().toInt());
  acquisition->setCamera(cameraList);
  acquisition->setMotor(motor);
  acquisition->setDac(dac);
  acquisition->setSequenceList(sequenceList);
  acquisition->start();
  QLOG_DEBUG ( ) << "AcquisitionWidget:: run started";
}
void 
AcquisitionWidget::stop(){
  acquisition->stop();
}
void
AcquisitionWidget::getPosition(QString positionQString) {
  
  QSqlQuery query(QSqlDatabase::database(path));
  QString statusQString = "POS:" + positionQString;
  query.prepare("update acquisition_sequence set status = ? where record = ?");
  query.addBindValue(statusQString);
  query.addBindValue(cur_record);
  query.exec();
  QLOG_DEBUG ( ) << "getPosition::Db " << query.lastError().text();
  InitConfig();
}
void AcquisitionWidget::getCameraStatus(bool imagesuccess) {
  QSqlQuery query(QSqlDatabase::database(path));
  QString statusQString;
  if (imagesuccess == true)
      statusQString = "OK";
  else
    statusQString = "FAIL";
  query.prepare("update acquisition_sequence set status = ? where record = ?");
  query.addBindValue(statusQString);
  query.addBindValue(cur_record);
  query.exec();
  InitConfig();
}
void AcquisitionWidget::getDacStatus(bool dacsuccess){
  QSqlQuery query(QSqlDatabase::database(path));

  QString statusQString;
  if (dacsuccess == true)
    statusQString = "OK";
  else
    statusQString = "FAIL";
  query.prepare("update acquisition_sequence set status = ? where record = ?");
  query.addBindValue(statusQString);
  query.addBindValue(cur_record);
  query.exec();
  InitConfig();
}
void AcquisitionWidget::getMotorStatus(bool motorsuccess){
  QSqlQuery query(QSqlDatabase::database(path));

  QString statusQString;
  if (motorsuccess == true)
    statusQString = "OK";
  else
    statusQString = "FAIL";
  query.prepare("update acquisition_sequence set status = ? where record = ?");
  query.addBindValue(statusQString);
  query.addBindValue(cur_record);
  query.exec();
  InitConfig();
}

void AcquisitionWidget::getTreatmentStatus(bool treatmentsuccess){
  QSqlQuery query(QSqlDatabase::database(path));
 
  QString statusQString;
  if (treatmentsuccess == true)
    statusQString = "OK";
  else
    statusQString = "FAIL";
  query.prepare("update acquisition_sequence set status = ? where record = ?");
  query.addBindValue(statusQString);
  query.addBindValue(cur_record);
  query.exec();
  InitConfig();
}
void AcquisitionWidget::getAcquiring(int record) {
  QSqlQuery query(QSqlDatabase::database(path));
 
  query.exec("update acquisition_sequence set acquiring = ''");
  query.exec("select record from acquisition_sequence order by record");
  QLOG_DEBUG( ) << query.lastError().text();      
  int seq_record = 0,count = 0;
  while (query.next()) {
    seq_record = query.value(0).toInt();
    if ( seq_record < 0 ) continue;
    if ( count == record ) 
      break;
    count++;
  }
  QLOG_DEBUG ( ) << seq_record;    
  query.prepare("update acquisition_sequence set acquiring = '*' where record = ?");
  query.addBindValue(seq_record);
  query.exec();
  InitConfig();
  cur_record = seq_record;   
}
void AcquisitionWidget::getFilenumber(int number) {
  filenumber->setText(QString::number(number));
}
void 
AcquisitionWidget::isopenCameraWindow(QVector<bool> isopencamerawindow) {
  acquisition->isopencamerawindow = isopencamerawindow;
}
// function : create connexion to the database
void 
AcquisitionWidget::dbConnexion() {

  path.append(QDir::separator()).append("acquisition.db3");
  path = QDir::toNativeSeparators(path);
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",path);
  db.setDatabaseName(path);  
  if ( !db.open() ) {
    QLOG_WARN ( ) << db.lastError().text();
    emit showWarning(db.lastError().text());
  }
  // Create acquisition tables
  QSqlQuery query(QSqlDatabase::database(path));
  query.exec("create table acquisition_sequence "
	     "(record int primary key, "
	     "type varchar(128), "
	     "instrument varchar(128), "
	     "settings varchar(255), "
	     "scanplan varchar(128), "
	     "status varchar(128), "
	     "acquiring varchar(64))");
  query.exec("update acquisition_sequence set status = ''");
  query.exec("update acquisition_sequence set acquiring = ''");

  QLOG_DEBUG ( ) << query.lastError().text();      
}
void
AcquisitionWidget::showAcquisitionWarning(QString message) {
  emit showWarning(message);
}

