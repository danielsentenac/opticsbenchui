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

#ifndef OPTICSBENCHUIMAIN_H
#define OPTICSBENCHUIMAIN_H


#include <QApplication>
#include <QtSql>
#include <QtGui>

#include "CameraWindow.h"
#include "MotorWindow.h"
#include "AcquisitionWidget.h"
#include "AnalysisWidget.h"
#include "DacWindow.h"
#include "Dac.h"
#include "ComediWindow.h"
#include "Comedi.h"
#ifdef COMEDICOUNTER
#include "ComediCounter.h"
#endif
#ifdef ADVANTECHDAC
#include "DacAdvantech.h"
#endif
#include "Motor.h"
#ifdef IEEE1394CAMERA
#include "CameraIEEE1394.h"
#endif
#ifdef GIGECAMERA
#include "CameraGiGE.h"
#endif
#ifdef NEOCAMERA
#include "CameraNeo.h"
#endif
#include "QsLog.h"
#include "QsLogDest.h"
#include "Assistant.h"

class OpticsBenchUIMain : public QMainWindow
{
  
  Q_OBJECT
    
    public:
  
  OpticsBenchUIMain( QString appDirPath = 0, QMainWindow* parent = 0, Qt::WFlags fl = Qt::Window );
  
  virtual ~OpticsBenchUIMain();
  
  QVector<bool> isopencamerawindow;
  bool isopenanalysiswidget;
  bool isopenacquisitionwidget;

  void setOpenCameraWindow(bool isopen, int cameranumber);

 signals:
  void isopenCameraWindow(QVector<bool> isopencamerawindow);
  void setDbPath(QString path);
  void setAcqFile(QString acqfile);

 public slots:
  void openacquisition();
  void openanalysis();
  void openCameraWindow(int cameraNumber);
  void openMotorWindow();
  void openDacWindow();
  void openComediWindow();
  void showDacWarning(QString);
  void showComediWarning(QString);
  void showMotorWarning(QString);
  void showCameraWarning(QString);
  void showCameraControlWidgetWarning(QString);
  void showAcquisitionWarning(QString);
  void showDocumentation();
  void openConfiguration();
  void saveAcqFile();
  void closeTab(int index);

 protected:
  void closeEvent(QCloseEvent *event);
  void keyPressEvent(QKeyEvent *e);

 private:
  QString appDirPath;
  Assistant *assistant;

  QSignalMapper *signalMapper;
  MotorWindow  *motorwindow;
  DacWindow    *dacwindow;
  ComediWindow *comediwindow;
  QTabWidget   *tab;
  AcquisitionWidget *acquisitionwidget;
  AnalysisWidget *analysiswidget;
  Dac      *dac;
  Comedi   *comedi;
  Motor    *motor;
  Camera   *cameraIEEE1394Mgr;
  Camera   *cameraGiGEMgr;
  Camera   *cameraNeoMgr;
  QVector<Camera*> cameraList;
  QVector<CameraWindow*> camerawindowList;
  
};


#endif // OPTICSBENCHUIMAIN_H
