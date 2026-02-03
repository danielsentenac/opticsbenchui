#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H
#include <QMovie>
#include <QtGlobal>
#include <QWidget>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && !defined(NO_MULTIMEDIA)
#include <QMediaPlayer>
#include <QAbstractVideoSurface>
#endif
#include "Camera.h"
#include "QsLog.h"

class VideoWidget;

/// \ingroup ui
/// Widget for displaying live camera frames.
class VideoPlayer : public QWidget
{
    Q_OBJECT
public:
    /// Construct a video player for a camera.
    /// \param parent Parent widget.
    /// \param _camera Camera providing frames.
    VideoPlayer(QWidget *parent = 0, Camera *_camera = 0);
    /// Destructor.
    ~VideoPlayer();
    /// Camera providing frames.
    Camera *camera;
  public slots:
  /// Set the video player resolution.
  /// \param width Width in pixels.
  /// \param height Height in pixels.
  void setVideoPlayerResolution(int width, int height);

  private slots:
  /// Periodic update to fetch frames.
  void update();
  /// Receive a frame and display it.
  /// \param _image New frame image.
  void setImageFromCamera(const QImage & _image);
  
 signals:
  /// Request a new image size.
  /// \param width Width in pixels.
  /// \param height Height in pixels.
  void setImageSize(const int &width, const int &height);

 protected:
  /// Handle window close events.
  /// \param event Close event.
  void closeEvent(QCloseEvent *event);
private:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && !defined(NO_MULTIMEDIA)
    QMediaPlayer mediaPlayer;
    /// Present an image to the rendering surface.
    bool presentImage(const QImage &image);
    QAbstractVideoSurface *surface;
#else
    /// Present an image to the rendering widget.
    bool presentImage(const QImage &image);
#endif
    QImage image;
    VideoWidget *videoWidget = nullptr;

};
#endif // VIDEOPLAYER_H
