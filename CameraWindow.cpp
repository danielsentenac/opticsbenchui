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
#ifndef NO_MULTIMEDIA
  player = new VideoPlayer(this,camera);
#else
  videoPlaceholder = new QLabel(tr("Video disabled (NO_MULTIMEDIA)"));
  videoPlaceholder->setAlignment(Qt::AlignCenter);
  videoPlaceholder->setMinimumSize(320, 240);
#endif
  cameraPropWidget = nullptr;
 
  if (camera->camera_err == 0) { 
#ifndef NO_MULTIMEDIA
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
#endif

    cameraWidget = new CameraControlWidget(camera);
    QDockWidget *dockWidget = new QDockWidget(tr("Controls"), this);
    dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
    QScrollArea *controlArea = new QScrollArea(this);
    controlArea->setWidget(cameraWidget);
    controlArea->setWidgetResizable(true);
    dockWidget->setWidget(controlArea);
    this->addDockWidget(Qt::BottomDockWidgetArea, dockWidget);
    
    if ( camera->propList.size() > 0 ) {  
      cameraPropWidget = new CameraPropWidget(camera);
      QDockWidget *dockWidget = new QDockWidget(tr("Properties"), this);
      dockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
      QScrollArea *propArea = new QScrollArea(this);
      propArea->setWidget(cameraPropWidget);
      propArea->setWidgetResizable(true);
      dockWidget->setWidget(propArea);
      this->addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }
    this->setWindowTitle(QString(camera->vendor) + " / " + 
			 QString(camera->model));
#ifndef NO_MULTIMEDIA
    this->setCentralWidget(player);
    this->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    this->set480x320();
#else
    this->setCentralWidget(videoPlaceholder);
    const int width = (cameraPropWidget != nullptr) ? 480 + OFFSET_X : 480;
    resize(QSize(width, 320 + OFFSET_Y));
#endif
  }
  else
    this->setWindowTitle(tr("No Camera found"));
}
CameraWindow::~CameraWindow()
{
  QLOG_INFO ( ) << "Deleting CameraWindow";
#ifndef NO_MULTIMEDIA
  delete player;
#else
  delete videoPlaceholder;
#endif
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
#ifndef NO_MULTIMEDIA
  emit setVideoPlayerResolution(480,320);
  const int width = (cameraPropWidget != nullptr) ? 480 + OFFSET_X : 480;
  resize(QSize(width, 320 + OFFSET_Y));
#endif
}
void CameraWindow::set640x480() {
#ifndef NO_MULTIMEDIA
  emit setVideoPlayerResolution(640,480); 
  const int width = (cameraPropWidget != nullptr) ? 640 + OFFSET_X : 640;
  resize(QSize(width, 480 + OFFSET_Y));
#endif
}
void CameraWindow::set1280x960() {
#ifndef NO_MULTIMEDIA
  emit setVideoPlayerResolution(1280,960);
  const int width = (cameraPropWidget != nullptr) ? 1280 + OFFSET_X : 1280;
  resize(QSize(width, 960 + OFFSET_Y));
#endif
}
void CameraWindow::update() {
  camera->getFeatures();
}
