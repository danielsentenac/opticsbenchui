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

#define DEBUG_LEVEL QsLogging::InfoLevel

OpticsBenchUIMain::OpticsBenchUIMain( QString _appDirPath, QMainWindow* parent, Qt::WindowFlags fl)
  : QMainWindow( parent, fl )
{
  appDirPath = _appDirPath;
  assistant = new Assistant(appDirPath);
  dacwindow = NULL;
  comedidacwindow = NULL;
  comedicounterwindow = NULL;
  motorwindow = NULL;
  superkwindow = NULL;
  isopencamerawindow.clear();
  isopenanalysiswidget = false; 
  isopenacquisitionwidget = false;
  QDir qdir;
   
#ifdef ADVANTECHDAC
  //
  // Create Dac manager
  //
  dac = new DacAdvantech(qdir.currentPath());
  connect(dac,SIGNAL(showWarning(QString)),this,SLOT(showDacWarning(QString)));
  dacwindow = new DacWindow(this,Qt::Window,dac);
  connect(this,SIGNAL(setDbPath(QString)),dacwindow,SLOT(setDbPath(QString)));
#endif

#ifdef COMEDICOUNTER
  //
  // Create Counter manager
  //
  comedicounter = new ComediCounter(qdir.currentPath());
  connect(comedicounter,SIGNAL(showWarning(QString)),this,SLOT(showComediWarning(QString)));
  comedicounterwindow = new ComediWindow(this,Qt::Window,comedicounter);
  connect(this,SIGNAL(setDbPath(QString)),comedicounterwindow,SLOT(setDbPath(QString)));
#endif

#ifdef COMEDIDAC
  //
  // Create Dac comedi manager
  //
  comedidac = new ComediDac(qdir.currentPath());
  connect(comedidac,SIGNAL(showWarning(QString)),this,SLOT(showComediWarning(QString)));
  comedidacwindow = new ComediWindow(this,Qt::Window,comedidac);
  connect(this,SIGNAL(setDbPath(QString)),comedidacwindow,SLOT(setDbPath(QString)));
#endif

#ifdef RASPIDAC
  //
  // Create Dac raspi manager
  //
  raspidac = new RaspiDac(qdir.currentPath());
  connect(raspidac,SIGNAL(showWarning(QString)),this,SLOT(showRaspiWarning(QString)));
  raspidacwindow = new RaspiWindow(this,Qt::Window,raspidac);
  connect(this,SIGNAL(setDbPath(QString)),raspidacwindow,SLOT(setDbPath(QString)));
#endif
  //
  // Create Motor manager
  //
  motor = new Motor(0,qdir.currentPath());
  connect(motor,SIGNAL(showWarning(QString)),this,SLOT(showMotorWarning(QString)));
  motorwindow = new MotorWindow(this,Qt::Window,motor);
  connect(this,SIGNAL(setDbPath(QString)),motorwindow,SLOT(setDbPath(QString)));
  
  //
  // Create SuperK manager
  //
  superk = new SuperK(0,qdir.currentPath());
  connect(superk,SIGNAL(showWarning(QString)),this,SLOT(showSuperKWarning(QString)));
  superkwindow = new SuperKWindow(this,Qt::Window,superk);
  connect(this,SIGNAL(setDbPath(QString)),superkwindow,SLOT(setDbPath(QString)));

  //
  // Create IEEE1394 Camera manager
  //
#ifdef IEEE1394CAMERA
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
#endif
  //
  // Create GiGEVision Camera manager
  //
#ifdef GIGECAMERA
  cameraGiGEMgr = new CameraGiGE();
  cameraGiGEMgr->findCamera();
  QLOG_INFO() << "Found " <<  cameraGiGEMgr->num << " GIGE camera";
  for (int i = 0 ; i < cameraGiGEMgr->num; i++) {
    Camera *camera = new CameraGiGE();
    camera->setCamera(cameraGiGEMgr->cameralist.at(i),i);
    cameraList.push_back(camera);
    isopencamerawindow.push_back(false);
    camerawindowList.push_back(NULL);
  }
#endif
  //
  // Create Neo Andor Camera manager
  //
#ifdef NEOCAMERA
  cameraNeoMgr = new CameraNeo();
  cameraNeoMgr->findCamera();
  QLOG_INFO() << "Found " <<  cameraNeoMgr->num << " NEO Andor camera";
  for (int i = 0 ; i < cameraNeoMgr->num; i++) {
    Camera *camera = new CameraNeo();
    camera->setCamera(cameraNeoMgr->cameralist.at(i),i);
    cameraList.push_back(camera);
    isopencamerawindow.push_back(false);
    camerawindowList.push_back(NULL);
  }
#endif
  //
  // Create Zyla Andor Camera manager
  //
#ifdef ZYLACAMERA
  cameraZylaMgr = new CameraZyla();
  cameraZylaMgr->findCamera();
  QLOG_INFO() << "Found " <<  cameraZylaMgr->num << " ZYLA Andor camera";
  for (int i = 0 ; i < cameraZylaMgr->num; i++) {
    Camera *camera = new CameraZyla();
    camera->setCamera(cameraZylaMgr->cameralist.at(i),i);
    cameraList.push_back(camera);
    isopencamerawindow.push_back(false);
    camerawindowList.push_back(NULL);
  }
#endif
  //
  // Create RAPTOR Falcon Camera manager
  //
#ifdef RAPTORFALCONCAMERA
  cameraRAPTORFALCONMgr = new CameraRAPTORFALCON();
  cameraRAPTORFALCONMgr->findCamera();
  QLOG_INFO() << "Found " <<  cameraRAPTORFALCONMgr->num << " RAPTOR Falcon camera";
  for (int i = 0 ; i < cameraRAPTORFALCONMgr->num; i++) {
    Camera *camera = new CameraRAPTORFALCON();
    camera->setCamera(cameraRAPTORFALCONMgr->cameralist.at(i),i);
    cameraList.push_back(camera);
    isopencamerawindow.push_back(false);
    camerawindowList.push_back(NULL);
  }
#endif
  //
  // Create RAPTOR Ninox640 Camera manager
  //
#ifdef RAPTORNINOX640CAMERA
  cameraRAPTORNINOX640Mgr = new CameraRAPTORNINOX640();
  cameraRAPTORNINOX640Mgr->findCamera();
  QLOG_INFO() << "Found " <<  cameraRAPTORNINOX640Mgr->num << " RAPTOR Ninox640 camera";
  for (int i = 0 ; i < cameraRAPTORNINOX640Mgr->num; i++) {
    Camera *camera = new CameraRAPTORNINOX640();
    camera->setCamera(cameraRAPTORNINOX640Mgr->cameralist.at(i),i);
    cameraList.push_back(camera);
    isopencamerawindow.push_back(false);
    camerawindowList.push_back(NULL);
  }
#endif
  //
  // Create RASPICAM Camera manager
  //
#ifdef RASPICAMERA
  cameraRaspiMgr = new CameraRaspi();
  cameraRaspiMgr->findCamera();
  QLOG_INFO() << "Found " <<  cameraRaspiMgr->num << " RASPICAM camera";
  for (int i = 0 ; i < cameraRaspiMgr->num; i++) {
    Camera *camera = new CameraRaspi();
    camera->setCamera(cameraRaspiMgr->cameralist.at(i),i);
    cameraList.push_back(camera);
    isopencamerawindow.push_back(false);
    camerawindowList.push_back(NULL);
  }
#endif

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
  
#ifdef ADVANTECHDAC
  acquisitionwidget->setDac(dac);
#endif

#ifdef COMEDICOUNTER
  acquisitionwidget->setComediCounter(comedicounter);
#endif

#ifdef COMEDIDAC
  acquisitionwidget->setComediDac(comedidac);
#endif

#ifdef RASPIDAC
  acquisitionwidget->setRaspiDac(raspidac);
#endif

  acquisitionwidget->setMotor(motor);
  acquisitionwidget->setSuperK(superk);
  acquisitionwidget->setCamera(cameraList);
  acquisitionwidget->setDelegates();
  
  setDockNestingEnabled(false);
  tab = new QTabWidget();
  tab->setTabsClosable(true);
  connect(tab, SIGNAL(tabCloseRequested ( int )), this, SLOT(closeTab(int)));
  setCentralWidget(tab);
  //this->setMinimumHeight(600);
  //this->setMinimumWidth(900);
  
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
  int cameranumber = 0;
 #ifdef IEEE1394CAMERA
  for (int i = 0 ; i < cameraIEEE1394Mgr->num; i++) {
    QString selectedCamera = QString(cameraIEEE1394Mgr->vendorlist.at(i)) + " / " + 
      QString(cameraIEEE1394Mgr->modelist.at(i));
    QAction *action = new QAction(selectedCamera, this);
    menuInstruments->addAction(action);
    connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
    signalMapper->setMapping(action, cameranumber++);
  }
#endif
#ifdef GIGECAMERA
  for (int i = 0 ; i < cameraGiGEMgr->num; i++) {
    QString selectedCamera = QString(cameraGiGEMgr->vendorlist.at(i)) + " / " +
      QString(cameraGiGEMgr->modelist.at(i));
    QAction *action = new QAction(selectedCamera, this);
    menuInstruments->addAction(action);
    connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
    signalMapper->setMapping(action, cameranumber++);
  }
#endif
#ifdef NEOCAMERA
  for (int i = 0 ; i < cameraNeoMgr->num; i++) {
    QString selectedCamera = QString(cameraNeoMgr->vendorlist.at(i)) + " / " +
      QString(cameraNeoMgr->modelist.at(i));
    QAction *action = new QAction(selectedCamera, this);
    menuInstruments->addAction(action);
    connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
    signalMapper->setMapping(action, cameranumber++);
  }
#endif
#ifdef ZYLACAMERA
  for (int i = 0 ; i < cameraZylaMgr->num; i++) {
    QString selectedCamera = QString(cameraZylaMgr->vendorlist.at(i)) + " / " +
      QString(cameraZylaMgr->modelist.at(i));
    QAction *action = new QAction(selectedCamera, this);
    menuInstruments->addAction(action);
    connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
    signalMapper->setMapping(action, cameranumber++);
  }
#endif
#ifdef RAPTORFALCONCAMERA
  for (int i = 0 ; i < cameraRAPTORFALCONMgr->num; i++) {
    QString selectedCamera = QString(cameraRAPTORFALCONMgr->vendorlist.at(i)) + " / " +
      QString(cameraRAPTORFALCONMgr->modelist.at(i));
    QAction *action = new QAction(selectedCamera, this);
    menuInstruments->addAction(action);
    connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
    signalMapper->setMapping(action, cameranumber++);
  }
#endif
#ifdef RAPTORNINOX640CAMERA
  for (int i = 0 ; i < cameraRAPTORNINOX640Mgr->num; i++) {
    QString selectedCamera = QString(cameraRAPTORNINOX640Mgr->vendorlist.at(i)) + " / " +
      QString(cameraRAPTORNINOX640Mgr->modelist.at(i));
    QAction *action = new QAction(selectedCamera, this);
    menuInstruments->addAction(action);
    connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
    signalMapper->setMapping(action, cameranumber++);
  }
#endif
#ifdef RASPICAMERA
  for (int i = 0 ; i < cameraRaspiMgr->num; i++) {
    QString selectedCamera = QString(cameraRaspiMgr->vendorlist.at(i)) + " / " +
      QString(cameraRaspiMgr->modelist.at(i));
    QAction *action = new QAction(selectedCamera, this);
    menuInstruments->addAction(action);
    connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
    signalMapper->setMapping(action, cameranumber++);
  }
#endif



  connect(signalMapper, SIGNAL(mapped(int)),this, SLOT(openCameraWindow(int)));

  menuInstruments->addAction("Motor", this, SLOT(openMotorWindow()) );

#ifdef SUPERK
  menuInstruments->addAction("SuperK", this, SLOT(openSuperKWindow()) );
#endif

#ifdef ADVANTECHDAC
  menuInstruments->addAction("Dac", this, SLOT(openDacWindow()) );
#endif

#ifdef COMEDICOUNTER
  menuInstruments->addAction("Counter", this, SLOT(openComediCounterWindow()) );
#endif

#ifdef COMEDIDAC
  menuInstruments->addAction("ComediDac", this, SLOT(openComediDacWindow()) );
#endif

#ifdef RASPIDAC
  menuInstruments->addAction("RaspiDac", this, SLOT(openRaspiDacWindow()) );
#endif

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
      Camera *camera =  cameraList.at(cameraNumber);
      CameraWindow *camerawindow = new CameraWindow(this,Qt::Window, 
						  camera,cameraNumber);
    camerawindow->show();
    camerawindow->update();
    camerawindowList.replace(cameraNumber,camerawindow);
  }
} 
void OpticsBenchUIMain::openMotorWindow() {
  if (motorwindow->isHidden()) 
    motorwindow->show();
}
void OpticsBenchUIMain::openSuperKWindow() {
  if (superkwindow->isHidden())
    superkwindow->show();
}
void OpticsBenchUIMain::openDacWindow() {
  if (dacwindow->isHidden())
    dacwindow->show();
}
void OpticsBenchUIMain::openComediCounterWindow() {
  if (comedicounterwindow->isHidden())
    comedicounterwindow->show();
}
void OpticsBenchUIMain::openComediDacWindow() {
  if (comedidacwindow->isHidden())
    comedidacwindow->show();
}
void OpticsBenchUIMain::openRaspiDacWindow() {
  if (raspidacwindow->isHidden())
    raspidacwindow->show();
}

