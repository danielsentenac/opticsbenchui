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
 * @file OpticsBenchUIMain.cpp
 * @brief Main application window.
 */

#include "OpticsBenchUIMain.h"
#include "Utils.h"
#include <QDesktopServices>
#include <QUrl>
#include <QScreen>
#include <QGuiApplication>
#include <functional>

namespace {
template <typename CameraType>
void setupCameraManager(Camera *&manager,
                        const QString &label,
                        QVector<Camera *> &cameraList,
                        QVector<bool> &isOpenCameraWindow,
                        QVector<CameraWindow *> &cameraWindowList)
{
  manager = new CameraType();
  manager->findCamera();
  QLOG_INFO() << "Found " << manager->num << " " << label << " camera";

  for (int i = 0; i < manager->num; i++) {
    Camera *camera = new CameraType();
    camera->setCamera(manager->cameralist.at(i), i);
    cameraList.push_back(camera);
    isOpenCameraWindow.push_back(false);
    cameraWindowList.push_back(NULL);
  }
}

void addCameraMenuActions(Camera *manager,
                          QMenu *menu,
                          QSignalMapper *signalMapper,
                          int &cameraNumber)
{
  if (!manager) {
    return;
  }
  for (int i = 0; i < manager->num; i++) {
    const QString selectedCamera =
        QString(manager->vendorlist.at(i)) + " / " +
        QString(manager->modelist.at(i));
    QAction *action = new QAction(selectedCamera, menu);
    menu->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
    signalMapper->setMapping(action, cameraNumber++);
  }
}
} // namespace

#define DEBUG_LEVEL QsLogging::InfoLevel

