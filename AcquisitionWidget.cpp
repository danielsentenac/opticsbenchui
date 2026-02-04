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
#include "Utils.h"

namespace {
const char kSelectionStylesheet[] =
    "QTreeView::item:selected{background-color: palette(highlight); "
    "color: palette(highlightedText);};";
}  // namespace

AcquisitionWidget::AcquisitionWidget(QString appDirPath)
    : appDirPath(appDirPath),
      path(),
      acqfile("Scan"),
      filenumber("0"),
      acquisitionrow(0),
      cur_record(0),
      cameraList(),
      motor(nullptr),
      superk(nullptr),
      dac(nullptr),
      comedicounter(nullptr),
      comedidac(nullptr),
      raspidac(nullptr),
      splashLabel(new QLabel()),
      acquisitiontitle(new QLabel("Acquisition sequence")),
      acquisitiontable(nullptr),
      acquisitionview(new QTableView()),
      reloadButton(new QPushButton("Reload", this)),
      updateButton(new QPushButton("Update", this)),
      removeButton(new QPushButton("Delete", this)),
      runButton(new QPushButton("Run", this)),
      stopButton(new QPushButton("Stop", this)),
      gridlayout(new QGridLayout()),
      acquisition(new AcquisitionThread()),
      sequenceList() {
  QLOG_DEBUG() << "AcquisitionWidget::AcquisitionWidget";

  dbConnexion();

  setLayout(gridlayout);

  gridlayout->addWidget(acquisitiontitle, 0, 0, 1, 10);
  acquisitiontable = new QSqlTableModel(this, QSqlDatabase::database(path));
  acquisitionview->setStyleSheet(kSelectionStylesheet);
  acquisitionview->setModel(acquisitiontable);
  acquisitionview->verticalHeader()->hide();
  Utils::ConfigureSqlTableView(acquisitionview);
  gridlayout->addWidget(acquisitionview, 1, 0, 1, 10);

  reloadButton->setFixedSize(100, 30);
  connect(reloadButton, SIGNAL(clicked()), this, SLOT(reload()));
  gridlayout->addWidget(reloadButton, 2, 0, 1, 1);

  updateButton->setFixedSize(100, 30);
  connect(updateButton, SIGNAL(clicked()), this, SLOT(update()));
  gridlayout->addWidget(updateButton, 2, 1, 1, 1);

  removeButton->setFixedSize(100, 30);
  connect(removeButton, SIGNAL(clicked()), this, SLOT(remove()));
  gridlayout->addWidget(removeButton, 2, 2, 1, 1);

  runButton->setFixedSize(100, 30);
  connect(runButton, SIGNAL(clicked()), this, SLOT(run()));
  gridlayout->addWidget(runButton, 5, 0, 1, 1);

  stopButton->setFixedSize(100, 30);
  connect(stopButton, SIGNAL(clicked()), this, SLOT(stop()));
  gridlayout->addWidget(stopButton, 5, 1, 1, 1);

  setupAcquisitionTable();
  InitConfig();

  connect(acquisition, SIGNAL(getPosition(QString)), this,
          SLOT(getPosition(QString)));
  connect(acquisition, SIGNAL(getSuperKData(QString)), this,
          SLOT(getSuperKData(QString)));
  connect(acquisition, SIGNAL(getAcquiring(int)), this, SLOT(getAcquiring(int)));
  connect(acquisition, SIGNAL(getFilenumber(int)), this,
          SLOT(getFilenumber(int)));
  connect(acquisition, SIGNAL(getDacStatus(bool)), this,
          SLOT(getDacStatus(bool)));
  connect(acquisition, SIGNAL(getMotorStatus(bool)), this,
          SLOT(getMotorStatus(bool)));
  connect(acquisition, SIGNAL(getSuperKStatus(bool)), this,
          SLOT(getSuperKStatus(bool)));
  connect(acquisition, SIGNAL(getCameraStatus(bool)), this,
          SLOT(getCameraStatus(bool)));
  connect(acquisition, SIGNAL(getTreatmentStatus(bool)), this,
          SLOT(getTreatmentStatus(bool)));
  connect(acquisition, SIGNAL(showWarning(QString)), this,
          SLOT(showAcquisitionWarning(QString)));
  connect(acquisition, SIGNAL(splashScreen(QString, int, int)), this,
          SLOT(splashScreen(QString, int, int)));
  connect(acquisition, SIGNAL(requestAnalysis()), this,
          SLOT(requestAnalysisFromThread()));
}

