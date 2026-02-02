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

#include <iostream>
#include <stdio.h>
#include <sstream>
#include <math.h>
#include "DriverMicos_Pollux.h"

using namespace DriverDefinition;

const int DriverMicos_Pollux::BUFFER_SIZE          = 32; // buffer size for serial messages
const int DriverMicos_Pollux::MAX_DEVICES          = 16; // maximum number of Micos_Pollux controllers
const int DriverMicos_Pollux::NB_ITEM_INIT_SETTING =  1; // number of items at the actuator
// setting string
const int DriverMicos_Pollux::NB_ITEM_DRV_SETTING  =  1; // number of items at the driver
//setting string

const DriverFeature DriverMicos_Pollux::MICOS_POLLUX_FEATURE = 
  { "deg","deg",1000.,1000.,CLOSED_LOOP,NULL };

// 
// Init implementation
//
int DriverMicos_Pollux::Init(string& rstateData) const 
{
  QLOG_DEBUG ( ) << "DriverMicos_Pollux::Init";

  int retStatus = 0;
  ADLimitSwitch limitSwitch;

  if (OperationComplete(rstateData, "NO_MICOS_POLLUX", limitSwitch) < 0)
    {
      retStatus = -1;
    }
  return retStatus;
}

//
// InitActuator implementation
//
// use company delivered optimized settings
//
int DriverMicos_Pollux::InitActuator(string actuatorSetting,
				     float  position) const 
{
  QLOG_DEBUG ( ) << "DriverMicos_Pollux::InitActuator";

  int retStatus = 0;
  string command = "";
  double vel = 0;
  char buffer[BUFFER_SIZE];
  // parse configuration parameters
  //   vel=xxx
  if (sscanf(actuatorSetting.c_str(),
	     "vel=%lf",
	     &vel) != NB_ITEM_INIT_SETTING) {
    ReportSettingError("Actuator setting string format incorrect");
    retStatus = -1;
    return retStatus;
  }

    // first send the address code
  if (SendAddressCode() != 0) {
    QLOG_DEBUG ( ) << "Could not send address code";
    retStatus = -1;
    return retStatus;
  }
  
  // send SVxxx<CR> to set the velocity
  sprintf(buffer, "%lf %d setnvel\r", vel,_axisNumber);
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) {
    ReportCommError(QString("Unable to write to port, command %1")
                    .arg(QString(command.c_str())));
    retStatus = -1;
    return retStatus;
  }

  // computation of the offset to apply to get a position
  _originOffset = 0.;
  return retStatus;
}

//
// GetPos implementation
//
// 1. send "npos<CR>" (tell position) to get the current position
// 2. read reply
// 3. parse position from reply
//
int DriverMicos_Pollux::GetPos(string  actuatorSetting,
			       float&  position) const 
{
  QLOG_DEBUG ( ) << "DriverMicos_Pollux::GetPos";

  int retStatus = 0;

  string command = "";
  string answer = "";
  char buffer[BUFFER_SIZE];
  // init position to zero
  position = 0;
    
  // first send the address code
  if (SendAddressCode() != 0) {
    QLOG_DEBUG ( ) << "Could not send address code";
    retStatus = -1;
    return retStatus;
  }
    
  // create buffer with command np<CR>
  sprintf(buffer, "%d npos\r", _axisNumber);

  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) {
    ReportCommError("Unable to write to port");
    retStatus = -1;
    return retStatus;
  }

  // read reply
  if (!_pcommChannel->Read(answer,NULL)) {
    ReportCommError("Unable to read from port");
    retStatus = -1;
    return retStatus;
  }
  
  // parse and extract position
  const char* realAnswer = answer.c_str();
  sscanf (realAnswer, "%f", &position);
  QLOG_DEBUG ( ) << "GetPos gives : " << position << " origineoffset : " << _originOffset;
  return retStatus;
}

