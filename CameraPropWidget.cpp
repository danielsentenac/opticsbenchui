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

#include "CameraPropWidget.h"

#define DOCK_WIDTH 220

CameraPropWidget::CameraPropWidget(Camera *_camera)
 
{
  camera = _camera;
  if (!camera) {
    QLOG_WARN() << "CameraPropWidget::CameraPropWidget> camera is null";
    return;
  }

  connect(camera,SIGNAL(updateProps()),this,SLOT(updateProps()));

  QGridLayout *layout = new QGridLayout(this);
  
  refreshButton = new QPushButton(QStringLiteral("Refresh"),this);
  refreshButton->setFixedHeight(30);
  refreshButton->setFixedWidth(80);
  connect(refreshButton, SIGNAL(clicked()), camera, SLOT(getProps()));
  layout->addWidget(refreshButton,0,0,1,1,Qt::AlignJustify);

  for (int i = 0 ; i < camera->propList.size(); i++) {
    QLabel *propLabel = new QLabel();
    propLabel->setText(camera->propList.at(i));
    propList.push_back(propLabel);
    layout->addWidget(propLabel,i+1,0,1,1,Qt::AlignLeft);
  }
  setMinimumWidth(DOCK_WIDTH);
  setLayout(layout);
}
CameraPropWidget::~CameraPropWidget()
{
}

void CameraPropWidget::updateProps() {
  if (!camera) {
    QLOG_WARN() << "CameraPropWidget::updateProps> camera is null";
    return;
  }
  QLOG_DEBUG ( ) << "CameraPropWidget::update properties ";
  for (int i = 0 ; i < propList.size(); i++) {
    QLabel * propLabel = propList.at(i);
    propLabel->setText(camera->propList.at(i)); 
    QLOG_DEBUG ( ) << "CameraPropWidget::update property "
                   << camera->propList.at(i);
  }
}

