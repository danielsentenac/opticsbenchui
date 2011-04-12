/*******************************************************************
This file is part of OpticsBenchUI.

OpticsBenchUI is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

#include "VideoWidget.h"

 VideoWidget::VideoWidget(QWidget *parent)
     : QWidget(parent)
     , surface(0)
 {
     setAutoFillBackground(false);
     setAttribute(Qt::WA_NoSystemBackground, true);
     setAttribute(Qt::WA_PaintOnScreen, true);

     QPalette palette = this->palette();
     palette.setColor(QPalette::Background, Qt::black);
     setPalette(palette);

     setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

     surface = new VideoWidgetSurface(this);
 }

 VideoWidget::~VideoWidget()
 {
     delete surface;
 }

 QSize VideoWidget::sizeHint() const
 {
     return surface->surfaceFormat().sizeHint();
 }

 void VideoWidget::paintEvent(QPaintEvent *event)
 {
     QPainter painter(this);

     if (surface->isActive()) {
         const QRect videoRect = surface->videoRect();

         if (!videoRect.contains(event->rect())) {
             QRegion region = event->region();
             region.subtract(videoRect);

             QBrush brush = palette().background();

             foreach (const QRect &rect, region.rects())
                 painter.fillRect(rect, brush);
         }

         surface->paint(&painter);
     } else {
         painter.fillRect(event->rect(), palette().background());
     }
 }

 void VideoWidget::resizeEvent(QResizeEvent *event)
 {
     QWidget::resizeEvent(event);

     surface->updateVideoRect();
 }
