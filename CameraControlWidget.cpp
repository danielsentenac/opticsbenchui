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

#include "CameraControlWidget.h"

CameraControlWidget::CameraControlWidget(Camera *_camera)
 
{
  
  camera = _camera;

  connect(this,SIGNAL(setFeature(char*,int)),camera,SLOT(setFeature(char*,int)));
  connect(camera,SIGNAL(updateFeatures()),this,SLOT(updateFeatures()));

  QGridLayout *layout = new QGridLayout(this);
  
  QLabel *brightName = new QLabel("Brightness");
  brightWidget = new QSlider();
  brightWidget->setMinimum(camera->features.feature[0].min);
  brightWidget->setMaximum(camera->features.feature[0].max);
  
  connect( brightWidget, SIGNAL(valueChanged(int)), this, SLOT(setFeatureBright(int)));
  layout->addWidget(brightName,0,0,1,1);
  layout->addWidget(brightWidget,1,0,1,1);
  
  QLabel *gammaName = new QLabel("Gamma");
  gammaWidget = new QSlider();
  gammaWidget->setMinimum(camera->features.feature[6].min);
  gammaWidget->setMaximum(camera->features.feature[6].max);

  connect( gammaWidget, SIGNAL(valueChanged(int)), this, SLOT(setFeatureGamma(int)));
  layout->addWidget(gammaName,0,1,1,1);
  layout->addWidget(gammaWidget,1,1,1,1);

  QLabel *gainName = new QLabel("Gain");
  gainWidget = new QSlider();
  gainWidget->setMinimum(camera->features.feature[8].min);
  gainWidget->setMaximum(camera->features.feature[8].max);
  
  connect( gainWidget, SIGNAL(valueChanged(int)), this, SLOT(setFeatureGain(int)));
  layout->addWidget(gainName,0,2,1,1);
  layout->addWidget(gainWidget,1,2,1,1);

  QLabel *exposureName = new QLabel("Exposure");
  exposureWidget = new QSlider();
  exposureWidget->setMinimum(camera->features.feature[1].min);
  exposureWidget->setMaximum(camera->features.feature[1].max);
  
  connect( exposureWidget, SIGNAL(valueChanged(int)), this, SLOT(setFeatureExposure(int)));
  layout->addWidget(exposureName,0,3,1,1);
  layout->addWidget(exposureWidget,1,3,1,1);
  
  setLayout(layout);
  this->setMinimumHeight(130);

}
CameraControlWidget::~CameraControlWidget()
{
}
void
CameraControlWidget::setFeatureBright(int value) {
  emit setFeature((char*)"BRIGHTNESS",value);
}
void
CameraControlWidget::setFeatureGamma(int value) {
  emit setFeature((char*)"GAMMA",value);
}
void
CameraControlWidget::setFeatureGain(int value) {
  emit setFeature((char*)"GAIN",value);
}
void
CameraControlWidget::setFeatureExposure(int value) {
  emit setFeature((char*)"EXPOSURE",value);
}
void CameraControlWidget::updateFeatures() {
  QLOG_DEBUG ( ) << "CameraControlWidget::update features " 
		<< camera->features.feature[0].value << " "
		<< camera->features.feature[6].value << " "
		<< camera->features.feature[8].value << " "
		<< camera->features.feature[1].value;
  brightWidget->setValue(camera->features.feature[0].value);
  gammaWidget->setValue(camera->features.feature[6].value);
  gainWidget->setValue(camera->features.feature[8].value);
  exposureWidget->setValue(camera->features.feature[1].value);
  
}
