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

#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include "Camera.h"

#include <QSlider>
#include <QSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QSignalMapper>
#include <QFileDialog>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QLCDNumber>
#include "QsLog.h"
#include "hdf5.h"
#include "hdf5_hl.h"

class CameraControlWidget : public QWidget
{
  Q_OBJECT
    
    public:
  CameraControlWidget(Camera *_camera = 0);
  ~CameraControlWidget();

 
  
 signals:
  void setFeature(int position, double value);
  void setMode(int position, bool value);
  void showWarning(QString message);

  private slots:
  void setFeatureValue(int);
  void setSliderValue(int);
  void setModeValue(int);
  void updateFeatures();
  void snapShot();

 private:
  Camera *camera;
  QVector<QSlider*> featureSliderList;
  QVector<QLabel*> valueMaxList, valueMinList;
  QVector<QLineEdit*> sliderValueList; 
  QVector<QCheckBox*> modeCheckBoxList;
  QPushButton *snapshotButton;
  QSignalMapper *featureMapper, *sliderValueMapper, *modeMapper;
};
#endif
