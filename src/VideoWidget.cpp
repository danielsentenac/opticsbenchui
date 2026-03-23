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
    return surface->surfaceFormat().sizeHint();
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
        QRect selRect = QRect(selectionStart, selectionEnd).normalized();
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

void VideoWidget::applyZoom()
{
    if (zoomRect.isValid())
        surface->setZoomRect(zoomRect.toRect());
    else
        surface->setZoomRect(QRect());
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
}

VideoWidget::~VideoWidget()
{
}

void VideoWidget::setImage(const QImage &image)
{
    currentImage = image;
    updateVideoRect();
    update();
}

QSize VideoWidget::sizeHint() const
{
    if (!currentImage.isNull()) {
        return currentImage.size();
    }

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
        QRect selRect = QRect(selectionStart, selectionEnd).normalized();
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

    QSize size;
    if (zoomRect.isValid()) {
        // Maintain the zoom region's aspect ratio without cropping
        size = QSize(static_cast<int>(zoomRect.width()),
                     static_cast<int>(zoomRect.height()));
        size.scale(this->size(), Qt::KeepAspectRatio);
    } else {
        size = currentImage.size();
        size.scale(this->size(), Qt::KeepAspectRatioByExpanding);
    }
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

void VideoWidget::applyZoom()
{
    updateVideoRect();
    update();
}

#endif

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
    QWidget::mouseMoveEvent(event);
}

void VideoWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isSelecting) {
        isSelecting = false;
        selectionEnd = event->pos();

        const QRect selRect = QRect(selectionStart, selectionEnd).normalized();
        if (selRect.width() > 5 && selRect.height() > 5) {
            const QPointF imgTL = widgetToImage(selRect.topLeft());
            const QPointF imgBR = widgetToImage(selRect.bottomRight());
            QRectF newZoom = QRectF(imgTL, imgBR).normalized();
            if (newZoom.width() > 4 && newZoom.height() > 4) {
                zoomRect = newZoom;
                applyZoom();
                return;
            }
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
