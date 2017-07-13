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

#include "PaintWidget.h"


PaintWidget::PaintWidget(QWidget* parent) : 
QGLWidget(parent)
{
}
PaintWidget::~PaintWidget()
{
}
void PaintWidget::paintGL(const QImage & data)
{
  gldata = QGLWidget::convertToGLFormat(data);
  resize(data.size());
  glDrawPixels(data.width(), data.height(), GL_RGBA, GL_UNSIGNED_BYTE, gldata.bits());
}
void PaintWidget::resizeGL(int w, int h)
{
 glViewport (0, 0, w, h);
 glMatrixMode (GL_PROJECTION);     
 glLoadIdentity();
 glOrtho(0, w,0,h,-1,1);
 glMatrixMode (GL_MODELVIEW);
}
