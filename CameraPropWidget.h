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

/**
 * @file CameraPropWidget.h
 * @brief Camera backend for Prop Widget.
 */

#ifndef CAMERAPROPWIDGET_H
#define CAMERAPROPWIDGET_H

#include "Camera.h"

#include <QSlider>
#include <QGridLayout>
#include <QLabel>
#include <QSignalMapper>
#include <QFileDialog>
#include <QPushButton>
#include <QCheckBox>
#include <QLCDNumber>
#include "QsLog.h"

/// \ingroup ui_camera
/// UI widget for displaying camera properties.
class CameraPropWidget : public QWidget
{
  Q_OBJECT
    
    public:
  /// Construct a property widget for a camera.
  /// \param _camera Camera to inspect.
  CameraPropWidget(Camera *_camera = 0);
  /// Destructor.
  ~CameraPropWidget();

 
  
 signals:
  /// Emit warnings to the UI.
  /// \param message Warning message.
  void showWarning(QString message);

  private slots:
  /// Refresh the property display.
  void updateProps();

 private:
  Camera *camera;
  QVector<QLabel*> propList;
  QPushButton *refreshButton;
};
#endif
