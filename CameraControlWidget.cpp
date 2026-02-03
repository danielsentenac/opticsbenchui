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

#include <QButtonGroup>
#include <QCheckBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLCDNumber>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalMapper>
#include <QSlider>

#include "Camera.h"
#include "QsLog.h"
#include "Utils.h"
#ifndef NO_HDF5
#include "hdf5.h"
#include "hdf5_hl.h"
#endif

#define DOCK_HEIGHT 130
#define EXTRACTLOWPACKED(SourcePtr) ( (SourcePtr[0] << 4) + (SourcePtr[1] & 0xF) )
#define EXTRACTHIGHPACKED(SourcePtr) ( (SourcePtr[2] << 4) + (SourcePtr[1] >> 4) )

#define NUM_DIGITS 2
#define SLIDER_FACTOR 1000

static bool IsAutoEnumFeature(const Camera *camera, int index) {
  if (camera == nullptr) {
    return false;
  }
  const QString name = camera->featureNameList.at(index);
  return name == "ExposureAuto" || name == "GainAuto";
}

static QString AutoEnumToText(int value) {
  switch (value) {
    case 1:
      return "Once";
    case 2:
      return "Continuous";
    default:
      return "Off";
  }
}

static int AutoEnumFromText(const QString &text, bool *ok) {
  const QString trimmed = text.trimmed();
  const QString lower = trimmed.toLower();
  if (lower == "off") {
    if (ok) {
      *ok = true;
    }
    return 0;
  }
  if (lower == "once") {
    if (ok) {
      *ok = true;
    }
    return 1;
  }
  if (lower == "continuous" || lower == "on") {
    if (ok) {
      *ok = true;
    }
    return 2;
  }
  bool localOk = false;
  const int numeric = trimmed.toInt(&localOk);
  if (ok) {
    *ok = localOk;
  }
  return numeric;
}

CameraControlWidget::CameraControlWidget(Camera *_camera)
 
