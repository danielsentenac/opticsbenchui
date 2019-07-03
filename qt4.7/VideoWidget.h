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

#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QtMultimedia>
#include <QtGui/QWidget>

#include "VideoWidgetSurface.h"


 

 class QAbstractVideoSurface;

 class VideoWidgetSurface;

 class VideoWidget : public QWidget
 {
     Q_OBJECT
 public:
     VideoWidget(QWidget *parent = 0);
     ~VideoWidget();

     QAbstractVideoSurface *videoSurface() const { return surface; }

     QSize sizeHint() const;

 protected:
     void paintEvent(QPaintEvent *event);
     void resizeEvent(QResizeEvent *event);

 private:
     VideoWidgetSurface *surface;
 };

 #endif
