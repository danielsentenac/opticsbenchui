/**
 * @file VideoWidget.cpp
 * @brief Video widget.
 */
#include "VideoWidget.h"
#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && !defined(NO_MULTIMEDIA)
#include "VideoWidgetSurface.h"
#include <QtWidgets>
#include <qvideosurfaceformat.h>
//! [0]
VideoWidget::VideoWidget(QWidget *parent)
    : QWidget(parent)
    , surface(0)
{
    setAutoFillBackground(false);
    setAttribute(Qt::WA_NoSystemBackground, true);
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::black);
    setPalette(palette);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
    surface = new VideoWidgetSurface(this);
}
//! [0]
//! [1]
VideoWidget::~VideoWidget()
{
    delete surface;
}
//! [1]
//! [2]
QSize VideoWidget::sizeHint() const
{
    // Only the unzoomed native view asks for the full frame size (so the
    // scroll area pans); zoomed views fit the viewport.
    const QSize src = sourceSize();
    if (nativeScale && !zoomRect.isValid() && !src.isEmpty())
        return src;
    return QSize(320, 240);
}
//! [2]
//! [3]
void VideoWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if (surface->isActive()) {
        const QRect videoRect = surface->videoRect();
        if (!videoRect.contains(event->rect())) {
            QRegion region = event->region();
            region = region.subtracted(videoRect);
            QBrush brush = palette().window();
            for (const QRect &rect : region.rects())
                painter.fillRect(rect, brush);
        }
        surface->paint(&painter);
    } else {
        painter.fillRect(event->rect(), palette().window());
    }
    // Draw rubber-band selection rectangle
    if (isSelecting) {
        const QRect selRect = selectionRect();
        painter.setPen(QPen(Qt::white, 1, Qt::DashLine));
        painter.setBrush(QColor(255, 255, 255, 30));
        painter.drawRect(selRect);
    }
}
//! [3]
//! [4]
void VideoWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    surface->updateVideoRect();
}
//! [4]

QPointF VideoWidget::widgetToImage(const QPoint &pt) const
{
    QRect tgt = surface->videoRect();
    if (tgt.width() == 0 || tgt.height() == 0)
        return QPointF();
    QRectF src = zoomRect.isValid() ? zoomRect : QRectF(surface->getSourceRect());
    const float relX = (pt.x() - tgt.x()) / static_cast<float>(tgt.width());
    const float relY = (pt.y() - tgt.y()) / static_cast<float>(tgt.height());
    QPointF result(src.x() + relX * src.width(), src.y() + relY * src.height());
    result.setX(qBound(src.left(), result.x(), src.right()));
    result.setY(qBound(src.top(), result.y(), src.bottom()));
    return result;
}

QPointF VideoWidget::imageToWidget(const QPointF &pt) const
{
    const QRect tgt = surface->videoRect();
    if (tgt.width() == 0 || tgt.height() == 0)
        return QPointF();
    const QRectF src = zoomRect.isValid() ? zoomRect : QRectF(surface->getSourceRect());
    const float relX = (pt.x() - src.x()) / static_cast<float>(src.width());
    const float relY = (pt.y() - src.y()) / static_cast<float>(src.height());
    QPointF result(tgt.x() + relX * tgt.width(), tgt.y() + relY * tgt.height());
    result.setX(qBound(static_cast<qreal>(tgt.left()), result.x(), static_cast<qreal>(tgt.right())));
    result.setY(qBound(static_cast<qreal>(tgt.top()), result.y(), static_cast<qreal>(tgt.bottom())));
    return result;
}

void VideoWidget::applyZoom()
{
    if (zoomRect.isValid())
        surface->setZoomRect(zoomRect.toRect());
    else
        surface->setZoomRect(QRect());
    updateGeometry();  // native mode: size hint follows the zoom region
    surface->updateVideoRect();
    update();
}

#else
#include <QtWidgets>
VideoWidget::VideoWidget(QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(false);
    setAttribute(Qt::WA_NoSystemBackground, true);
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::black);
    setPalette(palette);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
}

VideoWidget::~VideoWidget()
{
}

void VideoWidget::setImage(const QImage &image)
{
    const bool sizeChanged = (image.size() != currentImage.size());
    currentImage = image;
    if (sizeChanged)
        updateGeometry();
    updateVideoRect();
    update();
}

QSize VideoWidget::sizeHint() const
{
    // Only the unzoomed native view asks for the full frame size (so the
    // scroll area pans); zoomed views fit the viewport.
    const QSize src = sourceSize();
    if (nativeScale && !zoomRect.isValid() && !src.isEmpty())
        return src;
    return QSize(320, 240);
}

void VideoWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(event->rect(), palette().window());
    if (currentImage.isNull()) {
        return;
    }

    const QRectF srcRect = zoomRect.isValid() ? zoomRect : QRectF(currentImage.rect());
    painter.drawImage(targetRect, currentImage, srcRect);

    // Draw rubber-band selection rectangle
    if (isSelecting) {
        const QRect selRect = selectionRect();
        painter.setPen(QPen(Qt::white, 1, Qt::DashLine));
        painter.setBrush(QColor(255, 255, 255, 30));
        painter.drawRect(selRect);
    }
}

void VideoWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateVideoRect();
}

