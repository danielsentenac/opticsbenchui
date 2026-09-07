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
    connect(videoWidget, SIGNAL(pixelHovered(const QPointF &)),
            this, SLOT(onPixelHovered(const QPointF &)));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && !defined(NO_MULTIMEDIA)
    surface = videoWidget->videoSurface();
#endif
    // The scroll area is transparent in fit mode (the widget fills the
    // viewport) and provides panning in native 1:1 mode, where the widget
    // grows to the frame size.
    scrollArea = new QScrollArea(this);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(videoWidget);
    QBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(scrollArea);
    setLayout(layout);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && !defined(NO_MULTIMEDIA)
    mediaPlayer.setVideoOutput(videoWidget->videoSurface());
#endif
}
VideoPlayer::~VideoPlayer()
{
   // Do not stop camera here; acquisition may be running without UI.
}

void VideoPlayer::closeEvent(QCloseEvent* event)
{
  event->accept();
  QLOG_DEBUG ( ) << "Closing VideoPlayer";
  delete this;
}

void VideoPlayer::update() {
  // Present only when a new frame arrived: full-resolution frames make a
  // redundant repaint every 10 ms expensive.
  if (!imageDirty) {
    return;
  }
  imageDirty = false;
  presentImage(image);
}

void VideoPlayer::setImageFromCamera(const QImage &_image) {
  image = _image;
  imageDirty = true;
}

void VideoPlayer::setNativeScale(bool on) {
  if (videoWidget != nullptr) {
    videoWidget->setNativeScale(on);
  }
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

void VideoPlayer::onPixelHovered(const QPointF &imagePos) {
  // The displayed image is the sensor frame rescaled to the player
  // resolution: map back to sensor coordinates and read the value from the
  // full-resolution acquisition buffer. 16-bit backends fill buffer16 with
  // the raw counts; prefer it over the 8-bit display rendition.
  // No acquireMutex here: the acquisition thread holds it for essentially
  // the whole cycle, so locking would starve the readout. A single-element
  // read is atomic in practice and a concurrent frame write only makes the
  // value momentarily stale.
  if (camera == nullptr || image.isNull()) {
    return;
  }
  const unsigned int w = camera->width;
  const unsigned int h = camera->height;
  const ushort *buf16 = camera->buffer16;
  const uchar *buf8 = camera->buffer;
  if (w == 0 || h == 0 || (buf8 == nullptr && buf16 == nullptr)
      || image.width() == 0 || image.height() == 0) {
    return;
  }
  const int sx = qBound(0,
      static_cast<int>(imagePos.x() * w / image.width()),
      static_cast<int>(w) - 1);
  const int sy = qBound(0,
      static_cast<int>(imagePos.y() * h / image.height()),
      static_cast<int>(h) - 1);
  const size_t idx = static_cast<size_t>(sy) * w + sx;
  const int value = (buf16 != nullptr) ? buf16[idx] : buf8[idx];
  emit pixelValue(sx, sy, value);
}

void VideoPlayer::setVideoPlayerResolution(int width,int height) {
  emit setImageSize(width,height);
  this->setMaximumSize(QSize(width+20,height+20));
  this->setMinimumSize(QSize(width+20,height+20));
   
}
