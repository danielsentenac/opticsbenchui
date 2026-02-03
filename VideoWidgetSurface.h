#ifndef VIDEOWIDGETSURFACE_H
#define VIDEOWIDGETSURFACE_H
#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && !defined(NO_MULTIMEDIA)
#include <QAbstractVideoSurface>
#include <QImage>
#include <QRect>
#include <QVideoFrame>
//! [0]
/// \ingroup ui
/// Video surface that renders frames into a widget.
class VideoWidgetSurface : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    /// Construct a video surface for a widget.
    /// \param widget Target widget.
    /// \param parent Parent object.
    VideoWidgetSurface(QWidget *widget, QObject *parent = 0);
    /// Return the supported pixel formats.
    /// \param handleType Buffer handle type.
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const override;
    /// Check whether a format is supported.
    /// \param format Video surface format.
    bool isFormatSupported(const QVideoSurfaceFormat &format) const override;
    /// Start rendering with a format.
    /// \param format Video surface format.
    bool start(const QVideoSurfaceFormat &format) override;
    /// Stop rendering.
    void stop() override;
    /// Present a new frame.
    /// \param frame Video frame.
    bool present(const QVideoFrame &frame) override;
    /// Return the target rectangle for video.
    QRect videoRect() const { return targetRect; }
    /// Recompute the video rectangle.
    void updateVideoRect();
    /// Paint the current frame.
    /// \param painter Painter to draw with.
    void paint(QPainter *painter);
private:
    QWidget *widget;
    QImage::Format imageFormat;
    QRect targetRect;
    QSize imageSize;
    QRect sourceRect;
    QVideoFrame currentFrame;
};
//! [0]
#endif
#endif
