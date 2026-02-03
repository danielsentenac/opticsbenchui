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
}
//! [3]
//! [4]
void VideoWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    surface->updateVideoRect();
}
//! [4]
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

    painter.drawImage(targetRect, currentImage);
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

    QSize size = currentImage.size();
    size.scale(this->size().boundedTo(size), Qt::KeepAspectRatio);
    targetRect = QRect(QPoint(0, 0), size);
    targetRect.moveCenter(rect().center());
}
#endif
