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

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QOpenGLWidget>
#else
#include <QGLWidget>
#endif
#include <QImage>

/// \ingroup ui
/// OpenGL-based widget for rendering images.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class PaintWidget : public QOpenGLWidget
#else
class PaintWidget : public QGLWidget
#endif
{
  Q_OBJECT
    
    public:
  /// Construct a paint widget.
  /// \param parent Parent widget.
  PaintWidget(QWidget* parent = 0);
  
  /// Destructor.
  ~PaintWidget();
  
  /// Paint the given image using OpenGL.
  /// \param data Image to render.
  void paintGL(const QImage & data);
  /// Handle resize events for the GL viewport.
  /// \param w New width in pixels.
  /// \param h New height in pixels.
  void resizeGL(int w, int h);
  
 protected:
  /// Current image data for rendering.
  QImage gldata;
};
