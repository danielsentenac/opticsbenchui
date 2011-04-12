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

#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QtGui/QMovie>
#include <QtGui/QWidget>
#include <QtMultimedia>
#include <QTimer>

#include "VideoWidget.h"
#include "Camera.h"
#include "QsLog.h"

class QAbstractButton;
class QAbstractVideoSurface;

class VideoPlayer : public QWidget
{
  Q_OBJECT
    public:
  VideoPlayer(QWidget *parent = 0, Camera *_camera = 0);
  ~VideoPlayer();
  Camera *camera;
  public slots:
  void setVideoPlayerResolution(int width, int height);

  private slots:
  void update();
  void setImageFromCamera(const QImage & _image);
  
 signals:
  void setImageSize(const int &width, const int &height);

 protected:
  void closeEvent(QCloseEvent *event);
 
 private:
  bool presentImage(const QImage &image);
 
  QAbstractVideoSurface *surface;
  QImage image;
};

#endif