void VideoWidget::updateVideoRect()
{
    if (currentImage.isNull()) {
        targetRect = QRect();
        return;
    }

    // Fit mode, and any zoomed region: letterbox into the widget so the aspect
    // ratio is preserved (a zoom selection is magnified with nearest-neighbour
    // sampling, so sensor pixels stay visible as blocks). Native mode on the
    // full frame: 1 image pixel per screen pixel; the widget is at least as
    // large as the frame (see sizeHint) and the enclosing scroll area provides
    // panning.
    QSize size = sourceSize();
    if (!nativeScale || zoomRect.isValid())
        size.scale(this->size(), Qt::KeepAspectRatio);
    targetRect = QRect(QPoint(0, 0), size);
    targetRect.moveCenter(rect().center());
}

QPointF VideoWidget::widgetToImage(const QPoint &pt) const
{
    if (currentImage.isNull() || targetRect.width() == 0 || targetRect.height() == 0)
        return QPointF();
    QRectF src = zoomRect.isValid() ? zoomRect : QRectF(currentImage.rect());
    const float relX = (pt.x() - targetRect.x()) / static_cast<float>(targetRect.width());
    const float relY = (pt.y() - targetRect.y()) / static_cast<float>(targetRect.height());
    QPointF result(src.x() + relX * src.width(), src.y() + relY * src.height());
    result.setX(qBound(src.left(), result.x(), src.right()));
    result.setY(qBound(src.top(), result.y(), src.bottom()));
    return result;
}

QPointF VideoWidget::imageToWidget(const QPointF &pt) const
{
    if (currentImage.isNull() || targetRect.width() == 0 || targetRect.height() == 0)
        return QPointF();
    const QRectF src = zoomRect.isValid() ? zoomRect : QRectF(currentImage.rect());
    const float relX = (pt.x() - src.x()) / static_cast<float>(src.width());
    const float relY = (pt.y() - src.y()) / static_cast<float>(src.height());
    QPointF result(targetRect.x() + relX * targetRect.width(),
                   targetRect.y() + relY * targetRect.height());
    result.setX(qBound(static_cast<qreal>(targetRect.left()), result.x(), static_cast<qreal>(targetRect.right())));
    result.setY(qBound(static_cast<qreal>(targetRect.top()), result.y(), static_cast<qreal>(targetRect.bottom())));
    return result;
}

void VideoWidget::applyZoom()
{
    updateGeometry();  // native mode: size hint follows the zoom region
    updateVideoRect();
    update();
}

#endif

void VideoWidget::setNativeScale(bool on)
{
    if (nativeScale == on)
        return;
    nativeScale = on;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && !defined(NO_MULTIMEDIA)
    surface->setNativeScale(on);
    updateGeometry();
    surface->updateVideoRect();
#else
    updateGeometry();
    updateVideoRect();
#endif
    update();
}

QSize VideoWidget::sourceSize() const
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && !defined(NO_MULTIMEDIA)
    return surface->sourceSize();
#else
    if (zoomRect.isValid())
        return zoomRect.size().toSize();
    return currentImage.size();
#endif
}

QRect VideoWidget::displayRect() const
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && !defined(NO_MULTIMEDIA)
    return surface->videoRect();
#else
    return targetRect;
#endif
}

QRect VideoWidget::selectionRect() const
{
    const QRectF imageRect = selectionImageRect();
    if (!imageRect.isValid())
        return QRect();
    return QRect(imageToWidget(imageRect.topLeft()).toPoint(),
                 imageToWidget(imageRect.bottomRight()).toPoint()).normalized();
}

QRectF VideoWidget::selectionImageRect() const
{
    // Constrain the selection to the aspect ratio of the visible display so
    // the zoomed region fills the view with no bars. The display is the scroll
    // area viewport when hosted in one (in native mode this widget itself can
    // be larger than what is visible), else the widget.
    const QWidget *display = parentWidget() ? parentWidget() : this;
    const qreal aspect = (display->height() > 0)
        ? static_cast<qreal>(display->width()) / display->height() : 1.0;
    const QPointF imageStart = widgetToImage(selectionStart);
    const QPointF imageEnd = widgetToImage(selectionEnd);
    const qreal deltaX = imageEnd.x() - imageStart.x();
    const qreal deltaY = imageEnd.y() - imageStart.y();
    // Largest aspect-correct rectangle inside the dragged box
    const qreal w = qMin(qAbs(deltaX), qAbs(deltaY) * aspect);
    const qreal h = w / aspect;
    const qreal endX = imageStart.x() + (deltaX < 0 ? -w : w);
    const qreal endY = imageStart.y() + (deltaY < 0 ? -h : h);
    return QRectF(imageStart, QPointF(endX, endY)).normalized();
}

// ---------------------------------------------------------------------------
// Mouse events – shared by both Qt5 and Qt6 paths
// ---------------------------------------------------------------------------

void VideoWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isSelecting = true;
        selectionStart = event->pos();
        selectionEnd   = event->pos();
    }
    QWidget::mousePressEvent(event);
}

void VideoWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (isSelecting) {
        selectionEnd = event->pos();
        update();
    }
    if (displayRect().contains(event->pos()))
        emit pixelHovered(widgetToImage(event->pos()));
    QWidget::mouseMoveEvent(event);
}

void VideoWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isSelecting) {
        isSelecting = false;
        selectionEnd = event->pos();

        const QRectF newZoom = selectionImageRect();
        if (newZoom.width() > 4 && newZoom.height() > 4) {
            zoomRect = newZoom;
            applyZoom();
            return;
        }
        update();
    }
    QWidget::mouseReleaseEvent(event);
}

void VideoWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        zoomRect    = QRectF();
        isSelecting = false;
        applyZoom();
    }
    QWidget::mouseDoubleClickEvent(event);
}
