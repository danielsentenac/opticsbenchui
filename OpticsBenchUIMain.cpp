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

#include "OpticsBenchUIMain.h"
#include <QDesktopServices>

OpticsBenchUIMain::OpticsBenchUIMain( QString _appDirPath, QMainWindow* parent, Qt::WFlags fl)
  : QMainWindow( parent, fl )
{
  appDirPath = _appDirPath;
  assistant = new Assistant(appDirPath);

  isopencamerawindow.clear();
  isopenanalysiswidget = false; 
  isopenacquisitionwidget = false;
  
  //
  // Create Dac manager
  //
  QDir qdir;
  dac = new Dac(0,qdir.currentPath());
  connect(dac,SIGNAL(showWarning(QString)),this,SLOT(showDacWarning(QString)));
  dacwindow = new DacWindow(this,Qt::Window,dac);
  connect(this,SIGNAL(setDbPath(QString)),dacwindow,SLOT(setDbPath(QString)));
  //
  // Create Motor manager
  //
  motor = new Motor(0,qdir.currentPath());
  connect(motor,SIGNAL(showWarning(QString)),this,SLOT(showMotorWarning(QString)));
  motorwindow = new MotorWindow(this,Qt::Window,motor);
  connect(this,SIGNAL(setDbPath(QString)),motorwindow,SLOT(setDbPath(QString)));
  
  //
  // Create IEEE1394 Camera manager
  //
  cameraIEEE1394Mgr = new CameraIEEE1394();
  cameraIEEE1394Mgr->findCamera();
  QLOG_INFO() << "Found " <<  cameraIEEE1394Mgr->num << " IEEE1394 camera";
  
  for (int i = 0 ; i < cameraIEEE1394Mgr->num; i++) {
    Camera *camera = new CameraIEEE1394();
    camera->setCamera(cameraIEEE1394Mgr->cameralist.at(i),i);
    cameraList.push_back(camera);
    isopencamerawindow.push_back(false);
    camerawindowList.push_back(NULL);
  
  }
  analysiswidget = new AnalysisWidget();
  analysiswidget->setObjectName("Analysis");
  acquisitionwidget = new AcquisitionWidget(qdir.currentPath());
  acquisitionwidget->setObjectName("Acquisition");
  connect(this,SIGNAL(setDbPath(QString)),acquisitionwidget,SLOT(setDbPath(QString)));
  connect(this,SIGNAL(setAcqFile(QString)),acquisitionwidget,SLOT(setAcqFile(QString)));
  connect(this,SIGNAL(isopenCameraWindow(QVector<bool>)),
	  acquisitionwidget,SLOT(isopenCameraWindow(QVector<bool>)));
  connect(acquisitionwidget,SIGNAL(showWarning(QString)),this,
	  SLOT(showAcquisitionWarning(QString)));
  
  acquisitionwidget->setDac(dac);
  acquisitionwidget->setMotor(motor);
  acquisitionwidget->setCamera(cameraList);
  
  setDockNestingEnabled(false);
  tab = new QTabWidget();
  tab->setTabsClosable(true);
  connect(tab, SIGNAL(tabCloseRequested ( int )), this, SLOT(closeTab(int)));
  setCentralWidget(tab);
  this->setMinimumHeight(600);
  this->setMinimumWidth(900);
  
  // Creation of menuBar
  QMenuBar* menuBar = new QMenuBar(this);

  // Creating a menu "File"
  QMenu* menuFile = new QMenu("File");
  // Creating a menu "Instruments"
  QMenu* menuInstruments = new QMenu("Instruments");
  // Creating a menu "Operations"
  QMenu* menuOperations = new QMenu("Operations");
  // Creating a menu "Help"
  QMenu* menuHelp = new QMenu("Help");

  // Create a signal mapper for cameras
  signalMapper = new QSignalMapper(this);

  // And adding the menu "File" in the menu bar
  menuBar->addMenu(menuFile);
  // And adding the menu "Instruments" in the menu bar
  menuBar->addMenu(menuInstruments);
  // And adding the menu "Operations" in the menu bar
  menuBar->addMenu(menuOperations);
   // And adding the menu "Help" in the menu bar
  menuBar->addMenu(menuHelp);
  // Add a separator
  menuFile->addSeparator();
  // Add a separator
  menuInstruments->addSeparator();
  // Add a separator
  menuOperations->addSeparator();

 
  menuFile->addAction("Open Configuration", this, SLOT(openConfiguration()) );
  menuFile->addAction("Save Acquisition", this, SLOT(saveAcqFile()) );
  menuFile->addAction("Exit", this, SLOT(close()) );
 
  menuHelp->addAction("Documentation",this,SLOT(showDocumentation()));

  menuOperations->addAction("Acquisition", this, SLOT(openacquisition()));
  menuOperations->addAction("Analysis", this, SLOT(openanalysis()) );
 
  for (int i = 0 ; i < cameraIEEE1394Mgr->num; i++) {
    QString selectedCamera = QString(cameraIEEE1394Mgr->vendorlist.at(i)) + " / " + 
      QString(cameraIEEE1394Mgr->modelist.at(i));
    QAction *action = new QAction(selectedCamera, this);
    menuInstruments->addAction(action);
    connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
    signalMapper->setMapping(action, i);
  }
  connect(signalMapper, SIGNAL(mapped(int)),this, SLOT(openCameraWindow(int)));
  menuInstruments->addAction("Motor", this, SLOT(openMotorWindow()) );
  menuInstruments->addAction("Dac", this, SLOT(openDacWindow()) );
  // Then set the menu bar to the main window
  setMenuBar(menuBar);
}
void OpticsBenchUIMain::closeTab(int index)
{
  QWidget *currentwidget = tab->widget(index);
  if (currentwidget->objectName() == "Acquisition")
    isopenacquisitionwidget = false;
  else if (currentwidget->objectName() == "Analysis")
    isopenanalysiswidget = false;
  tab->removeTab(index);
}

