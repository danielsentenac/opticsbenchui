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

#ifndef ACQUISITIONWIDGET_H
#define ACQUISITIONWIDGET_H

#include <QtSql>
#include <QtWidgets>


#include "QsLog.h"
#include "AcquisitionThread.h"
#include "ComboBoxDelegate.h"

/// \ingroup ui
/// UI widget for configuring and running acquisition sequences.
class AcquisitionWidget : public QWidget
{
  Q_OBJECT
  
public:
  /// Construct the acquisition widget.
  /// \param appDirPath Application directory for resources and DBs.
  explicit AcquisitionWidget(QString appDirPath = QString());
  /// Destructor.
  ~AcquisitionWidget() override;
  
  /// Set the camera list used for acquisition.
  /// \param cameraList Cameras to use during acquisition.
  void setCamera(QVector<Camera*> cameraList);
  /// Set the motor controller.
  /// \param motor Motor controller instance.
  void setMotor(Motor* motor);
  /// Set the SuperK controller.
  /// \param superk SuperK controller instance.
  void setSuperK(SuperK* superk);
  /// Set the DAC controller.
  /// \param dac DAC controller instance.
  void setDac(Dac* dac);
  /// Set the Comedi counter controller.
  /// \param comedi Comedi counter instance.
  void setComediCounter(Comedi* comedi);
  /// Set the Comedi DAC controller.
  /// \param comedi Comedi DAC instance.
  void setComediDac(Comedi* comedi);
  /// Set the Raspi DAC controller.
  /// \param raspi Raspi DAC instance.
  void setRaspiDac(Raspi* raspi);
  /// Create and apply table delegates.
  void setDelegates();
  /// Set the database path used by the widget.
  /// \param path Filesystem path to DBs.
  void setDbPath(QString path);
  /// Set the acquisition file path/prefix.
  /// \param acqfile Acquisition file base.
  void setAcqFile(QString acqfile);

signals:
  /// Emit warnings to the UI.
  /// \param message Warning message.
  void showWarning(QString message);
  /// Request analysis execution.
  void requestAnalysis();
  
public slots:
  /// Refresh displayed values.
  void update();
  /// Reload the acquisition table.
  void reload();
  /// Remove the selected acquisition row.
  void remove();
  /// Start acquisition.
  void run();
  /// Stop acquisition.
  void stop();
  /// Show a splash screen during acquisition.
  /// \param imagepath Path to splash image.
  /// \param screen_x Screen X coordinate.
  /// \param screen_y Screen Y coordinate.
  void splashScreen(QString imagepath, int screen_x, int screen_y);
  /// Update the motor position display.
  /// \param positionQString Position string.
  void getPosition(QString positionQString);
  /// Update SuperK data display.
  /// \param dataStr Data string.
  void getSuperKData(QString dataStr);
  /// Update camera status indicator.
  /// \param imagesuccess True if image acquisition succeeded.
  void getCameraStatus(bool imagesuccess);
  /// Update DAC status indicator.
  /// \param dacsuccess True if DAC operation succeeded.
  void getDacStatus(bool dacsuccess);
  /// Update motor status indicator.
  /// \param motorsuccess True if motor operation succeeded.
  void getMotorStatus(bool motorsuccess);
  /// Update SuperK status indicator.
  /// \param superksuccess True if SuperK operation succeeded.
  void getSuperKStatus(bool superksuccess);
  /// Update treatment status indicator.
  /// \param treatmentstatus True if treatment succeeded.
  void getTreatmentStatus(bool treatmentstatus);
  /// Update acquisition progress.
  /// \param record Current record index.
  void getAcquiring(int record);
  /// Update file number.
  /// \param number File index.
  void getFilenumber(int number);
  /// Track which camera windows are open.
  /// \param isopencamerawindow Per-camera open flags.
  void isopenCameraWindow(QVector<bool> isopencamerawindow);
  /// Show acquisition warning to the user.
  /// \param message Warning message.
  void showAcquisitionWarning(QString message);
  /// Relay analysis request from acquisition thread.
  void requestAnalysisFromThread();
 
private:
  void InitConfig();
  void dbConnexion();
  void setupAcquisitionTable();
  void updateStatusForRecord(const QString& status, int record);
  void setAcquiringToLastRecord();
  QString statusFromSuccess(bool success) const;

  QString appDirPath;
  QString path;
  QString acqfile;
  QString filenumber;
  int acquisitionrow;
  int cur_record;

  QVector<Camera*> cameraList;
  Motor* motor;
  SuperK* superk;
  Dac* dac;
  Comedi* comedicounter;
  Comedi* comedidac;
  Raspi* raspidac;

  QLabel* splashLabel;
  QLabel* acquisitiontitle;
  QSqlTableModel* acquisitiontable;
  QTableView* acquisitionview;
  QPushButton* reloadButton;
  QPushButton* updateButton;
  QPushButton* removeButton;
  QPushButton* runButton;
  QPushButton* stopButton;
  QGridLayout* gridlayout;

  AcquisitionThread* acquisition;
  QVector<AcquisitionSequence*> sequenceList;

};
#endif
