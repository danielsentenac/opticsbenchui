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
  connect(camera,SIGNAL(updateMinMax(int,int)),this,SLOT(updateMinMax(int,int)));

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

  minLabel = new QLabel("min:");
  layout->addWidget(minLabel,0,DC1394_FEATURE_NUM,1,1,Qt::AlignCenter);
  maxLabel = new QLabel("max:");
  layout->addWidget(maxLabel,1,DC1394_FEATURE_NUM,1,1,Qt::AlignTop | Qt::AlignCenter);
  snapshotButton = new QPushButton("Snapshot",this);
  snapshotButton->setFixedHeight(30);
  snapshotButton->setFixedWidth(80);
  QObject::connect(snapshotButton, SIGNAL(clicked()), this, SLOT(snapShot()));
  layout->addWidget(snapshotButton,1,DC1394_FEATURE_NUM,1,1,Qt::AlignCenter);
  setLayout(layout);
  this->setMinimumHeight(130);

}
CameraControlWidget::~CameraControlWidget()
{
}

void
CameraControlWidget::snapShot() {
  
  // Open File
  QString filename = QFileDialog::getSaveFileName(this, tr("Save File in HDF5 format"),
						  QDir::currentPath ()+ 
						  QDir::separator() + "untitled.h5",
						  tr("HDF5 (*.h5)"));
  hid_t file_id = H5Fcreate(filename.toStdString().c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  if ( file_id < 0 ) {
    QLOG_WARN () << "Unable to open file - " << filename ;
    emit showWarning("Unable to open file - " + filename );
    return;
  }
  // Save data to File
  H5IMmake_image_8bit(file_id,"SNAPSHOT",
		      camera->width,
		      camera->height,
		      camera->getSnapshot());
  H5LTset_attribute_int(file_id, "SNAPSHOT", "min", &camera->snapShotMin,1);
  H5LTset_attribute_int(file_id, "SNAPSHOT", "max", &camera->snapShotMax,1);
  // Close file
  H5Fclose(file_id);
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
void
CameraControlWidget::updateMinMax(int min, int max) {
  minLabel->setText("min:" + QString::number(min));
  maxLabel->setText("max:" + QString::number(max));
}
