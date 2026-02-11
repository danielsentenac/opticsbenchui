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
 * @file OpticsBenchUIMain.h
 * @brief Main application window.
 */

#ifndef OPTICSBENCHUIMAIN_H
#define OPTICSBENCHUIMAIN_H

#include "PosixCompat.h"


#include <QApplication>
#include <QtSql>
#include <QtWidgets>
#include <QTimer>

#include "CameraWindow.h"
#include "MotorWindow.h"
#include "SuperKWindow.h"
#include "AcquisitionWidget.h"
#include "AnalysisWidget.h"
#include "DacWindow.h"
#include "Dac.h"
#if defined(COMEDICOUNTER) || defined(COMEDIDAC)
#include "ComediWindow.h"
#include "Comedi.h"
#endif
#include "RaspiWindow.h"
#include "Raspi.h"
#include "Motor.h"
#include "SuperK.h"
#include "QsLog.h"
#include "QsLogDest.h"
#include "Assistant.h"

#ifdef COMEDICOUNTER
#include "ComediCounter.h"
#endif
#ifdef COMEDIDAC
#include "ComediDac.h"
#endif
#ifdef RASPIDAC
#include "RaspiDac.h"
#endif
#ifdef ADVANTECHDAC
#include "DacAdvantech.h"
#endif
#ifdef USBCAMERA
#include "CameraUSB.h"
#endif
#ifdef IEEE1394CAMERA
#include "CameraIEEE1394.h"
#endif
#ifdef GIGECAMERA
#include "CameraGiGE.h"
#endif
#ifdef NEOCAMERA
#include "CameraNeo.h"
#endif
#ifdef ZYLACAMERA
#include "CameraZyla.h"
#endif
#ifdef RAPTORFALCONCAMERA
#include "CameraRAPTORFALCON.h"
#endif
#ifdef RAPTORNINOX640CAMERA
#include "CameraRAPTORNINOX640.h"
#endif
#ifdef RASPICAMERA
#include "CameraRaspi.h"
#endif
#ifdef ALLIEDVISIONCAMERA
#include "CameraAlliedVision.h"
#endif

/// \ingroup ui
/// Main application window coordinating device windows and widgets.
class OpticsBenchUIMain : public QMainWindow
{
  
  Q_OBJECT
    
    public:
  
  /// Construct the main window.
  /// \param appDirPath Application directory for resources and DBs.
  /// \param parent Parent window.
  /// \param fl Window flags.
  OpticsBenchUIMain( QString appDirPath = 0, QMainWindow* parent = 0, Qt::WindowFlags fl = Qt::Window );
  
  /// Destructor.
  virtual ~OpticsBenchUIMain();
  
  /// Per-camera open flags.
  QVector<bool> isopencamerawindow;
  /// True if analysis widget is open.
  bool isopenanalysiswidget;
  /// True if acquisition widget is open.
  bool isopenacquisitionwidget;

  /// Track camera window open/close state.
  /// \param isopen True if the window is open.
  /// \param cameranumber Camera index.
  void setOpenCameraWindow(bool isopen, int cameranumber);

 signals:
  /// Emit open/close state for camera windows.
  /// \param isopencamerawindow Per-camera open flags.
  void isopenCameraWindow(QVector<bool> isopencamerawindow);
  /// Emit new database path.
  /// \param path Filesystem path to DBs.
  void setDbPath(QString path);
  /// Emit new acquisition file path/prefix.
  /// \param acqfile Acquisition file base.
  void setAcqFile(QString acqfile);

 public slots:
  /// Open acquisition UI.
  void openacquisition();
  /// Open analysis UI.
  void openanalysis();
  /// Open a camera window by index.
  /// \param cameraNumber Camera index.
  void openCameraWindow(int cameraNumber);
  /// Open the motor control window.
  void openMotorWindow();
  /// Open the SuperK control window.
  void openSuperKWindow();
  /// Open the DAC control window.
  void openDacWindow();
#if defined(COMEDICOUNTER) || defined(COMEDIDAC)
  /// Open the Comedi counter window.
  void openComediCounterWindow();
  /// Open the Comedi DAC window.
  void openComediDacWindow();
#endif
  /// Open the Raspi DAC window.
  void openRaspiDacWindow();
  /// Display DAC warning message.
  /// \param message Warning text.
  void showDacWarning(QString message);
  /// Display Comedi warning message.
  /// \param message Warning text.
  void showComediWarning(QString message);
  /// Display Raspi warning message.
  /// \param message Warning text.
  void showRaspiWarning(QString message);
  /// Display motor warning message.
  /// \param message Warning text.
  void showMotorWarning(QString message);
  /// Display SuperK warning message.
  /// \param message Warning text.
  void showSuperKWarning(QString message);
  /// Display camera warning message.
  /// \param message Warning text.
  void showCameraWarning(QString message);
  /// Display camera control warning message.
  /// \param message Warning text.
  void showCameraControlWidgetWarning(QString message);
  /// Display acquisition warning message.
  /// \param message Warning text.
  void showAcquisitionWarning(QString message);
  /// Update flashing state for acquisition tab.
  /// \param running True while acquisition is active.
  void setAcquisitionRunning(bool running);
  /// Update flashing state for analysis tab.
  /// \param running True while analysis is active.
  void setAnalysisRunning(bool running);
  /// Timer slot to flash running tabs.
  void flashRunningTabs();
  /// Show help documentation.
  void showDocumentation();
  /// Show API documentation in the default browser.
  void showApiDocumentation();
  /// Open the configuration dialog.
  void openConfiguration();
  /// Apply and propagate a configuration directory.
  /// \param path Directory containing *.db3 files.
  void applyConfigurationDirectory(const QString& path);
  /// Save all configuration databases (*.db3) to a selected directory.
  void saveConfiguration();
  /// Close the tab at a given index.
  /// \param index Tab index.
  void closeTab(int index);

 protected:
  /// Handle application close events.
  /// \param event Close event.
  void closeEvent(QCloseEvent *event);
  /// Handle global key press events.
  /// \param e Key press event.
  void keyPressEvent(QKeyEvent *e);

 private:
  QString appDirPath;
  QString configDirPath;
  Assistant *assistant;

  QSignalMapper *signalMapper;
  MotorWindow   *motorwindow;
  SuperKWindow  *superkwindow;
  DacWindow     *dacwindow;
#if defined(COMEDICOUNTER) || defined(COMEDIDAC)
  ComediWindow  *comedicounterwindow, *comedidacwindow;
#endif
  RaspiWindow   *raspidacwindow;
  QTabWidget    *tab;
  AcquisitionWidget *acquisitionwidget;
  AnalysisWidget    *analysiswidget;
  QTimer* tabFlashTimer;
  bool flashOn;
  bool acquisitionRunning;
  bool analysisRunning;
  Dac      *dac;
#if defined(COMEDICOUNTER) || defined(COMEDIDAC)
  Comedi   *comedicounter, *comedidac;
#endif
  Raspi    *raspidac;
  Motor    *motor;
  SuperK   *superk;
  Camera   *cameraUSBMgr;
  Camera   *cameraIEEE1394Mgr;
  Camera   *cameraGiGEMgr;
  Camera   *cameraNeoMgr;
  Camera   *cameraZylaMgr;
  Camera   *cameraRAPTORFALCONMgr;
  Camera   *cameraRAPTORNINOX640Mgr;
  Camera   *cameraRaspiMgr;
  Camera   *cameraAlliedVisionMgr;
  QVector<Camera*> cameraList;
  QVector<CameraWindow*> camerawindowList;
  
};
#endif // OPTICSBENCHUIMAIN_H
