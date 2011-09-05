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

  connect(this,SIGNAL(setFeature(int,int)),camera,SLOT(setFeature(int,int)));
  connect(camera,SIGNAL(updateFeatures()),this,SLOT(updateFeatures()));

  signalMapper = new QSignalMapper(this);

  QGridLayout *layout = new QGridLayout(this);
  
  for (int i = 0 ; i < DC1394_FEATURE_NUM; i++) {
    if (camera->features.feature[i].available == DC1394_TRUE && 
	camera->features.feature[i].id != DC1394_FEATURE_TRIGGER) {
      
      QLabel *featureName = new QLabel(iidc_features[i]);
      QSlider *featureSlider = new QSlider();
      featureSlider->setMinimum(camera->features.feature[i].min);
      featureSlider->setMaximum(camera->features.feature[i].max);
      featureList.push_back(featureSlider);
      featureId.push_back(i);
      connect( featureSlider, SIGNAL(valueChanged(int)), signalMapper, SLOT(map()));
      signalMapper->setMapping(featureSlider, featureList.size() - 1 );
      
      layout->addWidget(featureName,0,i,1,1);
      layout->addWidget(featureSlider,1,i,1,1);
      QLOG_DEBUG() << " Feature slider added : " <<  iidc_features[i];
    }  
  }
  connect(signalMapper, SIGNAL(mapped(int)),this, SLOT(setFeatureValue(int)));

  setLayout(layout);
  this->setMinimumHeight(130);

}
CameraControlWidget::~CameraControlWidget()
{
}
void
CameraControlWidget::setFeatureValue(int position) {
  
  QSlider *featureSlider = featureList.at(position);
  QLOG_DEBUG() << " Feature emitting !" ;
  emit setFeature(featureId.at(position),featureSlider->value());
}

void CameraControlWidget::updateFeatures() {
 
  
  for (int i = 0 ; i < featureList.size(); i++) {
    QSlider * featureSlider = featureList.at(i);
    int id = featureId.at(i);
    featureSlider->setValue(camera->features.feature[id].value);
    QLOG_DEBUG ( ) << "CameraControlWidget::update features " 
		   << iidc_features[id] << " : "
		   << camera->features.feature[id].value;
  }
}
