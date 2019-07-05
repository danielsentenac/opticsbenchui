#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H
#include <QMediaPlayer>
#include <QMovie>
#include <QWidget>
#include "Camera.h"
#include "QsLog.h"

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
    QMediaPlayer mediaPlayer;
    bool presentImage(const QImage &image);
 
    QAbstractVideoSurface *surface;
    QImage image;

};
#endif // VIDEOPLAYER_H

