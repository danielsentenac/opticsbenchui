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

/**
 * @file AcquisitionSequence.h
 * @brief Acquisition sequence model.
 */

#ifndef ACQUISITIONSEQUENCE_H
#define ACQUISITIONSEQUENCE_H

#include <QtWidgets>
#ifndef NO_HDF5
#include "hdf5.h"
#include "hdf5_hl.h"
#else
typedef int hid_t;
typedef int herr_t;
#endif
#include "FileParser.h"
#include "QsLog.h"



/// Holds the full state of one acquisition step.
///
/// This structure aggregates scan parameters, device actions, and
/// buffers used during acquisition. It is passed between acquisition
/// coordination components and file persistence logic.
class AcquisitionSequence
{  
public:
  enum {
   IS_LOOP = 0,
   LOOP_START = 1,
   LOOP_END = 2
  };
   
  /// Create a new sequence with default values.
  AcquisitionSequence();
  /// Destructor.
  ~AcquisitionSequence();

  /// Elapsed time for the sequence (seconds).
  double etime;   
  /// Current record index.
  int record;
  /// Sequence record index.
  int seq_record;
  /// Status flag for the sequence.
  bool status;

  /// Instrument name.
  QString instrumentName;
  /// Instrument type identifier.
  QString instrumentType;
  /// Settings string.
  QString settings;
  /// Scan plan description.
  QString scanplan;
  /// Loop index.
  int loop;
  /// Loop end index.
  int loopends;
  /// Total number of loops.
  int loopNumber;
  /// Remaining loop count.
  int remainingLoops;
  /// Whether loop uses snake pattern.
  bool loopSnake;
  /// Whether loop is reversed.
  bool loopReverse;
  
  // File attributes
  /// File parser used for loading input data.
  FileParser *fileParser;

  // Treatment attributes
  /// Averaging mode identifier.
  QString avg ;
  /// Whether to reset treatment state.
  bool    reset;
  /// Treatment identifier.
  QString treatment;
  /// Averaging operation.
  QString avgOp;
  /// Average success flag.
  int avgSuccess;
  /// Average right-side count.
  int avgRight;
  /// Average left-side count.
  int avgLeft;
  /// Instrument reference string.
  QString instrumentRef;
  
  // Acquisition attributes
  /// Sleep duration between operations (ms).
  int sleep;

  // Motor attributes
  /// Motor action string.
  QString motorAction;
  /// SuperK action string.
  QString superkAction;
  /// Motor value for the action.
  float   motorValue; 
  /// Cached motor position.
  float   position;

  // SuperK attributes
  /// SuperK power value.
  float   superkPowerValue;
  /// SuperK ND value.
  float   superkNdValue;
  /// SuperK SWP value.
  float   superkSwpValue;
  /// SuperK LWP value.
  float   superkLwpValue;
  /// SuperK center wavelength value.
  float   superkCwValue;
  /// SuperK bandwidth value.
  float   superkBwValue;
  /// SuperK raw power value.
  float   superkRPowerValue;
  /// SuperK raw ND value.
  float   superkRNdValue;
  /// SuperK raw SWP value.
  float   superkRSwpValue;
  /// SuperK raw LWP value.
  float   superkRLwpValue;
  /// SuperK raw center wavelength value.
  float   superkRCwValue;
  /// SuperK raw bandwidth value.
  float   superkRBwValue;

  // Dac attributes
  /// DAC value (engineering units).
  double   dacValue;
  /// DAC value (raw units).
  double   dacRValue;
  /// DAC output index.
  int      dacOutput;

  // Counter Comedi attributes
  /// Comedi value (integer).
  int    comediValue;
  /// Comedi data (engineering units).
  double comediData;
  /// Comedi output index.
  int    comediOutput;

