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
 * @file CameraControlWidget.h
 * @brief Camera backend for Control Widget.
 */

#ifndef CAMERACONTROLWIDGET_H
#define CAMERACONTROLWIDGET_H

#include <QVector>
#include <QWidget>

class Camera;
class QSlider;
class QLabel;
class QLineEdit;
class QCheckBox;
class QPushButton;
class QButtonGroup;
class QSignalMapper;

/// \ingroup ui
/// UI widget for camera feature control.
class CameraControlWidget : public QWidget
{
  Q_OBJECT
    
public:
  /// Construct a control widget for a camera.
  /// \param _camera Camera to control.
  CameraControlWidget(Camera *_camera = nullptr);
  /// Destructor.
  ~CameraControlWidget();

signals:
  /// Set a camera feature value.
  /// \param position Feature index.
  /// \param value Feature value.
  void setFeature(int position, double value);
  /// Set a camera feature mode.
  /// \param position Feature index.
  /// \param value True for auto/enable, false for manual/disable.
  void setMode(int position, bool value);
  /// Emit warnings to the UI.
  /// \param message Warning message.
  void showWarning(QString message);

private slots:
  /// Update a feature from a slider.
  /// \param value New slider value.
  void setFeatureValue(int value);
  /// Update a slider from a text value.
  /// \param value New text value.
  void setSliderValue(int value);
  /// Update a feature mode from a checkbox.
  /// \param value Checkbox state.
  void setModeValue(int value);
  /// Refresh feature values from the camera.
  void updateFeatures();
  /// Trigger a single snapshot.
  void snapShot();
  /// Optimize acquisition settings.
  void optimizeAcquisition();

private:
  Camera *camera;
  QVector<QSlider*> featureSliderList;
  QVector<QLabel*> valueMaxList, valueMinList;
  QVector<QLineEdit*> sliderValueList; 
  QVector<QCheckBox*> modeCheckBoxList;
  QCheckBox* optimizeAcquisitionBox;
  QLabel  *optimizeAcquisitionLabel;
  QPushButton *snapshotButton;
  QLabel  *hflipLabel, *vflipLabel;
  QCheckBox *hflipBox, *vflipBox;
  QButtonGroup *colorGroup;  
  
  QSignalMapper *featureMapper, *sliderValueMapper, *modeMapper;
};
#endif