void OpticsBenchUIMain::openConfiguration() {
  QString path = "";
  path = QFileDialog::getExistingDirectory(this, tr("Open Configuration Location"),
						   QDir::homePath ());
  if (path != "")
    emit setDbPath(path);
}
void OpticsBenchUIMain::saveAcqFile() {
  QString acqfile = "";
  acqfile = QFileDialog::getSaveFileName(this, tr("Save Acquisition File in HDF5 format"),
					  QDir::currentPath ()+ 
					  QDir::separator() + "Scan",
					  tr("HDF5 (*.hf5)"));
  if (acqfile != "")
    emit setAcqFile(acqfile);
}
void OpticsBenchUIMain::showDocumentation()
{
  assistant->showDocumentation("index.html");
}
void OpticsBenchUIMain::openacquisition() {
  if (isopenacquisitionwidget == false) {    
    tab->setCurrentIndex(tab->addTab(acquisitionwidget,"Acquisition"));
    isopenacquisitionwidget = true;
  }
  else
    tab->setCurrentIndex(tab->indexOf(acquisitionwidget));
}
void OpticsBenchUIMain::openanalysis() {
  if (isopenanalysiswidget == false) {
    tab->setCurrentIndex(tab->addTab(analysiswidget,"Analysis"));
    isopenanalysiswidget = true;
  }
  else
    tab->setCurrentIndex(tab->indexOf(analysiswidget));
}
void OpticsBenchUIMain::openCameraWindow(int cameraNumber) {
  if (isopencamerawindow.at(cameraNumber) == false) {
    CameraWindow *camerawindow = new CameraWindow(this,Qt::Window, 
						  cameraList.at(cameraNumber),cameraNumber);
    camerawindow->show();
    camerawindow->update();
    camerawindowList.replace(cameraNumber,camerawindow);
  }
} 
void OpticsBenchUIMain::openMotorWindow() {
  if (motorwindow->isHidden()) 
    motorwindow->show();
}
void OpticsBenchUIMain::openDacWindow() {
  if (dacwindow->isHidden())
    dacwindow->show();
}
OpticsBenchUIMain::~OpticsBenchUIMain()
{
  delete acquisitionwidget;
  delete assistant;
  delete dacwindow;
  delete motorwindow;
  for (int i = 0 ; i < cameraList.size() ; i++) {
    if (isopencamerawindow.at(i) == true) {
     QLOG_INFO() << " Exiting Camera Window " << i;
     delete camerawindowList.at(i);
    }
  }
  for (int i = 0 ; i < cameraList.size() ; i++) {
    QLOG_INFO() << " Exiting Camera " << i;
     delete cameraList.at(i);
   }
  QLOG_INFO() << "OpticsBenchUI ended";
}
void OpticsBenchUIMain::setOpenCameraWindow(bool isopen, int cameranumber){
  isopencamerawindow.replace(cameranumber,isopen);
   emit isopenCameraWindow(isopencamerawindow);
}
void OpticsBenchUIMain::keyPressEvent(QKeyEvent *e) {
  if(e->type() == QKeyEvent::KeyPress) {
    if(e->matches(QKeySequence::Copy)) {
      delete this;
    } 
  }
}
  