  // SLM attributes
  /// Full path to SLM image set.
  QString fullpath;
  /// Current image path.
  QString imagepath;
  /// Image type/extension.
  QString imagetype;
  /// Keep-zero flag for SLM.
  QString keepzero;
  /// Loaded SLM image.
  QImage *slmimage;
  /// Image number.
  int imgnum;
  /// Start number for sequence.
  int startnum;
  /// Step number for sequence.
  int stepnum;
  /// SLM width in pixels.
  int slmwidth;
  /// SLM height in pixels.
  int slmheight;
  /// Screen X coordinate.
  int screen_x;
  /// Screen Y coordinate.
  int screen_y;
  /// Label used for SLM display.
  QLabel *slmLabel;

  // Camera attributes
  /// 2D float image buffer.
  float   *data_2D_FLOAT;
  /// 2D float image width.
  int data_2D_FLOAT_DIM_X;
  /// 2D float image height.
  int data_2D_FLOAT_DIM_Y;
  /// 2D float image minimum.
  float data_2D_FLOAT_MIN;
  /// 2D float image maximum.
  float data_2D_FLOAT_MAX;
  /// 8-bit image buffer.
  uchar *image;
  /// 16-bit image buffer.
  ushort *image16;
  /// 32-bit image buffer.
  int   *image32;
  /// Image minimum.
  int   imageMin;
  /// Image maximum.
  int   imageMax;
  /// Image width.
  int   imageWidth;
  /// Image height.
  int   imageHeight;

  // HDF5 attributes
  /// HDF5 group name.
  QString group;
  /// HDF5 data group name.
  QString datagroup;
  /// HDF5 dataset name.
  QString dataname;
  /// Group increment index.
  int     inc_group;
  /// HDF5 reference group handle.
  hid_t   refgrp;
  /// HDF5 group handle.
  hid_t   grp;
  /// HDF5 temporary group handle.
  hid_t   tmpgrp;
  /// HDF5 group name string.
  QString grpname;

  /// Attach an 8-bit image buffer.
  /// \param buffer Pointer to the image data.
  /// \param width Image width in pixels.
  /// \param height Image height in pixels.
  void setImage(uchar* buffer, int width, int height);
  /// Attach a 16-bit image buffer.
  /// \param buffer16 Pointer to the image data.
  /// \param width Image width in pixels.
  /// \param height Image height in pixels.
  void setImage16(ushort* buffer16, int width, int height);
  /// Attach a 32-bit image buffer.
  /// \param buffer32 Pointer to the image data.
  /// \param width Image width in pixels.
  /// \param height Image height in pixels.
  void setImage32(int* buffer32, int width, int height);
  /// Copy or derive an image from another sequence.
  /// \param sequenceLeft Source sequence.
  /// \return True if an image was set successfully.
  bool setImage(AcquisitionSequence *sequenceLeft);
  /// Set the image minimum for display scaling.
  /// \param _imageMin Minimum pixel value.
  void setImageMin(int _imageMin);
  /// Set the image maximum for display scaling.
  /// \param _imageMax Maximum pixel value.
  void setImageMax(int _imageMax);
  /// Prepare transient buffers and derived data before acquisition.
  void prepare();
  /// Load or refresh file-based data for this sequence.
  bool getFileData();
  /// Return the current 8-bit image buffer.
  uchar* getImage();
  /// Return the current 16-bit image buffer.
  ushort* getImage16();
  /// Return the current 32-bit image buffer.
  int* getImage32();
  /// Read an image buffer from the backing file.
  uchar* getImageFromFile();
  /// Compute averaged data from two sequences.
  /// \param sequenceLeft Left sequence.
  /// \param sequenceRight Right sequence.
  /// \return True on success.
  bool setAvg(AcquisitionSequence *sequenceLeft, AcquisitionSequence *sequenceRight);
  /// Compute phase data from two sequences.
  /// \param sequenceLeft Left sequence.
  /// \param sequenceRight Right sequence.
  /// \return True on success.
  bool setPhase(AcquisitionSequence *sequenceLeft, AcquisitionSequence *sequenceRight);
  /// Compute amplitude data from two sequences.
  /// \param sequenceLeft Left sequence.
  /// \param sequenceRight Right sequence.
  /// \return True on success.
  bool setAmplitude(AcquisitionSequence *sequenceLeft, AcquisitionSequence *sequenceRight);
  
};

#endif // ACQUISITIONSEQUENCE_H