OpticsBenchUIMain::~OpticsBenchUIMain()
{
  delete acquisitionwidget;
  delete assistant;
  if (dacwindow) delete dacwindow;
  if (comedidacwindow) delete comedidacwindow;
  if (comedicounterwindow) delete comedicounterwindow;
  if (motorwindow) delete motorwindow;
  if (superkwindow) delete superkwindow;
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
void myMessageOutput(QtMsgType type, const QMessageLogContext &, const QString & str) {
   const char * msg = str.toStdString().c_str();
}

void 
OpticsBenchUIMain::showDacWarning(QString message) {
  QMessageBox::warning(this, "Dac Error:", message);
}

void
OpticsBenchUIMain::showComediWarning(QString message) {
  QMessageBox::warning(this, "Comedi Error:", message);
}

void
OpticsBenchUIMain::showRaspiWarning(QString message) {
  QMessageBox::warning(this, "Raspi Error:", message);
}

void 
OpticsBenchUIMain::showMotorWarning(QString message) {
  QMessageBox::warning(this, "Motor Error:", message);
}
void
OpticsBenchUIMain::showSuperKWarning(QString message) {
  QMessageBox::warning(this, "SuperK Error:", message);
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
#ifdef GIGECAMERA
  g_type_init ();
#endif
  qInstallMessageHandler(myMessageOutput);

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
  logger.setLoggingLevel(DEBUG_LEVEL);

  QApplication app(argc, argv); 
  app.addLibraryPath("/usr/local/lib"); 
  QLOG_INFO() << "Built with Qt" << QT_VERSION_STR << "running on" << qVersion();
  QLOG_INFO() << "Qt User Data location : " 
              << QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation); 
  QLOG_INFO() << "OpticsBenchUI version " << OPTICSBENCHUIVERSION
              << " started : " <<  app.applicationDirPath();
  OpticsBenchUIMain* OpticsBenchUI = new OpticsBenchUIMain(app.applicationDirPath(),NULL,NULL);
  OpticsBenchUI->setWindowTitle("OpticsBenchUI");
  OpticsBenchUI->show();
  foreach (const QString &path, app.libraryPaths())
  QLOG_DEBUG() << path;
  QDesktopWidget *desktop = QApplication::desktop();
  QLOG_INFO() << " Screen number " << desktop->screenCount();
  QLOG_INFO() << " Virtual Desktop " << desktop->isVirtualDesktop();
  QLOG_INFO() << " X,Y " << desktop->screenGeometry(0).width() << " " << 
                 desktop->screenGeometry(0).height();
  return app.exec();
}