//
// Move implementation
//
// 1. send "nrmove<CR>" (move relative) to move xxxx steps
//
int DriverMicos_Pollux::Move(string actuatorSetting,
			     float  nbSteps,
			     int    unit) const 
{
  QLOG_DEBUG ( ) << "DriverMicos_Pollux::Move";

  int retStatus = 0;
  float range;
  float motion;
 
  string command = "";
  char buffer[BUFFER_SIZE];

  if ( ConvertUnit(unit,nbSteps,motion,range) ||
       (fabs(motion) > range))
    {
      retStatus  = DISPLACEMENT_EXCEEDS_RANGE;
      _lastError = OUT_OF_RANGE_ERROR;
    }
  else
    {
      // first send the address code
      if (SendAddressCode() != 0) {
	QLOG_DEBUG ( ) << "Could not send address code";
	retStatus = -1;
	return retStatus;
      }

      // create buffer with command MRxxxx<CR>
      sprintf(buffer, "%f %d nrmove\r", motion, _axisNumber);
      QLOG_DEBUG ( ) << "Send MoveRel Command " << QString( buffer);
      // send command
      command = buffer;
      if (_pcommChannel->Write(command,NULL) < (int)command.length()) {
	ReportCommError("Unable to write to port");
	retStatus = -1;
      }
    }
  
  return retStatus;
}

//
// MoveAbs implementation
//
// 1. send "nmove<CR>" (move absolute) to move to position absPos (not used because open_loop device)
//
int DriverMicos_Pollux::MoveAbs(string actuatorSetting,
				float  absPos,
				int    unit) const 
{
  QLOG_DEBUG ( ) << "DriverMicos_Pollux::MoveAbs";
  
  int retStatus = -1;
  float convertedAbsPos;
  float range;
  string command = "";
  char buffer[BUFFER_SIZE];

  if (!ConvertUnit(unit,absPos,convertedAbsPos,range))
    {
      // first send the address code
      if (SendAddressCode() != 0) {
	QLOG_DEBUG ( ) << "Could not send address code";
	return retStatus;
      }
      convertedAbsPos -= _originOffset;
      // create buffer with command nm<CR>
      sprintf(buffer, "%f %d nmove\r", convertedAbsPos, _axisNumber);
      QLOG_DEBUG ( ) << "Send MoveAbs Command " <<  QString( buffer);
      // send command
      command = buffer;
      if (_pcommChannel->Write(command,NULL) < (int)command.length()) { 
	ReportCommError("Unable to write to port");
      }
      else 
	{
	  retStatus = 0;
	}
    }
  return retStatus;
}

//
// Stop implementation
//
// 1. send "nabort<CR>" (stop smoothly) to stop the
//    current movement
//
int DriverMicos_Pollux::Stop(string actuatorSetting) const 
{
  QLOG_DEBUG ( ) << "DriverMicos_Pollux::Stop";

  int retStatus = 0;

  string command = "";
  char buffer[BUFFER_SIZE];
    
  // first send the address code
  if (SendAddressCode() != 0) {
    QLOG_DEBUG ( ) << "Could not send address code";
    retStatus = -1;
    return retStatus;
  }

  // create buffer with command AB1<CR>
  sprintf(buffer, "%d nabort\r", _axisNumber);
  
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) { 
    ReportCommError("Unable to write to port");
    retStatus = -1;
  }

  return retStatus;
}

