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

#ifndef OPTICSBENCHUIMAINUIMAIN_H
#define OPTICSBENCHUIMAINUIMAIN_H


#include <QApplication>
#include <QtSql>
#include <QtGui>

#include "CameraWindow.h"
#include "MotorWindow.h"
#include "DacWindow.h"
#include "AcquisitionWidget.h"
#include "AnalysisWidget.h"
#include "Dac.h"
#include "Motor.h"
#include "CameraIEEE1394.h"
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
  void showDacWarning(QString);
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
  QTabWidget   *tab;
  AcquisitionWidget *acquisitionwidget;
  AnalysisWidget *analysiswidget;
  Dac      *dac;
  Motor    *motor;
  Camera   *cameraIEEE1394Mgr;
  Camera   *cameraGiGEMgr;
  Camera   *cameraCameraLinkMgr;
  QVector<Camera*> cameraList;
  QVector<CameraWindow*> camerawindowList;

};


#endif // OPTICSBENCHUIMAINUIMAIN_H