{
  QLOG_INFO() << "CameraControlWidget::CameraControlWidget> SLIDER_FACTOR " << QString::number(SLIDER_FACTOR);
  QLOG_INFO() << "CameraControlWidget::CameraControlWidget> NUM_DIGITS " << NUM_DIGITS;
  camera = _camera;
  
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
    if ( camera->featureAbsCapableList.at(i) ) {
      featureSlider->setMinimum((int) (camera->featureMinList.at(i) * SLIDER_FACTOR ));
      featureSlider->setMaximum((int)(camera->featureMaxList.at(i) * SLIDER_FACTOR ));
      featureSlider->setValue((int)(camera->featureValueList.at(i) * SLIDER_FACTOR ));
      sliderValue->setText(QString::number(camera->featureValueList.at(i),'f',NUM_DIGITS));
    }
    else {
      QLOG_INFO() <<  "CameraControlWidget::CameraControlWidget::> " << camera->featureNameList.at(i)
                << " value=" << camera->featureValueList.at(i)
                << " min=" << camera->featureMinList.at(i)
                << " max=" << camera->featureMaxList.at(i);
      featureSlider->setMinimum((int) (camera->featureMinList.at(i) ));
      featureSlider->setMaximum((int)(camera->featureMaxList.at(i) ));
      featureSlider->setValue((int)(camera->featureValueList.at(i) ));
      if (IsAutoEnumFeature(camera, i)) {
        sliderValue->setText(AutoEnumToText((int)camera->featureValueList.at(i)));
      } else {
        sliderValue->setText(QString::number((int)camera->featureValueList.at(i)));
      }
    }
    
    QLabel *valueMin = new QLabel(QString::number(camera->featureMinList.at(i)));
    QLabel *valueMax = new QLabel(QString::number(camera->featureMaxList.at(i)));
    if (IsAutoEnumFeature(camera, i)) {
      valueMin->setText("Off");
      valueMax->setText("Continuous");
    }
    featureSliderList.push_back(featureSlider);
    sliderValueList.push_back(sliderValue);
    valueMinList.push_back(valueMin);
    valueMaxList.push_back(valueMax);

    QCheckBox *modeCheck = new QCheckBox();
    modeCheckBoxList.push_back(modeCheck);
    
    connect(this,SIGNAL(setFeature(int,double)),camera,SLOT(setFeature(int,double)),Qt::UniqueConnection);
    connect(this,SIGNAL(setMode(int,bool)),camera,SLOT(setMode(int,bool)),Qt::UniqueConnection);
    connect(camera,SIGNAL(updateFeatures()),this,SLOT(updateFeatures()),Qt::UniqueConnection);

    connect( featureSlider, SIGNAL(valueChanged(int)), featureMapper, SLOT(map()),Qt::UniqueConnection);
    connect( sliderValue, SIGNAL(returnPressed()), sliderValueMapper, SLOT(map()),Qt::UniqueConnection);
    connect( modeCheck, SIGNAL(stateChanged(int)), modeMapper, SLOT(map()),Qt::UniqueConnection);
    
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
      featureSlider->setBackgroundRole(QPalette::WindowText);
#else
      featureSlider->setBackgroundRole(QPalette::Foreground);
#endif
      featureSlider->setDisabled(true);
      modeCheck->setChecked(false);
    }
    else
      modeCheck->setChecked(true);
    QLOG_DEBUG() << " Feature slider added : " <<  camera->featureNameList.at(i);
    col+=2;
  }
  connect(featureMapper, SIGNAL(mapped(int)),this, SLOT(setFeatureValue(int)),Qt::UniqueConnection);
  connect(sliderValueMapper, SIGNAL(mapped(int)),this, SLOT(setSliderValue(int)),Qt::UniqueConnection);
  connect(modeMapper, SIGNAL(mapped(int)),this, SLOT(setModeValue(int)),Qt::UniqueConnection);

  QLabel *minPixLabel = new QLabel("min:");
  QLCDNumber *minValue = new  QLCDNumber();
  minValue->setSegmentStyle(QLCDNumber::Flat);
  layout->addWidget(minPixLabel,0,1000,1,1,Qt::AlignCenter);
  layout->addWidget(minValue,0,1000+1,1,1,Qt::AlignCenter);
  connect(camera,SIGNAL(updateMin(int)),minValue,SLOT(display(int)),Qt::UniqueConnection);
   
  QLabel *maxPixLabel = new QLabel("max:");
  QLCDNumber *maxValue = new  QLCDNumber();
  maxValue->setSegmentStyle(QLCDNumber::Flat);
  layout->addWidget(maxPixLabel,1,1000,1,1,Qt::AlignTop | Qt::AlignCenter);
  layout->addWidget(maxValue,1,1000+1,1,1,Qt::AlignTop | Qt::AlignCenter);
  connect(camera,SIGNAL(updateMax(int)),maxValue,SLOT(display(int)),Qt::UniqueConnection);

  QLabel *avgPixLabel = new QLabel("avg:");
  QLCDNumber *avgValue = new  QLCDNumber();
  avgValue->setSegmentStyle(QLCDNumber::Flat);
  layout->addWidget(avgPixLabel,1,1000,1,1,Qt::AlignBottom | Qt::AlignCenter);
  layout->addWidget(avgValue,1,1000+1,1,1,Qt::AlignBottom | Qt::AlignCenter);
  connect(camera,SIGNAL(updateAvg(int)),avgValue,SLOT(display(int)),Qt::UniqueConnection);

 /* optimizeAcquisitionLabel = new QLabel("Optimize:");
  optimizeAcquisitionBox = new QCheckBox(); 
  connect( optimizeAcquisitionBox, SIGNAL(stateChanged(int)), this, SLOT(optimizeAcquisition()),Qt::UniqueConnection);
  layout->addWidget(optimizeAcquisitionLabel,2,1000,1,1,Qt::AlignTop | Qt::AlignCenter);
  layout->addWidget(optimizeAcquisitionBox,2,1000+1,1,1,Qt::AlignTop | Qt::AlignCenter);
*/
  snapshotButton = new QPushButton("Snapshot",this);
  snapshotButton->setFixedHeight(30);
  snapshotButton->setFixedWidth(80);
  QObject::connect(snapshotButton, SIGNAL(clicked()), this, SLOT(snapShot()),Qt::UniqueConnection);
  layout->addWidget(snapshotButton,0,1000+2,1,1,Qt::AlignCenter);
  
  vflipLabel = new QLabel("Flip vertical");
  vflipBox = new QCheckBox();
  QObject::connect(vflipBox, SIGNAL(stateChanged(int)), camera, SLOT(vflipImage(int)),Qt::UniqueConnection);
  layout->addWidget(vflipLabel,1,1000+2,1,1,Qt::AlignBottom);
  layout->addWidget(vflipBox,1,1000+3,1,1,Qt::AlignBottom);

  hflipLabel = new QLabel("Flip horizontal");
  hflipBox = new QCheckBox();
  QObject::connect(hflipBox, SIGNAL(stateChanged(int)), camera, SLOT(hflipImage(int)),Qt::UniqueConnection);
  layout->addWidget(hflipLabel,2,1000+2,1,1,Qt::AlignCenter);
  layout->addWidget(hflipBox,2,1000+3,1,1,Qt::AlignCenter);

  colorGroup = new QButtonGroup(this);
  connect(colorGroup, SIGNAL(buttonClicked(int)), camera, SLOT(setColorTable(int)),Qt::UniqueConnection) ;
  QRadioButton *grayButton = new QRadioButton("gray");
  QRadioButton *hotButton =  new QRadioButton(" hot ");
  colorGroup->addButton(grayButton);
  colorGroup->addButton(hotButton);
  layout->addWidget(grayButton,1,2000,1,1,Qt::AlignBottom);
  layout->addWidget(hotButton,2,2000,1,1,Qt::AlignCenter);
  
  grayButton->setChecked(true);
  setMinimumHeight(DOCK_HEIGHT);
  setLayout(layout);
  //this->setMinimumHeight(150);
}
CameraControlWidget::~CameraControlWidget()
{
}

