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

#include "CameraWindow.h"
#include "OpticsBenchUIMain.h"

#define OFFSET_X 250
#define OFFSET_Y 250

CameraWindow::CameraWindow( QMainWindow* parent, Qt::WindowFlags fl , Camera *_camera, int _cameraNumber)
  : QMainWindow( parent, fl )
{
  camera = _camera;
  cameraNumber = _cameraNumber;
  parentWindow = parent;
  OpticsBenchUIMain *OpticsBenchui = (OpticsBenchUIMain*) parentWindow;
  OpticsBenchui->setOpenCameraWindow(true,cameraNumber);
  player = new VideoPlayer(this,camera);
  cameraPropWidget = nullptr;
 
  if (camera->camera_err == 0) { 
    connect(this,SIGNAL(setVideoPlayerResolution(int,int)),player,
	    SLOT(setVideoPlayerResolution(int,int)));
    
    // Creation of menuBar
    QMenuBar* menuBar = new QMenuBar(this);
    menuBar->resize(15,15);
    // Creating a menu "File"
    QMenu* menuResolution = new QMenu("Resolution");
    
    // And adding the menu "menuResolution" in the menu bar
    menuBar->addMenu(menuResolution);
    
    // Add a separator
    menuResolution->addSeparator();
    
    menuResolution->addAction("480x320", this, SLOT(set480x320()) );
    menuResolution->addAction("640x480", this, SLOT(set640x480()) );
    menuResolution->addAction("1280x960", this, SLOT(set1280x960()) );
    
    cameraWidget = new CameraControlWidget(player->camera);
    QDockWidget *dockWidget = new QDockWidget(tr("Controls"), this);
    dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
    QScrollArea *controlArea = new QScrollArea(this);
    controlArea->setWidget(cameraWidget);
    controlArea->setWidgetResizable(true);
    dockWidget->setWidget(controlArea);
    this->addDockWidget(Qt::BottomDockWidgetArea, dockWidget);
    
    if ( player->camera->propList.size() > 0 ) {  
      cameraPropWidget = new CameraPropWidget(player->camera);
      QDockWidget *dockWidget = new QDockWidget(tr("Properties"), this);
      dockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
      QScrollArea *propArea = new QScrollArea(this);
      propArea->setWidget(cameraPropWidget);
      propArea->setWidgetResizable(true);
      dockWidget->setWidget(propArea);
      this->addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }
    this->setWindowTitle(QString(player->camera->vendor) + " / " + 
			 QString(player->camera->model));
    this->setCentralWidget(player);
    this->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    this->set480x320();
  }
  else
    this->setWindowTitle(tr("No Camera found"));
}
CameraWindow::~CameraWindow()
{
  QLOG_INFO ( ) << "Deleting CameraWindow";
  delete player;
  delete cameraWidget;
}
void CameraWindow::closeEvent(QCloseEvent* event)
{
  event->accept();
  QLOG_DEBUG ( ) << "Closing CameraWindow";
  OpticsBenchUIMain *OpticsBenchui = (OpticsBenchUIMain*) parentWindow;
  OpticsBenchui->setOpenCameraWindow(false,cameraNumber);
  delete this;
}
void CameraWindow::set480x320() {
  emit setVideoPlayerResolution(480,320);
  if (cameraPropWidget != nullptr) {
    this->setMaximumSize(QSize(480 + OFFSET_X, 320 + OFFSET_Y));
    //this->setMinimumSize(QSize(480 + OFFSET_X, 320 + OFFSET_Y));  
  }
  else {
    this->setMaximumSize(QSize(480, 320 + OFFSET_Y));
    //this->setMinimumSize(QSize(480, 320 + OFFSET_Y));
  }
}
void CameraWindow::set640x480() {
  emit setVideoPlayerResolution(640,480); 
  if (cameraPropWidget != nullptr) {
    this->setMaximumSize(QSize(640 + OFFSET_X, 480 + OFFSET_Y));
    //this->setMinimumSize(QSize(640 + OFFSET_X, 480 + OFFSET_Y));
  }
  else {
    this->setMaximumSize(QSize(640, 480 + OFFSET_Y));
    //this->setMinimumSize(QSize(640, 480 + OFFSET_Y));
  }
}
void CameraWindow::set1280x960() {
  emit setVideoPlayerResolution(1280,960);
  if (cameraPropWidget != nullptr) {
    this->setMaximumSize(QSize(1280 + OFFSET_X, 960 + OFFSET_Y));
    //this->setMinimumSize(QSize(1280 + OFFSET_X, 960 + OFFSET_Y));
  }
  else {
    this->setMaximumSize(QSize(1280, 960 + OFFSET_Y));
    //this->setMinimumSize(QSize(1280, 960 + OFFSET_Y));
  }
}
void CameraWindow::update() {
  camera->getFeatures();
}
