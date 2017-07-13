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

#ifndef VIDEOWIDGETSURFACE_H
#define VIDEOWIDGETSURFACE_H

 #include <QtCore/QRect>
 #include <QtGui/QImage>
 #include <QtMultimedia/QAbstractVideoSurface>
 #include <QtMultimedia/QVideoFrame>
 #include <QtMultimedia>

 class VideoWidgetSurface : public QAbstractVideoSurface
 {
     Q_OBJECT
 public:
     VideoWidgetSurface(QWidget *widget, QObject *parent = 0);

     QList<QVideoFrame::PixelFormat> supportedPixelFormats(
             QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;
     bool isFormatSupported(const QVideoSurfaceFormat &format, QVideoSurfaceFormat *similar) const;

     bool start(const QVideoSurfaceFormat &format);
     void stop();

     bool present(const QVideoFrame &frame);

     QRect videoRect() const { return targetRect; }
     void updateVideoRect();

     void paint(QPainter *painter);

 private:
     QWidget *widget;
     QImage::Format imageFormat;
     QRect targetRect;
     QSize imageSize;
     QRect sourceRect;
     QVideoFrame currentFrame;
 };

 #endif
