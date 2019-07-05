#include "VideoPlayer.h"
#include "VideoWidget.h"
#include <QtWidgets>
#include <qvideosurfaceformat.h>
VideoPlayer::VideoPlayer(QWidget *parent, Camera *_camera)
    : QWidget(parent)
    , mediaPlayer(0, QMediaPlayer::VideoSurface)
{
    camera = _camera;
    connect(camera, SIGNAL(getImage(const QImage &)),this, SLOT(setImageFromCamera(const QImage &)));
    connect(this, SIGNAL(setImageSize(const int &,const int &)),camera, SLOT(setImageSize(const int &,const int &)));
    
    this->setVideoPlayerResolution(480,320);
    
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(10);
    camera->start();

    VideoWidget *videoWidget = new VideoWidget;
    surface = videoWidget->videoSurface();
    QBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(videoWidget);
    setLayout(layout);
    mediaPlayer.setVideoOutput(videoWidget->videoSurface());
}
VideoPlayer::~VideoPlayer()
{
   camera->stop();
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
  this->setMaximumSize(QSize(width+20,height+20));
  this->setMinimumSize(QSize(width+20,height+20));
   
}

