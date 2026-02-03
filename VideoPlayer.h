#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H
#include <QMediaPlayer>
#include <QMovie>
#include <QWidget>
#include "Camera.h"
#include "QsLog.h"

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
    QMediaPlayer mediaPlayer;
    /// Present an image to the rendering surface.
    bool presentImage(const QImage &image);
 
    QAbstractVideoSurface *surface;
    QImage image;

};
#endif // VIDEOPLAYER_H