void
CameraControlWidget::snapShot() {
#ifdef NO_HDF5
  Utils::EmitWarning(this, __FUNCTION__,
                     "HDF5 support is disabled. Snapshot in HDF5 format is unavailable.");
  return;
#else
  
  uchar *img8 = nullptr;
  int *img32 = nullptr;
  
  // Take snapshot
  if ( camera->pixel_encoding == B8 ) {
     QLOG_INFO() << " Take SNAPSHOT...";
     img8 = camera->getSnapshot();
  }
  else  
     img32 = camera->getSnapshot32();
  // Save File
  QString filename = QFileDialog::getSaveFileName(
      this, tr("Take snapshot in HDF5 format"),
      Utils::DefaultHdf5Path("untitled"),
      Utils::Hdf5FileDialogFilter());
  if (filename == "") return;
  filename = Utils::EnsureHdf5Extension(filename);
  hid_t file_id = H5Fcreate(filename.toStdString().c_str(), 
			    H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  if ( file_id < 0 ) {
    QLOG_WARN () << "Unable to open file - " << filename ;
    Utils::EmitWarning(this, __FUNCTION__, "Unable to open file - " + filename );
    return;
  }
  // Save data to File
  if ( camera->pixel_encoding == B8 ) {
    QLOG_INFO () << "Saving image to file - " << filename ;
    QLOG_INFO () << "width - " << camera->width ;
    QLOG_INFO () << "height - " << camera->height ;
    QLOG_INFO () << "min - " << camera->snapShotMin ;
    QLOG_INFO () << "max - " << camera->snapShotMax ;
 
    H5IMmake_image_8bit(file_id,"SNAPSHOT",
                         camera->width,
                         camera->height,
		         img8);
    
    H5LTset_attribute_int(file_id, "SNAPSHOT", "min", &camera->snapShotMin,1);
    H5LTset_attribute_int(file_id, "SNAPSHOT", "max", &camera->snapShotMax,1);
  }
  else  {
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
#endif
}
void 
CameraControlWidget::optimizeAcquisition() {
  if (camera->optimizeAcquisition == false)
    camera->optimizeAcquisition = true;
  else
    camera->optimizeAcquisition = false;
}
void
CameraControlWidget::setFeatureValue(int position) {
  
  QSlider *featureSlider = featureSliderList.at(position);
  QLOG_INFO() << "CameraControlWidget::setFeatureValue> Request "
              << camera->featureNameList.at(position) << " value "
              << featureSlider->value();
  double value;
  if ( camera->featureAbsCapableList.at(position) )
    value = (double) (featureSlider->value()) / SLIDER_FACTOR; 
  else
    value = (double) (featureSlider->value());
  QLOG_INFO() << "CameraControlWidget::setFeatureValue> Request "
              << camera->featureNameList.at(position) << " value "
              << QString::number(value);

  emit setFeature(camera->featureIdList.at(position),value);
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
 else {
   if (IsAutoEnumFeature(camera, position)) {
     bool ok = false;
     const int enumValue = AutoEnumFromText(slidervalueStr, &ok);
     if (ok) {
       featureSlider->setValue(enumValue);
     }
   } else {
     featureSlider->setValue(value);
   }
 }
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
      featureSlider->setBackgroundRole(QPalette::WindowText);
#else
      featureSlider->setBackgroundRole(QPalette::Foreground);
#endif
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
      QLOG_INFO() <<  "CameraControlWidget::updateFeatures> SLIDER " << camera->featureNameList.at(i)
		<< " value=" << camera->featureValueList.at(i) 
                << " min=" << camera->featureMinList.at(i) * SLIDER_FACTOR
		<< " max=" << camera->featureMaxList.at(i) * SLIDER_FACTOR;
    }
    else {
      featureSlider->setMinimum((int) (camera->featureMinList.at(i) ));
      featureSlider->setMaximum((int)(camera->featureMaxList.at(i) ));
      featureSlider->setValue((int)(camera->featureValueList.at(i) ));
      QLOG_DEBUG() <<  "CameraControlWidget::updateFeatures> SLIDER " << camera->featureNameList.at(i)
		<< " value=" << camera->featureValueList.at(i)
                << " min=" << camera->featureMinList.at(i)
		<< " max=" << camera->featureMaxList.at(i);
    }
    valueMinList.at(i)->setText(QString::number(camera->featureMinList.at(i)));
    valueMaxList.at(i)->setText(QString::number(camera->featureMaxList.at(i)));
    QLineEdit *sliderValue = sliderValueList.at(i);
    if ( camera->featureAbsCapableList.at(i) ) {
      QLOG_INFO() << " ABS CAPABLE " << camera->featureNameList.at(i);
      sliderValue->setText(QString::number(camera->featureAbsValueList.at(i),'f',NUM_DIGITS));
    }
    else {
      QLOG_INFO() << "CameraControlWidget::updateFeatures> UPDATE FEATURE " << i <<  " " <<  camera->featureNameList.at(i) << " value : " << (int)camera->featureValueList.at(i);
      if (IsAutoEnumFeature(camera, i)) {
        sliderValue->setText(AutoEnumToText((int)camera->featureValueList.at(i)));
        valueMinList.at(i)->setText("Off");
        valueMaxList.at(i)->setText("Continuous");
      } else {
        sliderValue->setText(QString::number((int)camera->featureValueList.at(i)));
      }
   }
  }
}