AcquisitionWidget::~AcquisitionWidget() {
  delete splashLabel;
  delete acquisition;
  QSqlDatabase db = QSqlDatabase::database(path);
  db.close();
  delete acquisitiontable;
  QSqlDatabase::removeDatabase(path);
  QLOG_DEBUG() << "Deleting AcquisitionWidget";
}

void AcquisitionWidget::setAcqFile(QString acqfile) {
  this->acqfile = acqfile;
  filenumber = "0";
}

void AcquisitionWidget::setDbPath(QString path) {
  {
    QSqlDatabase db = QSqlDatabase::database(this->path);
    db.close();
    delete acquisitiontable;
  }
  QSqlDatabase::removeDatabase(this->path);

  appDirPath = path;
  dbConnexion();

  acquisitiontable = new QSqlTableModel(this, QSqlDatabase::database(this->path));
  acquisitionview->setModel(acquisitiontable);
  acquisitionview->verticalHeader()->hide();
  Utils::ConfigureSqlTableView(acquisitionview);
  setupAcquisitionTable();
  InitConfig();
}

void AcquisitionWidget::setCamera(QVector<Camera*> cameraList) {
  this->cameraList = cameraList;
}

void AcquisitionWidget::setMotor(Motor* motor) {
  this->motor = motor;
}

void AcquisitionWidget::setSuperK(SuperK* superk) {
  this->superk = superk;
}

void AcquisitionWidget::setDac(Dac* dac) {
  this->dac = dac;
}

void AcquisitionWidget::setComediCounter(Comedi* comedi) {
  comedicounter = comedi;
}

void AcquisitionWidget::setComediDac(Comedi* comedi) {
  comedidac = comedi;
}

void AcquisitionWidget::setRaspiDac(Raspi* raspi) {
  raspidac = raspi;
}

void AcquisitionWidget::setDelegates() {
  // Populate types
  QStringList *typeList = new QStringList();
  typeList->append("");
  typeList->append("MOTOR");
  typeList->append("DAC");
  typeList->append("COMEDIDAC");
  typeList->append("COMEDICOUNTER");
  typeList->append("RASPIDAC");
  typeList->append("SUPERK");
  typeList->append("CAMERA");
  typeList->append("SLM");
  typeList->append("FILE");
  typeList->append("TREATMENT");
  ComboBoxDelegate *typeCombo = new ComboBoxDelegate(this, typeList);

  // Populate instrument list from motor database
  QString motordb = Utils::BuildDbPath(appDirPath, "motor.db3");
  QSqlQuery querymotor(QSqlDatabase::database(motordb));
  QStringList *instrumentList = new QStringList();
  instrumentList->append("");
  querymotor.exec("select * from motor_actuator");
  while (querymotor.next()) {
    instrumentList->append(querymotor.value(0).toString());
  }
  
  // Populate instrument list from dac database
  QString dacdb = Utils::BuildDbPath(appDirPath, "dac.db3");
  QSqlQuery querydac(QSqlDatabase::database(dacdb));
  querydac.exec("select name from dac_settings");
  while (querydac.next()) {
    instrumentList->append(querydac.value(0).toString());
  }

  // Populate instrument list from comedi counter database
  QString comedicounterdb = Utils::BuildDbPath(appDirPath, "comedicounter.db3");
  QSqlQuery querycomedicounter(QSqlDatabase::database(comedicounterdb));
  querycomedicounter.exec("select name from comedi_settings");
  while (querycomedicounter.next()) {
    instrumentList->append(querycomedicounter.value(0).toString());
  }

  // Populate instrument list from comedi dac database
  QString comedidacdb = Utils::BuildDbPath(appDirPath, "comedidac.db3");
  QSqlQuery querycomedidac(QSqlDatabase::database(comedidacdb));
  querycomedidac.exec("select name from comedi_settings");
  while (querycomedidac.next()) {
    instrumentList->append(querycomedidac.value(0).toString());
  }

  // Populate instrument list from superk database
  QString superkdb = Utils::BuildDbPath(appDirPath, "superk.db3");
  QSqlQuery querysuperk(QSqlDatabase::database(superkdb));
  querysuperk.exec("select name from superk_driver");
  while (querysuperk.next()) {
    instrumentList->append(querysuperk.value(0).toString());
  }

  // Populate instrument list from cameraList
  for (int i = 0; i < cameraList.size(); i++) {
    Camera* camera = cameraList.at(i);
    instrumentList->append(camera->model);
  }
  ComboBoxDelegate *instrumentCombo = new ComboBoxDelegate(this, instrumentList);
  
  acquisitionview->setItemDelegateForColumn(1, typeCombo);
  acquisitionview->setItemDelegateForColumn(2, instrumentCombo);

}

