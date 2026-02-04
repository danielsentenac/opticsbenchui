/**
 * @file VideoWidget.h
 * @brief Video widget.
 */
#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H
#include <QtGlobal>
#include <QWidget>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QImage>
#include <QRect>
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && !defined(NO_MULTIMEDIA)
#include "VideoWidgetSurface.h"
#endif
//! [0]
/// \ingroup ui
/// Widget for rendering video frames from a surface.
class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    /// Construct a video widget.
    /// \param parent Parent widget.
    VideoWidget(QWidget *parent = 0);
    /// Destructor.
    ~VideoWidget();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && !defined(NO_MULTIMEDIA)
    /// Return the underlying video surface.
    QAbstractVideoSurface *videoSurface() const { return surface; }
#else
    /// Set the current image to display.
    /// \param image Latest video frame.
    void setImage(const QImage &image);
#endif
    /// Recommended size for layout.
    QSize sizeHint() const override;
protected:
    /// Paint the current frame.
    /// \param event Paint event.
    void paintEvent(QPaintEvent *event) override;
    /// Handle resize events.
    /// \param event Resize event.
    void resizeEvent(QResizeEvent *event) override;
private:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && !defined(NO_MULTIMEDIA)
    VideoWidgetSurface *surface;
#else
    void updateVideoRect();
    QImage currentImage;
    QRect targetRect;
#endif
};
//! [0]
#endif // VIDEOWIDGET_H
