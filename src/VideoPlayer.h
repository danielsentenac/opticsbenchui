/**
 * @file VideoPlayer.h
 * @brief Video Player component.
 */
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

class QScrollArea;
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
  /// Show one sensor pixel per screen pixel (scroll to pan) instead of
  /// fitting the frame to the player.
  /// \param on Enable native 1:1 rendering.
  void setNativeScale(bool on);

  private slots:
  /// Periodic update to fetch frames.
  void update();
  /// Receive a frame and display it.
  /// \param _image New frame image.
  void setImageFromCamera(const QImage & _image);
  /// Resolve the sensor pixel under the mouse and emit its value.
  /// \param imagePos Position in displayed-image coordinates.
  void onPixelHovered(const QPointF &imagePos);

 signals:
  /// Request a new image size.
  /// \param width Width in pixels.
  /// \param height Height in pixels.
  void setImageSize(const int &width, const int &height);
  /// Report the sensor pixel under the mouse pointer.
  /// \param x Sensor column.
  /// \param y Sensor row.
  /// \param value Pixel value from the acquisition buffer (0-255).
  void pixelValue(int x, int y, int value);

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
    /// Set when a new frame arrived and has not been presented yet.
    bool imageDirty = false;
    VideoWidget *videoWidget = nullptr;
    /// Hosts the video widget; provides panning in native 1:1 mode.
    QScrollArea *scrollArea = nullptr;

};
#endif // VIDEOPLAYER_H