void AcquisitionWidget::InitConfig() {
  acquisitiontable->setSort(0, Qt::AscendingOrder);
  acquisitiontable->select();
  acquisitionrow = acquisitiontable->rowCount();
  acquisitiontable->insertRow(acquisitionrow);
  acquisitionview->resizeColumnsToContents();
  acquisitionview->resizeRowsToContents();
}

void AcquisitionWidget::setupAcquisitionTable() {
  acquisitiontable->setTable("acquisition_sequence");
  acquisitiontable->setEditStrategy(QSqlTableModel::OnManualSubmit);
  acquisitiontable->setHeaderData(0, Qt::Horizontal, tr("record"));
  acquisitiontable->setHeaderData(1, Qt::Horizontal, tr("type"));
  acquisitiontable->setHeaderData(2, Qt::Horizontal, tr("instrument"));
  acquisitiontable->setHeaderData(3, Qt::Horizontal, tr("settings"));
  acquisitiontable->setHeaderData(4, Qt::Horizontal, tr("scanplan"));
  acquisitiontable->setHeaderData(5, Qt::Horizontal, tr("status"));
  acquisitiontable->setHeaderData(6, Qt::Horizontal, tr("acquiring"));
}

void AcquisitionWidget::update() {
  acquisitiontable->submitAll();
  InitConfig();
}

void AcquisitionWidget::reload() {
  InitConfig();
}

void AcquisitionWidget::remove() {
  QModelIndexList selected;
  QItemSelectionModel *selmodel;
  selmodel = acquisitionview->selectionModel();
  selected = selmodel->selectedIndexes();
  for (int i = selected.size() - 1; i >= 0; i--) {
    acquisitiontable->removeRow(selected.at(i).row());
  }
  acquisitiontable->submitAll();
  InitConfig();
}

void AcquisitionWidget::run() {
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
    if (seq_record < 0) {
      continue;
    }
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
  for (int i = 0; i < sequenceList.size(); i++) {
    AcquisitionSequence *sequence = sequenceList.at(i);
    sequence->prepare();
  }
  if (motor) {
    QSet<QString> motorNames;
    for (int i = 0; i < sequenceList.size(); i++) {
      const AcquisitionSequence *sequence = sequenceList.at(i);
      if (sequence->instrumentType == "MOTOR") {
        motorNames.insert(sequence->instrumentName);
      }
    }
    for (const QString& name : motorNames) {
      if (!name.isEmpty()) {
        motor->connectMotor(name);
      }
    }
  }
  // Set File name and number from widget
  QString filepath = acqfile + "_" + filenumber + ".h5";
  acquisition->setFile(filepath, filenumber.toInt());
  acquisition->setCamera(cameraList);
  acquisition->setMotor(motor);
  acquisition->setSuperK(superk);
  acquisition->setDac(dac);
  acquisition->setComediCounter(comedicounter);
  acquisition->setComediDac(comedidac);
  acquisition->setRaspiDac(raspidac);
  acquisition->setSequenceList(sequenceList);
  acquisition->start();
  QLOG_DEBUG() << "AcquisitionWidget::run started";
}

void AcquisitionWidget::stop() {
  acquisition->stop();
  setAcquiringToLastRecord();
}

void AcquisitionWidget::splashScreen(QString imagepath, int screen_x,
                                     int screen_y) {
  QPixmap pixmap(imagepath);
  splashLabel->setPixmap(pixmap);
  splashLabel->setWindowState(Qt::WindowFullScreen);
  splashLabel->move(screen_x, screen_y);
  splashLabel->show();
  QLOG_INFO() << "AcquisitionWidget::splashScreen> Show splash screen "
              << imagepath << " done";
  acquisition->wakeSplashScreen();
}

void AcquisitionWidget::getPosition(QString positionQString) {
  updateStatusForRecord("POS:" + positionQString, cur_record);
}

void AcquisitionWidget::getSuperKData(QString dataStr) {
  updateStatusForRecord("Data:" + dataStr, cur_record);
}

QString AcquisitionWidget::statusFromSuccess(bool success) const {
  return success ? "OK" : "FAIL";
}

