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

#define DOCK_HEIGHT 130
#define EXTRACTLOWPACKED(SourcePtr) ( (SourcePtr[0] << 4) + (SourcePtr[1] & 0xF) )
#define EXTRACTHIGHPACKED(SourcePtr) ( (SourcePtr[2] << 4) + (SourcePtr[1] >> 4) )

#define NUM_DIGITS 2
#define SLIDER_FACTOR 100.0

CameraControlWidget::CameraControlWidget(Camera *_camera)
 
{
  
  camera = _camera;
  
  connect(this,SIGNAL(setFeature(int,double)),camera,SLOT(setFeature(int,double)));
  connect(this,SIGNAL(setMode(int,bool)),camera,SLOT(setMode(int,bool)));
  connect(camera,SIGNAL(updateFeatures()),this,SLOT(updateFeatures()));

  featureMapper = new QSignalMapper(this);
  sliderValueMapper = new QSignalMapper(this);
  modeMapper = new QSignalMapper(this);

  QGridLayout *layout = new QGridLayout(this);
  
  int col = 0;
  for (int i = 0 ; i < camera->featureNameList.size(); i++) {
    QLabel *featureName = new QLabel(camera->featureNameList.at(i));
    QSlider *featureSlider = new QSlider();
    QLineEdit *sliderValue = new  QLineEdit();
    sliderValue->setFixedSize(QSize(60, 20));
    featureSlider->setTickPosition(QSlider::TicksLeft);
    QLOG_INFO() << "CameraControlWidget::CameraControlWidget> SLIDER_FACTOR " << QString::number(SLIDER_FACTOR);
    QLOG_INFO() << "CameraControlWidget::CameraControlWidget> NUM_DIGITS " << NUM_DIGITS;
    if ( camera->featureAbsCapableList.at(i) ) {
      featureSlider->setMinimum((int) (camera->featureMinList.at(i) * SLIDER_FACTOR ));
      featureSlider->setMaximum((int)(camera->featureMaxList.at(i) * SLIDER_FACTOR ));
      featureSlider->setValue((int)(camera->featureValueList.at(i) * SLIDER_FACTOR ));
      sliderValue->setText(QString::number(camera->featureValueList.at(i),'f',NUM_DIGITS));
    }
    else {
      featureSlider->setMinimum((int) (camera->featureMinList.at(i) ));
      featureSlider->setMaximum((int)(camera->featureMaxList.at(i) ));
      featureSlider->setValue((int)(camera->featureValueList.at(i) ));
      sliderValue->setText(QString::number((int)camera->featureValueList.at(i)));
    }
    
    QLabel *valueMin = new QLabel(QString::number(camera->featureMinList.at(i)));
    QLabel *valueMax = new QLabel(QString::number(camera->featureMaxList.at(i)));
    featureSliderList.push_back(featureSlider);
    sliderValueList.push_back(sliderValue);
    valueMinList.push_back(valueMin);
    valueMaxList.push_back(valueMax);

    QCheckBox *modeCheck = new QCheckBox();
    modeCheckBoxList.push_back(modeCheck);
    connect( featureSlider, SIGNAL(valueChanged(int)), featureMapper, SLOT(map()));
    connect( sliderValue, SIGNAL(editingFinished()), sliderValueMapper, SLOT(map()));
    connect( modeCheck, SIGNAL(stateChanged(int)), modeMapper, SLOT(map()));
    
    featureMapper->setMapping(featureSlider, featureSliderList.size() - 1 );
    sliderValueMapper->setMapping(sliderValue, sliderValueList.size() - 1 );
    modeMapper->setMapping(modeCheck, modeCheckBoxList.size() - 1 );
    
    layout->addWidget(featureName,0,col,1,2);
    layout->addWidget(featureSlider,1,col,1,1);
    layout->addWidget(valueMax,1,col+1,1,1,Qt::AlignTop);
    layout->addWidget(valueMin,1,col+1,1,1,Qt::AlignBottom);
    layout->addWidget(sliderValue,2,col,1,2);
    if ( camera->modeCheckEnabled == true )
      layout->addWidget(modeCheck,3,col,1,2);
    if (camera->featureModeAutoList.at(i) == true) {
      featureSlider->setBackgroundRole(QPalette::Foreground);
      featureSlider->setDisabled(true);
      modeCheck->setChecked(false);
    }
    else
      modeCheck->setChecked(true);
    QLOG_DEBUG() << " Feature slider added : " <<  camera->featureNameList.at(i);
    col+=2;
  }
  connect(featureMapper, SIGNAL(mapped(int)),this, SLOT(setFeatureValue(int)));
  connect(sliderValueMapper, SIGNAL(mapped(int)),this, SLOT(setSliderValue(int)));
  connect(modeMapper, SIGNAL(mapped(int)),this, SLOT(setModeValue(int)));

  QLabel *minPixLabel = new QLabel("min:");
  QLCDNumber *minValue = new  QLCDNumber();
  minValue->setSegmentStyle(QLCDNumber::Flat);
  layout->addWidget(minPixLabel,0,1000,1,1,Qt::AlignCenter);
  layout->addWidget(minValue,0,1000+1,1,1,Qt::AlignCenter);
  connect(camera,SIGNAL(updateMin(int)),minValue,SLOT(display(int)));
   
  QLabel *maxPixLabel = new QLabel("max:");
  QLCDNumber *maxValue = new  QLCDNumber();
  maxValue->setSegmentStyle(QLCDNumber::Flat);
  layout->addWidget(maxPixLabel,1,1000,1,1,Qt::AlignTop | Qt::AlignCenter);
  layout->addWidget(maxValue,1,1000+1,1,1,Qt::AlignTop | Qt::AlignCenter);
  connect(camera,SIGNAL(updateMax(int)),maxValue,SLOT(display(int)));

  snapshotButton = new QPushButton("Snapshot",this);
  snapshotButton->setFixedHeight(30);
  snapshotButton->setFixedWidth(80);
  QObject::connect(snapshotButton, SIGNAL(clicked()), this, SLOT(snapShot()));
  layout->addWidget(snapshotButton,0,1000+2,1,1,Qt::AlignCenter);
  setMinimumHeight(DOCK_HEIGHT);
  setLayout(layout);
}
CameraControlWidget::~CameraControlWidget()
{
}