OpticsBenchUIMain::OpticsBenchUIMain( QString _appDirPath, QMainWindow* parent, Qt::WindowFlags fl)
  : QMainWindow( parent, fl )
{
  appDirPath = _appDirPath;
  assistant = new Assistant(appDirPath);
  dacwindow = NULL;
#if defined(COMEDICOUNTER) || defined(COMEDIDAC)
  comedidacwindow = NULL;
  comedicounterwindow = NULL;
#endif
  motorwindow = NULL;
  superkwindow = NULL;
  isopencamerawindow.clear();
  isopenanalysiswidget = false; 
  isopenacquisitionwidget = false;
  QDir qdir;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  if (QScreen *screen = QGuiApplication::primaryScreen()) {
    resize(screen->availableGeometry().size() * 0.7);
  }
#else
  resize(QDesktopWidget().availableGeometry(this).size() * 0.7);
#endif
   
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
  // Create USB Camera manager
  //
#ifdef USBCAMERA
  setupCameraManager<CameraUSB>(cameraUSBMgr, "USB", cameraList, isopencamerawindow, camerawindowList);
#endif
  //
  // Create IEEE1394 Camera manager
  //
#ifdef IEEE1394CAMERA
  setupCameraManager<CameraIEEE1394>(cameraIEEE1394Mgr,
                                     "IEEE1394",
                                     cameraList,
                                     isopencamerawindow,
                                     camerawindowList);
#endif
  //
  // Create GiGEVision Camera manager
  //
#ifdef GIGECAMERA
  setupCameraManager<CameraGiGE>(cameraGiGEMgr,
                                 "GIGE",
                                 cameraList,
                                 isopencamerawindow,
                                 camerawindowList);
#endif
  //
  // Create Neo Andor Camera manager
  //
#ifdef NEOCAMERA
  setupCameraManager<CameraNeo>(cameraNeoMgr,
                                "NEO Andor",
                                cameraList,
                                isopencamerawindow,
                                camerawindowList);
#endif
  //
  // Create Zyla Andor Camera manager
  //
#ifdef ZYLACAMERA
  setupCameraManager<CameraZyla>(cameraZylaMgr,
                                 "ZYLA Andor",
                                 cameraList,
                                 isopencamerawindow,
                                 camerawindowList);
#endif
  //
  // Create RAPTOR Falcon Camera manager
  //
#ifdef RAPTORFALCONCAMERA
  setupCameraManager<CameraRAPTORFALCON>(cameraRAPTORFALCONMgr,
                                        "RAPTOR Falcon",
                                        cameraList,
                                        isopencamerawindow,
                                        camerawindowList);
#endif
  //
  // Create RAPTOR Ninox640 Camera manager
  //
#ifdef RAPTORNINOX640CAMERA
  setupCameraManager<CameraRAPTORNINOX640>(cameraRAPTORNINOX640Mgr,
                                          "RAPTOR Ninox640",
                                          cameraList,
                                          isopencamerawindow,
                                          camerawindowList);
#endif
  //
  // Create RASPICAM Camera manager
  //
#ifdef RASPICAMERA
  setupCameraManager<CameraRaspi>(cameraRaspiMgr,
                                  "RASPICAM",
                                  cameraList,
                                  isopencamerawindow,
                                  camerawindowList);
#endif

  //
  // Create ALLIEDVISION Camera manager
  //
#ifdef ALLIEDVISIONCAMERA
  setupCameraManager<CameraAlliedVision>(cameraAlliedVisionMgr,
                                         "ALLIED VISION",
                                         cameraList,
                                         isopencamerawindow,
                                         camerawindowList);
#endif

  analysiswidget = new AnalysisWidget(qdir.currentPath());
  analysiswidget->setObjectName("Analysis");
  acquisitionwidget = new AcquisitionWidget(qdir.currentPath());
  acquisitionwidget->setObjectName("Acquisition");
  connect(this,SIGNAL(setDbPath(QString)),acquisitionwidget,SLOT(setDbPath(QString)));
  connect(this,SIGNAL(setDbPath(QString)),analysiswidget,SLOT(setDbPath(QString)));
  connect(this,SIGNAL(setAcqFile(QString)),acquisitionwidget,SLOT(setAcqFile(QString)));
  connect(this,SIGNAL(isopenCameraWindow(QVector<bool>)),
	  acquisitionwidget,SLOT(isopenCameraWindow(QVector<bool>)));
  connect(acquisitionwidget,SIGNAL(showWarning(QString)),this,
	  SLOT(showAcquisitionWarning(QString)));
  connect(acquisitionwidget,SIGNAL(requestAnalysis()),analysiswidget,
          SLOT(runFromAcquisition()));
  
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
#ifndef NO_HDF5
  menuFile->addAction("Save Acquisition", this, SLOT(saveAcqFile()) );
#else
  QAction* saveAcqAction = menuFile->addAction("Save Acquisition", this, SLOT(saveAcqFile()) );
  saveAcqAction->setEnabled(false);
#endif
  menuFile->addAction("Exit", this, SLOT(close()) );
 
  menuHelp->addAction("Documentation",this,SLOT(showDocumentation()));
  menuHelp->addAction("API Documentation",this,SLOT(showApiDocumentation()));

  menuOperations->addAction("Acquisition", this, SLOT(openacquisition()));
  menuOperations->addAction("Analysis", this, SLOT(openanalysis()) );
  int cameranumber = 0;
#ifdef USBCAMERA
  addCameraMenuActions(cameraUSBMgr, menuInstruments, signalMapper, cameranumber);
#endif
 #ifdef IEEE1394CAMERA
  addCameraMenuActions(cameraIEEE1394Mgr, menuInstruments, signalMapper, cameranumber);
#endif
#ifdef GIGECAMERA
  addCameraMenuActions(cameraGiGEMgr, menuInstruments, signalMapper, cameranumber);
#endif
#ifdef NEOCAMERA
  addCameraMenuActions(cameraNeoMgr, menuInstruments, signalMapper, cameranumber);
#endif
#ifdef ZYLACAMERA
  addCameraMenuActions(cameraZylaMgr, menuInstruments, signalMapper, cameranumber);
#endif
#ifdef RAPTORFALCONCAMERA
  addCameraMenuActions(cameraRAPTORFALCONMgr, menuInstruments, signalMapper, cameranumber);
#endif
#ifdef RAPTORNINOX640CAMERA
  addCameraMenuActions(cameraRAPTORNINOX640Mgr, menuInstruments, signalMapper, cameranumber);
#endif
#ifdef RASPICAMERA
  addCameraMenuActions(cameraRaspiMgr, menuInstruments, signalMapper, cameranumber);
#endif
#ifdef ALLIEDVISIONCAMERA
  addCameraMenuActions(cameraAlliedVisionMgr, menuInstruments, signalMapper, cameranumber);
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
#ifdef NO_HDF5
  QMessageBox::warning(this, tr("OpticsBenchUI"),
                       tr("HDF5 support is disabled. Acquisition files cannot be saved."));
  return;
#endif
  QString acqfile = "";
  acqfile = QFileDialog::getSaveFileName(
      this, tr("Save Acquisition File in HDF5 format"),
      Utils::DefaultHdf5Path("Scan"),
      Utils::Hdf5FileDialogFilter());
  if (acqfile != "") {
    acqfile = Utils::EnsureHdf5Extension(acqfile);
    emit setAcqFile(acqfile);
  }
}
void OpticsBenchUIMain::showDocumentation()
{
  assistant->showDocumentation("index.html");
}
void OpticsBenchUIMain::showApiDocumentation()
{
  const QString apiIndex = appDirPath + QLatin1String("/docs/api/html/index.html");
  if (!QFile::exists(apiIndex)) {
    QMessageBox::warning(this, tr("OpticsBenchUI"),
                         tr("API documentation not found at:\n%1").arg(apiIndex));
    return;
  }
  QDesktopServices::openUrl(QUrl::fromLocalFile(apiIndex));
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
#if defined(COMEDICOUNTER) || defined(COMEDIDAC)
void OpticsBenchUIMain::openComediCounterWindow() {
  if (comedicounterwindow->isHidden())
    comedicounterwindow->show();
}
void OpticsBenchUIMain::openComediDacWindow() {
  if (comedidacwindow->isHidden())
    comedidacwindow->show();
}
#endif
void OpticsBenchUIMain::openRaspiDacWindow() {
  if (raspidacwindow->isHidden())
    raspidacwindow->show();
}

OpticsBenchUIMain::~OpticsBenchUIMain()
{
  delete acquisitionwidget;
  delete assistant;
  if (dacwindow) delete dacwindow;
#if defined(COMEDICOUNTER) || defined(COMEDIDAC)
  if (comedidacwindow) delete comedidacwindow;
  if (comedicounterwindow) delete comedicounterwindow;
#endif
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
  const QString sLogPath(qdir.currentPath() + QDir::separator() +
                         "OpticsBenchUI_" + Utils::CurrentTimestampString() +
                         ".log");
  QsLogging::DestinationPtr fileDestination(QsLogging::DestinationFactory::MakeFileDestination(sLogPath) );
  QsLogging::DestinationPtr debugDestination(QsLogging::DestinationFactory::MakeDebugOutputDestination() );
  logger.addDestination(debugDestination.get());
  logger.addDestination(fileDestination.get());
  logger.setLoggingLevel(DEBUG_LEVEL);

  QApplication app(argc, argv); 
  app.setStyle("Fusion");
  QPalette darkPalette;
  darkPalette.setColor(QPalette::Window, QColor(30, 30, 30));
  darkPalette.setColor(QPalette::WindowText, QColor(230, 230, 230));
  darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
  darkPalette.setColor(QPalette::AlternateBase, QColor(35, 35, 35));
  darkPalette.setColor(QPalette::ToolTipBase, QColor(230, 230, 230));
  darkPalette.setColor(QPalette::ToolTipText, QColor(30, 30, 30));
  darkPalette.setColor(QPalette::Text, QColor(230, 230, 230));
  darkPalette.setColor(QPalette::Button, QColor(45, 45, 45));
  darkPalette.setColor(QPalette::ButtonText, QColor(230, 230, 230));
  darkPalette.setColor(QPalette::BrightText, QColor(255, 80, 80));
  darkPalette.setColor(QPalette::Link, QColor(138, 180, 248));
  darkPalette.setColor(QPalette::Highlight, QColor(88, 132, 214));
  darkPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
  app.setPalette(darkPalette);
  app.addLibraryPath("/usr/local/lib"); 
  QLOG_INFO() << "Built with Qt" << QT_VERSION_STR << "running on" << qVersion();
  QLOG_INFO() << "Qt User Data location : " 
              << QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation); 
  QLOG_INFO() << "OpticsBenchUI version " << OPTICSBENCHUIVERSION
              << " started : " <<  app.applicationDirPath();
  OpticsBenchUIMain* OpticsBenchUI =
      new OpticsBenchUIMain(app.applicationDirPath(), nullptr, Qt::Window);
  OpticsBenchUI->setWindowTitle("OpticsBenchUI");
  OpticsBenchUI->show();
  foreach (const QString &path, app.libraryPaths())
  QLOG_DEBUG() << path;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  const QList<QScreen *> screens = QGuiApplication::screens();
  QLOG_INFO() << " Screen number " << screens.size();
  if (!screens.isEmpty()) {
    const QRect geom = screens.first()->geometry();
    QLOG_INFO() << " X,Y " << geom.width() << " " << geom.height();
  }
#else
  QDesktopWidget *desktop = QApplication::desktop();
  QLOG_INFO() << " Screen number " << desktop->screenCount();
  QLOG_INFO() << " Virtual Desktop " << desktop->isVirtualDesktop();
  QLOG_INFO() << " X,Y " << desktop->screenGeometry(0).width() << " "
              << desktop->screenGeometry(0).height();
#endif
  return app.exec();
}
