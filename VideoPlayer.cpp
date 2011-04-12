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

#include "VideoPlayer.h"

VideoPlayer::VideoPlayer(QWidget *parent, Camera *_camera)
  : QWidget(parent)
  , surface(0)
{
  camera = _camera;
  if (camera->camera_err == 0) { 
  
    connect(camera, SIGNAL(getImage(const QImage &)),this, SLOT(setImageFromCamera(const QImage &)));
    connect(this, SIGNAL(setImageSize(const int &,const int &)),camera, SLOT(setImageSize(const int &,const int &)));
    
    this->setVideoPlayerResolution(480,320);
    
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(10);
    camera->start();
    
    VideoWidget *videoWidget = new VideoWidget(this);
    surface = videoWidget->videoSurface();
    
    QBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(videoWidget);
    
    setLayout(layout);
  }
}

VideoPlayer::~VideoPlayer()
{
  camera->stop();
  QLOG_DEBUG ( ) << "Deleting VideoPlayer";
}
void VideoPlayer::closeEvent(QCloseEvent* event)
{
  event->accept();
  QLOG_DEBUG ( ) << "Closing VideoPlayer";
  delete this;
}

void VideoPlayer::update() {
  presentImage(image);
}

void VideoPlayer::setImageFromCamera(const QImage &_image) {
  image = _image;
  
}

bool VideoPlayer::presentImage(const QImage &image)
{
  QVideoFrame frame(image);
  if (!frame.isValid()) {
    QLOG_DEBUG ( ) << "Invalid frame";
    return false;
  }
  
  QVideoSurfaceFormat currentFormat = surface->surfaceFormat();

  if (frame.pixelFormat() != currentFormat.pixelFormat()
      || frame.size() != currentFormat.frameSize()) {
    QVideoSurfaceFormat format(frame.size(), frame.pixelFormat());

    if (!surface->start(format)) {
      QLOG_DEBUG ( ) << "Start surface failed";
      return false;
    }
  }

  if (!surface->present(frame)) {
    surface->stop(); 

    return false;
  } else {
    return true;
  }
}
void VideoPlayer::setVideoPlayerResolution(int width,int height) {
  emit setImageSize(width,height);
  this->setMaximumSize(QSize(width,height));
  this->setMinimumSize(QSize(width,height));
}
