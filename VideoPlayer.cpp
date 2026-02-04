/**
 * @file VideoPlayer.cpp
 * @brief Video Player component.
 */
#include "VideoPlayer.h"
#include "VideoWidget.h"
#include <QtWidgets>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && !defined(NO_MULTIMEDIA)
#include <QVideoFrame>
#include <qvideosurfaceformat.h>
#endif

VideoPlayer::VideoPlayer(QWidget *parent, Camera *_camera)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && !defined(NO_MULTIMEDIA)
    : QWidget(parent)
    , mediaPlayer(0, QMediaPlayer::VideoSurface)
#else
    : QWidget(parent)
#endif
{
    camera = _camera;
    connect(camera, SIGNAL(getImage(const QImage &)),this, SLOT(setImageFromCamera(const QImage &)));
    connect(this, SIGNAL(setImageSize(const int &,const int &)),camera, SLOT(setImageSize(const int &,const int &)));
    
    this->setVideoPlayerResolution(480,320);
    
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(10);
    camera->start();

    videoWidget = new VideoWidget;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && !defined(NO_MULTIMEDIA)
    surface = videoWidget->videoSurface();
#endif
    QBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(videoWidget);
    setLayout(layout);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && !defined(NO_MULTIMEDIA)
    mediaPlayer.setVideoOutput(videoWidget->videoSurface());
#endif
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && !defined(NO_MULTIMEDIA)
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
#else
  if (videoWidget == nullptr) {
    return false;
  }

  videoWidget->setImage(image);
  return true;
#endif
}

void VideoPlayer::setVideoPlayerResolution(int width,int height) {
  emit setImageSize(width,height);
  this->setMaximumSize(QSize(width+20,height+20));
  this->setMinimumSize(QSize(width+20,height+20));
   
}
