/**
 * @file VideoWidget.h
 * @brief Video widget.
 */
#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H
#include <QtGlobal>
#include <QWidget>
#include <QPoint>
#include <QRectF>
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
/// Supports rubber-band zoom (draw a rectangle with the mouse) and
/// double-click to reset to full view.
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
    /// Start a rubber-band zoom selection.
    void mousePressEvent(QMouseEvent *event) override;
    /// Update the rubber-band selection rectangle.
    void mouseMoveEvent(QMouseEvent *event) override;
    /// Finish the rubber-band selection and apply zoom.
    void mouseReleaseEvent(QMouseEvent *event) override;
    /// Reset zoom to full view.
    void mouseDoubleClickEvent(QMouseEvent *event) override;
private:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && !defined(NO_MULTIMEDIA)
    VideoWidgetSurface *surface;
#else
    void updateVideoRect();
    QImage currentImage;
    QRect targetRect;
#endif
    /// Currently visible region of the image (empty = full view).
    QRectF zoomRect;
    bool isSelecting = false;
    QPoint selectionStart;
    QPoint selectionEnd;
    /// Map a widget-space point to image-space coordinates given current zoom.
    QPointF widgetToImage(const QPoint &pt) const;
    /// Commit the current zoomRect to the display.
    void applyZoom();
};
//! [0]
#endif // VIDEOWIDGET_H