void OpticsBenchUIMain::closeEvent(QCloseEvent* event)
{
  event->accept();
  delete this;
}
void myMessageOutput(QtMsgType type, const char *msg) {
  switch (type) {
  case QtDebugMsg:
    fprintf(stderr, "Debug: %s\n", msg);
    break;
  case QtWarningMsg:
    fprintf(stderr, "Warning: %s\n", msg);
    break;
  case QtCriticalMsg:
    fprintf(stderr, "Critical: %s\n", msg);
    break;
  case QtFatalMsg:
    fprintf(stderr, "Fatal: %s\n", msg);
    abort();
  }
}
void 
OpticsBenchUIMain::showDacWarning(QString message) {
  QMessageBox::warning(this, "Dac Error:", message);
}
void 
OpticsBenchUIMain::showMotorWarning(QString message) {
  QMessageBox::warning(this, "Motor Error:", message);
}
void 
OpticsBenchUIMain::showCameraWarning(QString message) {
  QMessageBox::warning(this, "Camera Error:", message);
}
void 
OpticsBenchUIMain::showAcquisitionWarning(QString message) {
  QMessageBox::warning(this, "Acquisition Error:", message);
}
void 
OpticsBenchUIMain::showCameraControlWidgetWarning(QString message) {
  QMessageBox::warning(this, "CameraControlWidget Error:", message);
}
int main(int argc, char *argv[])
{
  qInstallMsgHandler(myMessageOutput);
  QApplication app(argc, argv);
 
  // init the logging mechanism
  QsLogging::Logger& logger = QsLogging::Logger::instance();
  QDir qdir;
  const QString sLogPath(qdir.currentPath() +  QDir::separator() +
                        "OpticsBenchUI_" + 
                         QDateTime::currentDateTime().toString("MMMdd,yy-hh:mm:ss") +
                         ".log");
  QsLogging::DestinationPtr fileDestination(QsLogging::DestinationFactory::MakeFileDestination(sLogPath) );
  QsLogging::DestinationPtr debugDestination(QsLogging::DestinationFactory::MakeDebugOutputDestination() );
  logger.addDestination(debugDestination.get());
  logger.addDestination(fileDestination.get());
  logger.setLoggingLevel(QsLogging::InfoLevel);
  
  QLOG_INFO() << "OpticsBenchUI started : " + app.applicationDirPath();
  QLOG_INFO() << "Built with Qt" << QT_VERSION_STR << "running on" << qVersion();
  QLOG_INFO() << " Qt User Data location : " 
              << QDesktopServices::storageLocation(QDesktopServices::DataLocation); 
  OpticsBenchUIMain* OpticsBenchUI = new OpticsBenchUIMain(app.applicationDirPath(),NULL,NULL);
  OpticsBenchUI->setWindowTitle("Tests & Measurements");
  OpticsBenchUI->show();
  return app.exec();
}
