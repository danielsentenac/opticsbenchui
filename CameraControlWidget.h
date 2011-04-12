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
#include <QGridLayout>
#include <QLabel>
#include "QsLog.h"

class CameraControlWidget : public QWidget
{
  Q_OBJECT
    
    public:
  CameraControlWidget(Camera *_camera = 0);
  ~CameraControlWidget();

 
  
 signals:
  void setFeature(char* feature,int value);
  
  private slots:
  void setFeatureBright(int);
  void setFeatureGamma(int);
  void setFeatureGain(int);
  void setFeatureExposure(int);
  void updateFeatures();
 private:
  Camera *camera;

  QSlider *brightWidget;
  QSlider *gammaWidget;
  QSlider *gainWidget;
  QSlider *exposureWidget;
};
#endif