void
CameraControlWidget::snapShot() {
  
  uchar *img8 = NULL;
  int *img32 = NULL;
  
  // Take snapshot
  if ( camera->pixel_encoding == B8 )
     img8 = camera->getSnapshot();
  else if ( camera->pixel_encoding == B10 || 
            camera->pixel_encoding == B12 ||
            camera->pixel_encoding == B12P||
	    camera->pixel_encoding == B16 ) 
     img32 = camera->getSnapshot32();
  // Open File
  QString filename = QFileDialog::getOpenFileName(this, tr("Take snapshot in HDF5 format"),
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
  if ( camera->pixel_encoding == B8 ) {
    H5IMmake_image_8bit(file_id,"SNAPSHOT",
                         camera->width,
		         camera->height,
		         img8);
    H5LTset_attribute_int(file_id, "SNAPSHOT", "min", &camera->snapShotMin,1);
    H5LTset_attribute_int(file_id, "SNAPSHOT", "max", &camera->snapShotMax,1);
  }
  else if ( camera->pixel_encoding == B10 ||
            camera->pixel_encoding == B12 ||
            camera->pixel_encoding == B12P||
            camera->pixel_encoding == B16 ) {
     QLOG_INFO() << "CameraControlWidget::snapShot> Take snapshot pixel encoding " 
		 << camera->pixel_encoding
                 << " width " << camera->width
                 << " height " << camera->height;
     hsize_t dset_dims[2];
     dset_dims[0] = camera->height;
     dset_dims[1] = camera->width;
     H5LTmake_dataset_int(file_id,"SNAPSHOT32", 2,dset_dims,img32);
     H5LTset_attribute_int(file_id, "SNAPSHOT32", "min", &camera->snapShotMin,1);
     H5LTset_attribute_int(file_id, "SNAPSHOT32", "max", &camera->snapShotMax,1);
  }
  // Close file
  H5Fclose(file_id);
}
void
CameraControlWidget::setFeatureValue(int position) {
  
  QSlider *featureSlider = featureSliderList.at(position);
  QLOG_INFO() << "CameraControlWidget::setFeatureValue> SLIDER_FACTOR " << QString::number(SLIDER_FACTOR);
  QLOG_INFO() << "CameraControlWidget::setFeatureValue> NUM_DIGITS " << NUM_DIGITS;
  QLOG_INFO() << "CameraControlWidget::setFeatureValue> Request "
              << camera->featureNameList.at(position) << " value "
              << featureSlider->value();
  double value;
  if ( camera->featureAbsCapableList.at(position) )
    value = (double) (featureSlider->value()) / SLIDER_FACTOR; 
  else
    value = (double) (featureSlider->value());
  emit setFeature(camera->featureIdList.at(position),value);
  camera->getFeatures();
}
void
CameraControlWidget::setSliderValue(int position) {

 QLineEdit *sliderValue = sliderValueList.at(position);
 QString slidervalueStr = sliderValue->text();
 double value = slidervalueStr.toDouble();
 QSlider *featureSlider = featureSliderList.at(position);
 QLOG_INFO() << "CameraControlWidget::setSliderValue> SLIDER_FACTOR " << QString::number(SLIDER_FACTOR);
 QLOG_INFO() << "CameraControlWidget::setSliderValue> NUM_DIGITS " << NUM_DIGITS;
 QLOG_INFO() << "CameraControlWidget::setSliderValue> Request "
              << camera->featureNameList.at(position) << " value "
              << slidervalueStr << " To Slider " << QString::number(value * SLIDER_FACTOR)
              << " SLIDER_FACTOR " << QString::number(SLIDER_FACTOR);
 if ( camera->featureAbsCapableList.at(position) ) 
   featureSlider->setValue(value * SLIDER_FACTOR);
 else 
   featureSlider->setValue(value);
}

void
CameraControlWidget::setModeValue(int position) {
  
  QCheckBox *modeCheck = modeCheckBoxList.at(position);
  QSlider *featureSlider = featureSliderList.at(position);
  if (modeCheck->isEnabled() == true) {
    emit setMode(camera->featureIdList.at(position),modeCheck->isChecked());
    camera->getFeatures();
    if (modeCheck->isChecked() == true) {
      QLOG_DEBUG() << " Mode change to true emitting !" ;
      featureSlider->setDisabled(false);
      featureSlider->setBackgroundRole(QPalette::Window);
    }
    else {
      QLOG_DEBUG() << " Mode change to false emitting !" ;
      featureSlider->setDisabled(true);
      featureSlider->setBackgroundRole(QPalette::Foreground);
    }
  }
}
void CameraControlWidget::updateFeatures() {
  for (int i = 0 ; i < featureSliderList.size(); i++) {
    QSlider * featureSlider = featureSliderList.at(i);
    if ( camera->featureAbsCapableList.at(i) ) {
      featureSlider->setMinimum((int) (camera->featureMinList.at(i) * SLIDER_FACTOR ));
      featureSlider->setMaximum((int)(camera->featureMaxList.at(i) * SLIDER_FACTOR ));
      featureSlider->setValue((int)(camera->featureValueList.at(i) * SLIDER_FACTOR ));
    }
    else {
      featureSlider->setMinimum((int) (camera->featureMinList.at(i) ));
      featureSlider->setMaximum((int)(camera->featureMaxList.at(i) ));
      featureSlider->setValue((int)(camera->featureValueList.at(i) ));
    }
    valueMinList.at(i)->setText(QString::number(camera->featureMinList.at(i)));
    valueMaxList.at(i)->setText(QString::number(camera->featureMaxList.at(i)));
    QLOG_INFO ( ) << "CameraControlWidget::update features " 
		   << camera->featureNameList.at(i) << " : "
		   << camera->featureValueList.at(i);
    QLineEdit *sliderValue = sliderValueList.at(i);
    if ( camera->featureAbsCapableList.at(i) ) {
      QLOG_DEBUG() << " ABS CAPABLE " << camera->featureNameList.at(i);
      sliderValue->setText(QString::number(camera->featureAbsValueList.at(i),'f',NUM_DIGITS));
    }
    else {
      QLOG_DEBUG() << " NOT ABS CAPABLE " << camera->featureNameList.at(i);
      sliderValue->setText(QString::number((int)camera->featureValueList.at(i)));
   }
  }
}