//
// OperationComplete implementation
//
// 1. send "nst<CR>" to retrieve the status
// 2. get the reply
// 3. parse the reply
// 4. compare the bits
//
//
int DriverMicos_Pollux::OperationComplete(string& rstateData,
					  string actuatorSetting,
					  ADLimitSwitch& rlimitSwitch) const
{
  QLOG_DEBUG ( ) << "DriverMicos_Pollux::OperationComplete";
   
  int retStatus = -1;

  string       command = "";
  string       answer = "";
  char         buffer[BUFFER_SIZE];
  int status = 0;

  // first send the address code
  if (SendAddressCode() != 0) {
    QLOG_DEBUG ( ) <<"Could not send address code";
    retStatus = -1;
    return retStatus;
  }
   
  // create buffer with command nst<CR>
  sprintf(buffer, "%d nst\r", _axisNumber);
  QLOG_DEBUG ( ) <<"send message : " <<QString(   buffer);
 
    
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) { 
    ReportCommError("Unable to write to port");
    retStatus = -1;
    return retStatus;
  }

  // read reply
  if (!_pcommChannel->Read(answer,NULL)) {
    ReportCommError("Unable to read from port");
    retStatus = -1;
    QLOG_DEBUG ( ) << "Status reply expected, but got reply " << QString(answer.c_str());
    return retStatus;
  }
  QLOG_DEBUG ( ) << "Status reply = " << QString( answer.c_str());
  rstateData = answer;
  // No limits !
  rlimitSwitch = IN_BETWEEN;

  // Evaluate status of motion
  const char* realAnswer = answer.c_str();
  sscanf (realAnswer, "%d", &status);
  if (status == 1) // motion not completed
    retStatus = 0;
  if (status == 0) // motion completed
    retStatus = 1; 
 
  return retStatus;
}

//
// GetActuatorFeature implementation
//
//    Gives the translation stages features
//
int DriverMicos_Pollux::GetActuatorFeature(DriverFeature& ractuatorFeature) const
{
  QLOG_DEBUG ( ) << "DriverMicos_Pollux::GetActuatorFeature";

  int retStatus = 0;
  
  ractuatorFeature = MICOS_POLLUX_FEATURE;
 
  return retStatus;
}
//
// SendAddressCode helper function
// 
// 1. parse the driverSetting (_setting) string and extract the
//    controller's device number
// 2. create the device address code
// 3. send the address code
//
int DriverMicos_Pollux::SendAddressCode() const
{
  QLOG_DEBUG ( ) << "DriverMicos_Pollux::SendAddressCode";
  
  int retStatus = 0;
  string command = "";
  
  // parse driverSetting string for axis number
  if (!sscanf(_setting.c_str(),"axisNumber=%d",
	      &_axisNumber)) {
    ReportSettingError("driverSetting string incorrect");
    retStatus = -1;
    return retStatus;
  }
  
  // check for range
  QLOG_DEBUG ( ) << "axis number " << _axisNumber;
  if ((_axisNumber < 1) || (_axisNumber > MAX_DEVICES)) {
    ReportRangeError("axis number out of range");
    retStatus = -1;
    return retStatus;
  }
  return retStatus;
}

// -----------------------------------------------------------------------------
/// Operation : ConvertUnit
///  Performs the following conversion :
///  - from custom unit to default unit
///  - no conversion
///  - from default unit to custom
///  converts a value into default motion unit if it is expressed in custom one
///  and gives the range associated to the unit
//
/// @param unit
///    motion unit
/// @param valueToConvert
///    value to be converted
/// @rconvertedValue
///    conversion result
// -----------------------------------------------------------------------------
// Use indifferently default units or custom units configuration (STEPS == MM)
int DriverMicos_Pollux::ConvertUnit(int unit, 
				    float valueToConvert, 
				    float& rconvertedValue,
				    float& rrange) const 
{
  QLOG_DEBUG ( ) << "DriverMicos_Pollux::ConvertUnit";

  int retStatus = 0;
  const float STEPS_FOR_ONE_MM = 1;

  if (unit == CUSTOM_UNIT)
    {
      rrange  = MICOS_POLLUX_FEATURE.customRange * STEPS_FOR_ONE_MM;
      rconvertedValue = valueToConvert * STEPS_FOR_ONE_MM; 
    }
  else if (unit == DEFAULT_UNIT)
    {
      rrange  = MICOS_POLLUX_FEATURE.range;
      rconvertedValue = valueToConvert; 
    }
  else if (unit == FROM_DEFAULT_TO_CUSTOM)
    {
      rrange = MICOS_POLLUX_FEATURE.customRange;
      rconvertedValue =  valueToConvert / STEPS_FOR_ONE_MM;
    }    
  else
    {
      retStatus = -1;
    }

  return retStatus;
}
