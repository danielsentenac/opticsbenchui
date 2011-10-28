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
  connect(this,SIGNAL(setMode(int,bool)),camera,SLOT(setMode(int,bool)));
  connect(camera,SIGNAL(updateFeatures()),this,SLOT(updateFeatures()));
 
  featureMapper = new QSignalMapper(this);
  modeMapper = new QSignalMapper(this);

  QGridLayout *layout = new QGridLayout(this);
  
  for (int i = 0 ; i < DC1394_FEATURE_NUM; i++) {
    if (camera->features.feature[i].available == DC1394_TRUE && 
	camera->features.feature[i].id != DC1394_FEATURE_TRIGGER) {
      
      QLabel *featureName = new QLabel(iidc_features[i]);
      QSlider *featureSlider = new QSlider();
      QLCDNumber *slidervalue = new  QLCDNumber();
      QLabel *absValue = new QLabel("-");
      slidervalue->setSegmentStyle(QLCDNumber::Flat);
      featureSlider->setMinimum(camera->features.feature[i].min);
      featureSlider->setMaximum(camera->features.feature[i].max);
      connect( featureSlider, SIGNAL( valueChanged( int ) ), slidervalue, SLOT( display( int ) ) );
      featureList.push_back(featureSlider);
      absvalueList.push_back(absValue);
      featureId.push_back(i);
      QCheckBox *modeCheck = new QCheckBox();
      modeList.push_back(modeCheck);
      /* if (camera->features.feature[i].on_off_capable == DC1394_FALSE) {
	modeCheck->setDisabled(true);
	}*/
      connect( featureSlider, SIGNAL(valueChanged(int)), featureMapper, SLOT(map()));
      connect( modeCheck, SIGNAL(stateChanged(int)), modeMapper, SLOT(map()));

      featureMapper->setMapping(featureSlider, featureList.size() - 1 );
      modeMapper->setMapping(modeCheck, modeList.size() - 1 );

      layout->addWidget(featureName,0,i,1,1);
      layout->addWidget(featureSlider,1,i,1,1);
      layout->addWidget(slidervalue,2,i,1,1);
      layout->addWidget(modeCheck,3,i,1,1);
      layout->addWidget(absValue,4,i,1,1);
      if (camera->features.feature[i].current_mode == DC1394_FEATURE_MODE_AUTO) {
	featureSlider->setBackgroundRole(QPalette::Foreground);
	featureSlider->setDisabled(true);
	modeCheck->setChecked(false);
      }
      else
	modeCheck->setChecked(true);
      QLOG_DEBUG() << " Feature slider added : " <<  iidc_features[i];
    }  
  }
  connect(featureMapper, SIGNAL(mapped(int)),this, SLOT(setFeatureValue(int)));
  connect(modeMapper, SIGNAL(mapped(int)),this, SLOT(setModeValue(int)));

  QLabel *minLabel = new QLabel("min:");
  QLCDNumber *minValue = new  QLCDNumber();
  minValue->setSegmentStyle(QLCDNumber::Flat);
  layout->addWidget(minLabel,0,DC1394_FEATURE_NUM,1,1,Qt::AlignCenter);
  layout->addWidget(minValue,0,DC1394_FEATURE_NUM+1,1,1,Qt::AlignCenter);
  connect(camera,SIGNAL(updateMin(int)),minValue,SLOT(display(int)));
   
  QLabel *maxLabel = new QLabel("max:");
  QLCDNumber *maxValue = new  QLCDNumber();
  maxValue->setSegmentStyle(QLCDNumber::Flat);
  layout->addWidget(maxLabel,1,DC1394_FEATURE_NUM,1,1,Qt::AlignTop | Qt::AlignCenter);
  layout->addWidget(maxValue,1,DC1394_FEATURE_NUM+1,1,1,Qt::AlignTop | Qt::AlignCenter);
  connect(camera,SIGNAL(updateMax(int)),maxValue,SLOT(display(int)));

  snapshotButton = new QPushButton("Snapshot",this);
  snapshotButton->setFixedHeight(30);
  snapshotButton->setFixedWidth(80);
  QObject::connect(snapshotButton, SIGNAL(clicked()), this, SLOT(snapShot()));
  layout->addWidget(snapshotButton,2,DC1394_FEATURE_NUM,1,2,Qt::AlignJustify);
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
  if (filename == "") return;
  hid_t file_id = H5Fcreate(filename.toStdString().c_str(), 
			    H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
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
  camera->getFeatures();
}
void
CameraControlWidget::setModeValue(int position) {
  
  QCheckBox *modeCheck = modeList.at(position);
  QSlider *featureSlider = featureList.at(position);
  if (modeCheck->isEnabled() == true) {
    if (modeCheck->isChecked() == true) {
      QLOG_DEBUG() << " Mode change to true emitting !" ;
      emit setMode(featureId.at(position),modeCheck->isChecked());
      featureSlider->setDisabled(false);
      featureSlider->setBackgroundRole(QPalette::Window);
    }
    else {
      QLOG_DEBUG() << " Mode change to false emitting !" ;
      emit setMode(featureId.at(position),modeCheck->isChecked());
      featureSlider->setDisabled(true);
      featureSlider->setBackgroundRole(QPalette::Foreground);
    }
  }
  camera->getFeatures();
}
void CameraControlWidget::updateFeatures() {
 
  
  for (int i = 0 ; i < featureList.size(); i++) {
    QSlider * featureSlider = featureList.at(i);
    int id = featureId.at(i);
    featureSlider->setValue(camera->features.feature[id].value);
    QLOG_DEBUG ( ) << "CameraControlWidget::update features " 
		   << iidc_features[id] << " : "
		   << camera->features.feature[id].value;
    QLabel *absValue = absvalueList.at(i);
    float value;
    if ( camera->features.feature[id].absolute_capable )
      absValue->setNum(camera->features.feature[id].abs_value);
  }
}

