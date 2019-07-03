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

#ifndef CAMERAWINDOW_H
#define CAMERAWINDOW_H


#include "VideoPlayer.h"
#include "Camera.h"
#include "CameraControlWidget.h"
#include "CameraPropWidget.h"

#include "QsLog.h"

class CameraWindow : public QMainWindow
{
  
  Q_OBJECT
    
    public:
  
  CameraWindow( QMainWindow* parent = 0, Qt::WFlags fl = Qt::Window, Camera *_camera = 0, 
		int cameraNumber = 0);
  virtual ~CameraWindow();
  
  void update();

 public slots:
  void set480x320();
  void set640x480();
  void set1280x960();

 signals:
  void setVideoPlayerResolution(int,int);
  
 protected:
  void closeEvent(QCloseEvent *event);
    
 private:

  QMainWindow *parentWindow;
  CameraControlWidget *cameraWidget;
  CameraPropWidget *cameraPropWidget;
  Camera *camera;
  int cameraNumber;
  VideoPlayer *player;
};
#endif // CAMERAWINDOW_H