void AcquisitionWidget::updateStatusForRecord(const QString& status,
                                              int record) {
  QSqlQuery query(QSqlDatabase::database(path));
  query.prepare("update acquisition_sequence set status = ? where record = ?");
  query.addBindValue(status);
  query.addBindValue(record);
  query.exec();
  QLOG_DEBUG() << "AcquisitionWidget::statusUpdate"
               << query.lastError().text();
  InitConfig();
}

void AcquisitionWidget::getCameraStatus(bool imagesuccess) {
  updateStatusForRecord(statusFromSuccess(imagesuccess), cur_record);
}

void AcquisitionWidget::getDacStatus(bool dacsuccess) {
  updateStatusForRecord(statusFromSuccess(dacsuccess), cur_record);
}

void AcquisitionWidget::getMotorStatus(bool motorsuccess) {
  updateStatusForRecord(statusFromSuccess(motorsuccess), cur_record);
}

void AcquisitionWidget::getSuperKStatus(bool superksuccess) {
  updateStatusForRecord(statusFromSuccess(superksuccess), cur_record);
}

void AcquisitionWidget::getTreatmentStatus(bool treatmentsuccess) {
  updateStatusForRecord(statusFromSuccess(treatmentsuccess), cur_record);
}

void AcquisitionWidget::getAcquiring(int record) {
  QSqlQuery query(QSqlDatabase::database(path));

  query.exec("update acquisition_sequence set acquiring = ''");
  query.exec("select record from acquisition_sequence order by record");
  QLOG_DEBUG() << query.lastError().text();
  int seq_record = 0;
  int count = 0;
  while (query.next()) {
    seq_record = query.value(0).toInt();
    if (seq_record < 0) {
      continue;
    }
    if (count == record) {
      break;
    }
    count++;
  }
  QLOG_DEBUG() << seq_record;
  query.prepare("update acquisition_sequence set acquiring = '*' where record = ?");
  query.addBindValue(seq_record);
  query.exec();
  InitConfig();
  cur_record = seq_record;
}

void AcquisitionWidget::setAcquiringToLastRecord() {
  QSqlQuery query(QSqlDatabase::database(path));
  query.exec("update acquisition_sequence set acquiring = ''");
  if (!query.exec("select record from acquisition_sequence "
                  "where record >= 0 order by record desc limit 1")) {
    QLOG_WARN() << "AcquisitionWidget::setAcquiringToLastRecord"
                << query.lastError().text();
    InitConfig();
    return;
  }
  if (query.next()) {
    const int lastRecord = query.value(0).toInt();
    QSqlQuery update(QSqlDatabase::database(path));
    update.prepare("update acquisition_sequence set acquiring = '*' where record = ?");
    update.addBindValue(lastRecord);
    update.exec();
  }
  InitConfig();
}
void AcquisitionWidget::getFilenumber(int number) {
  filenumber = QString::number(number);
}

void AcquisitionWidget::isopenCameraWindow(QVector<bool> isopencamerawindow) {
  acquisition->setIsOpenCameraWindow(isopencamerawindow);
}

void AcquisitionWidget::dbConnexion() {
  QLOG_DEBUG() << "AcquisitionWidget::dbConnexion";
  path = Utils::BuildDbPath(appDirPath, "acquisition.db3");
  QSqlDatabase db = Utils::ConnectSqliteDb(path,
                                           "AcquisitionWidget::dbConnexion>");
  if (!db.isOpen()) {
    Utils::EmitWarning(this, __FUNCTION__,
                       db.lastError().text());
  }
  // Create acquisition tables
  QSqlQuery query(QSqlDatabase::database(path));
  Utils::ExecSql(
      query,
      "create table if not exists acquisition_sequence "
      "(record int primary key, "
      "type varchar(128), "
      "instrument varchar(128), "
      "settings varchar(255), "
      "scanplan varchar(128), "
      "status varchar(128), "
      "acquiring varchar(64))",
      "AcquisitionWidget::dbConnexion>");
  Utils::ExecSql(query, "update acquisition_sequence set status = ''",
                 "AcquisitionWidget::dbConnexion>");
  Utils::ExecSql(query, "update acquisition_sequence set acquiring = ''",
                 "AcquisitionWidget::dbConnexion>");
}

void AcquisitionWidget::showAcquisitionWarning(QString message) {
  Utils::EmitWarning(this, __FUNCTION__, message);
}

void AcquisitionWidget::requestAnalysisFromThread() {
  emit requestAnalysis();
}
