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

#ifndef ACQUISITIONTHREAD_H
#define ACQUISITIONTHREAD_H

#include "AcquisitionSequence.h"
#include "Camera.h"
#include "Motor.h"
#include "SuperK.h"
#include "Dac.h"
#include "Comedi.h"
#include "Raspi.h"
#include "QsLog.h"


/// Executes acquisition sequences in a worker thread.
///
/// This thread coordinates cameras, motors, DACs, and file I/O for each
/// acquisition record and emits progress/status signals to the UI.
class AcquisitionThread : public QThread
{
  Q_OBJECT

public:
  /// Create a new acquisition thread.
  explicit AcquisitionThread(QObject* parent = nullptr);
  /// Destructor.
  ~AcquisitionThread() override;

  /// Set the list of active cameras.
  /// \param cameraList Cameras used during acquisition.
  void setCamera(const QVector<Camera*>& cameraList);
  /// Provide the motor controller.
  /// \param motor Motor controller instance.
  void setMotor(Motor* motor);
  /// Provide the SuperK laser controller.
  /// \param superk SuperK controller instance.
  void setSuperK(SuperK* superk);
  /// Provide the DAC controller.
  /// \param dac DAC controller instance.
  void setDac(Dac* dac);
  /// Provide the Comedi counter controller.
  /// \param comedi Comedi counter instance.
  void setComediCounter(Comedi* comedi);
  /// Provide the Comedi DAC controller.
  /// \param comedi Comedi DAC instance.
  void setComediDac(Comedi* comedi);
  /// Provide the Raspi DAC controller.
  /// \param raspi Raspi DAC instance.
  void setRaspiDac(Raspi* raspi);
  /// Set the output filename and index.
  /// \param filename Base output filename.
  /// \param filenumber Output file index.
  void setFile(const QString& filename, int filenumber);
  /// Set the sequence list to execute.
  /// \param sequenceList Acquisition sequences to run.
  void setSequenceList(const QVector<AcquisitionSequence*>& sequenceList);
  /// Set which camera windows are open in the UI.
  /// \param isOpenCameraWindow Per-camera open flags.
  void setIsOpenCameraWindow(const QVector<bool>& isOpenCameraWindow);
  /// Wake the splash screen waiting for acquisition.
  void wakeSplashScreen();
  /// Request the thread to stop.
  void stop();

signals:
  /// Emit motor position text.
  /// \param positionQString Position string.
  void getPosition(QString positionQString);
  /// Emit SuperK data as text.
  /// \param dataStr Data string.
  void getSuperKData(QString dataStr);
  /// Emit SuperK power value.
  /// \param power Power value.
  void getPower(int power);
  /// Emit SuperK ND value.
  /// \param nd ND value.
  void getNd(int nd);
  /// Emit SuperK SWP value.
  /// \param swp SWP value.
  void getSwp(int swp);
  /// Emit SuperK LWP value.
  /// \param lwp LWP value.
  void getLwp(int lwp);
  /// Emit camera acquisition status.
  /// \param imagesuccess True if image acquisition succeeded.
  void getCameraStatus(bool imagesuccess);
  /// Emit DAC status.
  /// \param dacsuccess True if DAC operation succeeded.
  void getDacStatus(bool dacsuccess);
  /// Emit motor status.
  /// \param motorsuccess True if motor operation succeeded.
  void getMotorStatus(bool motorsuccess);
  /// Emit SuperK status.
  /// \param superksuccess True if SuperK operation succeeded.
  void getSuperKStatus(bool superksuccess);
  /// Emit treatment status.
  /// \param treatmentsuccess True if treatment succeeded.
  void getTreatmentStatus(bool treatmentsuccess); 
  /// Emit acquisition progress record.
  /// \param record Current record index.
  void getAcquiring(int record);
  /// Emit the current file number.
  /// \param number File index.
  void getFilenumber(int number);
  /// Emit warnings to the UI.
  /// \param message Warning message.
  void showWarning(QString message);
  /// Show a splash screen with image and position.
  /// \param imagepath Path to the splash image.
  /// \param screen_x Screen X coordinate.
  /// \param screen_y Screen Y coordinate.
  void splashScreen(QString imagepath, int screen_x, int screen_y);	
  /// Request analysis execution (triggered by ANALYSE=START in settings).
  void requestAnalysis();

protected:
  /// Thread entry point.
  void run() override;

private:
  /// Execute one acquisition sequence.
  /// \param sequence Sequence to execute.
  void execute(AcquisitionSequence *sequence);
  /// Advance to the next record.
  /// \param sequence Sequence being executed.
  /// \param cur_record Current record index.
  void nextRecord(AcquisitionSequence *sequence, int cur_record);
  /// Persist acquired data for the current record.
  /// \param sequence Sequence being executed.
  /// \param cur_record Current record index.
  void saveData(AcquisitionSequence *sequence, int cur_record);
  /// Store an image from a camera into the sequence buffer.
  /// \param buffer Image buffer.
  /// \param width Image width.
  /// \param height Image height.
  /// \param bitdepth Bit depth indicator.
  void setImageFromCamera(uchar*, int, int, int);
  /// Determine snake/reverse behavior for a record.
  /// \param cur_record Current record index.
  bool isSnakeReverseForRecord(int cur_record) const;
  /// Update child sequence snake/reverse state.
  /// \param parent_start Parent start record.
  /// \param parent_end Parent end record.
  /// \param reset True to reset state.
  void updateChildSnakeReverse(int parent_start, int parent_end, bool reset);

  QVector<Camera*> cameraList;
  QVector<AcquisitionSequence*> sequenceList;
  QVector<bool> isopencamerawindow;
  QMutex* mutex;
  QWaitCondition* splashScreenOk;

  int record;
  Motor* motor;
  SuperK* superk;
  Dac* dac;
  Comedi* comedicounter;
  Comedi* comedidac;
  Raspi* raspidac;
  bool dacsuccess;
  bool comedicountersuccess;
  bool comedidacsuccess;
  bool raspidacsuccess;
  bool slmsuccess;
  bool imagesuccess;
  bool filesuccess;
  bool treatmentsuccess;
  bool suspend;

  // Data File
  hid_t file_id;
  QVector<hid_t> ids;
  herr_t status;
  QString filename;
  int filenumber;
 
};

#endif // ACQUISITIONTHREAD_H
