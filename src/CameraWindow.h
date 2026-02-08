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
 * @file CameraWindow.h
 * @brief Camera backend for Window.
 */

#ifndef CAMERAWINDOW_H
#define CAMERAWINDOW_H

#include <QMainWindow>
#include "Camera.h"
#include "CameraControlWidget.h"
#include "CameraPropWidget.h"
#ifndef NO_MULTIMEDIA
#include "VideoPlayer.h"
#else
class VideoPlayer;
#endif
#include "QsLog.h"

/// \ingroup ui_camera
/// Window hosting camera controls and live video.
class CameraWindow : public QMainWindow
{
  
  Q_OBJECT
    
    public:
  
  /// Construct a camera window.
  /// \param parent Parent window.
  /// \param fl Window flags.
  /// \param _camera Camera to display/control.
  /// \param cameraNumber Camera index.
  CameraWindow( QMainWindow* parent = 0, Qt::WindowFlags fl = Qt::Window, Camera *_camera = 0, 
		int cameraNumber = 0);
  /// Destructor.
  virtual ~CameraWindow();
  
  /// Refresh the UI.
  void update();

 public slots:
  /// Set video resolution to 480x320.
  void set480x320();
  /// Set video resolution to 640x480.
  void set640x480();
  /// Set video resolution to 1280x960.
  void set1280x960();

signals:
  /// Set video player resolution.
  /// \param width Width in pixels.
  /// \param height Height in pixels.
  void setVideoPlayerResolution(int width,int height);
  
 protected:
  /// Handle window close events.
  /// \param event Close event.
  void closeEvent(QCloseEvent *event);
    
 private:

  QMainWindow *parentWindow;
  CameraControlWidget *cameraWidget;
  CameraPropWidget *cameraPropWidget;
  Camera *camera;
  int cameraNumber;
#ifndef NO_MULTIMEDIA
  VideoPlayer *player;
#else
  QLabel *videoPlaceholder;
#endif
};
#endif // CAMERAWINDOW_H
