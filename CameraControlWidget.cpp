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
  
  for (int i = 0 ; i < camera->featureNameList.size(); i++) {
         
    QLabel *featureName = new QLabel(camera->featureNameList.at(i));
    QSlider *featureSlider = new QSlider();
    QLCDNumber *slidervalue = new  QLCDNumber();
    QLabel *absValue = new QLabel("-");
    slidervalue->setSegmentStyle(QLCDNumber::Flat);
    featureSlider->setMinimum(camera->featureMinList.at(i));
    featureSlider->setMaximum(camera->featureMaxList.at(i));
    connect( featureSlider, SIGNAL( valueChanged( int ) ), slidervalue, SLOT( display( int ) ) );
    featureSliderList.push_back(featureSlider);
    absValueLabelList.push_back(absValue);

    QCheckBox *modeCheck = new QCheckBox();
    modeCheckBoxList.push_back(modeCheck);
    connect( featureSlider, SIGNAL(valueChanged(int)), featureMapper, SLOT(map()));
    connect( modeCheck, SIGNAL(stateChanged(int)), modeMapper, SLOT(map()));
    
    featureMapper->setMapping(featureSlider, featureSliderList.size() - 1 );
    modeMapper->setMapping(modeCheck, modeCheckBoxList.size() - 1 );
    
    layout->addWidget(featureName,0,i,1,1);
    layout->addWidget(featureSlider,1,i,1,1);
    layout->addWidget(slidervalue,2,i,1,1);
    layout->addWidget(modeCheck,3,i,1,1);
    layout->addWidget(absValue,4,i,1,1);
    if (camera->featureModeAutoList.at(i) == true) {
      featureSlider->setBackgroundRole(QPalette::Foreground);
      featureSlider->setDisabled(true);
      modeCheck->setChecked(false);
    }
    else
      modeCheck->setChecked(true);
    QLOG_DEBUG() << " Feature slider added : " <<  camera->featureNameList.at(i);
    
  }
  connect(featureMapper, SIGNAL(mapped(int)),this, SLOT(setFeatureValue(int)));
  connect(modeMapper, SIGNAL(mapped(int)),this, SLOT(setModeValue(int)));

  QLabel *minLabel = new QLabel("min:");
  QLCDNumber *minValue = new  QLCDNumber();
  minValue->setSegmentStyle(QLCDNumber::Flat);
  layout->addWidget(minLabel,0,1000,1,1,Qt::AlignCenter);
  layout->addWidget(minValue,0,1000+1,1,1,Qt::AlignCenter);
  connect(camera,SIGNAL(updateMin(int)),minValue,SLOT(display(int)));
   
  QLabel *maxLabel = new QLabel("max:");
  QLCDNumber *maxValue = new  QLCDNumber();
  maxValue->setSegmentStyle(QLCDNumber::Flat);
  layout->addWidget(maxLabel,1,1000,1,1,Qt::AlignTop | Qt::AlignCenter);
  layout->addWidget(maxValue,1,1000+1,1,1,Qt::AlignTop | Qt::AlignCenter);
  connect(camera,SIGNAL(updateMax(int)),maxValue,SLOT(display(int)));

  snapshotButton = new QPushButton("Snapshot",this);
  snapshotButton->setFixedHeight(30);
  snapshotButton->setFixedWidth(80);
  QObject::connect(snapshotButton, SIGNAL(clicked()), this, SLOT(snapShot()));
  layout->addWidget(snapshotButton,2,1000,1,2,Qt::AlignJustify);
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
  
  QSlider *featureSlider = featureSliderList.at(position);
  QLOG_DEBUG() << " Feature emitting !" ;
  emit setFeature(camera->featureIdList.at(position),featureSlider->value());
  camera->getFeatures();
}
void
CameraControlWidget::setModeValue(int position) {
  
  QCheckBox *modeCheck = modeCheckBoxList.at(position);
  QSlider *featureSlider = featureSliderList.at(position);
  if (modeCheck->isEnabled() == true) {
    if (modeCheck->isChecked() == true) {
      QLOG_DEBUG() << " Mode change to true emitting !" ;
      emit setMode(camera->featureIdList.at(position),modeCheck->isChecked());
      featureSlider->setDisabled(false);
      featureSlider->setBackgroundRole(QPalette::Window);
    }
    else {
      QLOG_DEBUG() << " Mode change to false emitting !" ;
      emit setMode(camera->featureIdList.at(position),modeCheck->isChecked());
      featureSlider->setDisabled(true);
      featureSlider->setBackgroundRole(QPalette::Foreground);
    }
  }
  camera->getFeatures();
}
void CameraControlWidget::updateFeatures() {
  for (int i = 0 ; i < featureSliderList.size(); i++) {
    QSlider * featureSlider = featureSliderList.at(i);
    featureSlider->setValue(camera->featureValueList.at(i));
    QLOG_DEBUG ( ) << "CameraControlWidget::update features " 
		   << camera->featureNameList.at(i) << " : "
		   << camera->featureValueList.at(i);
    QLabel *absValue = absValueLabelList.at(i);
    if ( camera->featureAbsCapableList.at(i) )
      absValue->setNum(camera->featureAbsValueList.at(i));
  }
}

