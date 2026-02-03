#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H
#include "VideoWidgetSurface.h"
#include <QWidget>
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
    /// Return the underlying video surface.
    QAbstractVideoSurface *videoSurface() const { return surface; }
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
    VideoWidgetSurface *surface;
};
//! [0]
#endif // VIDEOWIDGET_H
