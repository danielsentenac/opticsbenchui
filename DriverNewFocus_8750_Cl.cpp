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
#include "DriverNewFocus_8750_Cl.h"

using namespace DriverDefinition;

const int DriverNewFocus_8750_Cl::NB_ITEM_SETTING = 1;
const int DriverNewFocus_8750_Cl::WRITE_ERROR     = -1;
const int DriverNewFocus_8750_Cl::READ_ERROR      = -2;
const ADActuatorFeature DriverNewFocus_8750_Cl::NEWFOCUS_8750_CL_FEATURE 
= { "pulses","pulses",3.0e6,3.0e6,CLOSED_LOOP ,NULL};

static const string DELIMITER = "=";


// -----------------------------------------------------------------------------
/// Operation : Init
///    Initializes the driver by putting it in online and servocontrol mode 
///    to perform remote control
/// @param rstateData
///    state data of the driver
// -----------------------------------------------------------------------------
int DriverNewFocus_8750_Cl::Init(string& rstateData) const 
{
  QLOG_DEBUG() << "DriverNewFocus_8750_Cl::Init";

  int retStatus = 0;
  string defaultLoadCommand;
  string enableCommand;
  string servoLoopEnableCommand;
  string getStatusCommand;
  string answer;

  defaultLoadCommand     = "DEF\n";
  enableCommand          = "MON A" + _setting + "\n";
  servoLoopEnableCommand = "SER A" + _setting + "\n"; 
  getStatusCommand = "STA A" + _setting + "\n";

  if (SendCommand(defaultLoadCommand,answer) == WRITE_ERROR ||
      SendCommand(enableCommand,answer) == WRITE_ERROR ||
      SendCommand(servoLoopEnableCommand,answer) == WRITE_ERROR ||
      SendCommand(getStatusCommand,answer) ||
      answer.find(DELIMITER,0) == string::npos)
 
  {
    QLOG_DEBUG() << "Init failed for A" << QString(_setting.c_str());
    retStatus = -1;
  }
  else 
  {
    rstateData = answer;
  }
  return retStatus;
}

// -----------------------------------------------------------------------------
/// Operation : InitActuator
///    Initializes the actuator whose setting is given in parameter
///    by setting its velocity
//
/// @param actuatorSetting
///   actuator setting
/// @param position
///   required position at initialization
///
// -----------------------------------------------------------------------------
int DriverNewFocus_8750_Cl::InitActuator(string actuatorSetting,
                                         float  position) const 
{
  QLOG_DEBUG() << "DriverNewFocus_8750_Cl::InitActuator";

  const int NB_ITEM_INIT_SETTING = 2;
  int retStatus = 0;
  int motor;
  float frequency;
  string answer;
  ostringstream setFrequencyCommandstream;
  ostringstream setPositionCommandstream;

  if (sscanf(actuatorSetting.c_str(),"motor=%d type=%*d frequency=%f",
	     &motor,&frequency) != NB_ITEM_INIT_SETTING)
  {
    throw("Bad Actuator setting\n");
  }
  setFrequencyCommandstream << "VEL A" << _setting  
			    <<   " "   << motor << "=" << frequency << "\n";

  setPositionCommandstream << "POS A" << _setting << " " << position << "\n";

  if ((SendCommand(setFrequencyCommandstream.str(),answer) == WRITE_ERROR) || 
      (SendCommand(setPositionCommandstream.str(),answer) == WRITE_ERROR))
  {
    QLOG_DEBUG() << "Init actuator failed on A" << QString(_setting.c_str()) << ":" <<  motor;
    retStatus = -1;
  }

  return retStatus;
}

// -----------------------------------------------------------------------------
/// Operation : GetPos
///    Returns the position of the actuator whose setting is given in parameter
//
/// @param actuatorSetting
///    actuator setting
/// @param position
///    Position of the actuator
// -----------------------------------------------------------------------------
int DriverNewFocus_8750_Cl::GetPos(string  actuatorSetting,
                                   float&  position) const 
{
  QLOG_DEBUG() << "DriverNewFocus_8750_Cl::GetPos";

  const int GET_POS_NB_ITEM = 1;
  int retStatus = 0;
  string answer;
  ostringstream getPositionCommandstream;

  getPositionCommandstream<< "POS A" << _setting << endl;

  if (SendCommand(getPositionCommandstream.str(),answer))
  {
    QLOG_DEBUG() << "Get position failed on A" << QString(_setting.c_str());
    retStatus = -1;
  } 
   
  if (sscanf(answer.c_str(),"A%*d=%f",&position) != GET_POS_NB_ITEM)
  {
    position  = 0;
    retStatus = -1;
   
  } 

  return retStatus;
}

// -----------------------------------------------------------------------------
/// Operation : Move
///    Moves the actuator whose setting is given in parameter on nbSteps
//
/// @param actuatorSetting
///    actuator setting
/// @param nbSteps
///    number of steps to perform
/// @param unit
///   unit used for the motion. 0 = default unit,  1 = custom one
//    
// -----------------------------------------------------------------------------
int DriverNewFocus_8750_Cl::Move(string actuatorSetting,
                                 float  nbSteps,
                                 int    unit) const 
{
  QLOG_DEBUG() << "DriverNewFocus_8750_Cl::Move";

  int retStatus = OK;
  float range;
  float motion;
  string answer;
  ostringstream moveCommandstream;
  string startCommand;
   
  if (ConvertUnit(unit,nbSteps,motion,range) ||
      (fabs(motion) > range))
  {
    _lastError = OUT_OF_RANGE_ERROR;
    retStatus = DISPLACEMENT_EXCEEDS_RANGE;
  }
  else 
  {
    moveCommandstream << "REL A" << _setting << "=" << (int)motion << "\n";
    startCommand = "GO A" + _setting + "\n";
    if (SendCommand(moveCommandstream.str(),answer) == WRITE_ERROR ||
	SendCommand(startCommand,answer) == WRITE_ERROR)
    {
      QLOG_DEBUG() << "Move failed on A" << QString(_setting.c_str());
      retStatus = -1;
    }
  }
  return retStatus;
}

// -----------------------------------------------------------------------------
/// Operation : MoveAbs
///    Moves the actuator whose setting is given in parameter to the selected
///    position. On the open loop picomotors, this feature is not available.
///    
//
/// @param actuatorSetting
///    actuator setting
/// @param absPos
///    position to reach
/// @param unit
///   unit used for the motion. 0 = default unit,  1 = custom one
//    
// -----------------------------------------------------------------------------
int DriverNewFocus_8750_Cl::MoveAbs(string actuatorSetting,
                                    float  absPos,
                                    int    unit) const 
{
  QLOG_DEBUG() << "DriverNewFocus_8750_Cl::MoveAbs";

  int retStatus = -1;
  float convertedAbsPos;
  float range;
  string startCommand;
  string answer;
  ostringstream moveAbsCommandstream;

  if (!ConvertUnit(unit,absPos,convertedAbsPos,range))
  {
    moveAbsCommandstream << "ABS A" << _setting << "=" << (int)absPos << "\n";
    startCommand = "GO A" + _setting + "\n";
    if (SendCommand(moveAbsCommandstream.str(),answer) == WRITE_ERROR ||
	SendCommand(startCommand,answer) == WRITE_ERROR)
    {
      QLOG_DEBUG() << "MoveAbs failed on A" << QString(_setting.c_str());
    }
    else 
    {
      retStatus = 0;
    }
  }
  return retStatus;
}

// -----------------------------------------------------------------------------
/// Operation : Stop
///    Stops the motion of the current active picomotor on the driver
//
/// @param actuatorSetting
///    actuator setting
// -----------------------------------------------------------------------------
int DriverNewFocus_8750_Cl::Stop(string actuatorSetting) const 
{
  QLOG_DEBUG() << "DriverNewFocus_8750_Cl::Stop";

  int retStatus = 0;
  string stopChannelCommand;
  string answer;

  stopChannelCommand = "STO A" + _setting + "\n";

  if (SendCommand(stopChannelCommand,answer) == WRITE_ERROR) 
  {  
    QLOG_DEBUG() << "Stop failed on A" << QString(_setting.c_str());
  }

 
  return retStatus;
}


// -----------------------------------------------------------------------------
/// Operation : OperationComplete
///   Checks the completion of a motion : 
///      0 : motion is being performed 
///      1 : motion is performed 
//
/// @param rstateData
///   state data of the driver
/// @param actuatorSetting
///    actuator setting
/// @param rlimitSwitch
///    reached limit flag
// -----------------------------------------------------------------------------
int DriverNewFocus_8750_Cl::OperationComplete(string& rstateData,
                                              string actuatorSetting,
                                              ADLimitSwitch& rlimitSwitch) const 
{
  QLOG_DEBUG() << "DriverNewFocus_8750_Cl::OperationComplete";

  const unsigned int UPPER_LIMIT_STATE = 0x40;
  const unsigned int LOWER_LIMIT_STATE = 0x20;
  const unsigned int MOTION_PERFORMED  = 0x01;

  int retStatus = -1;
  unsigned int driverStatus;
  string answer;
  string getStatusCommand;
  string servoLoopEnableCommand;
  istringstream streamStatus;

  getStatusCommand       = "STA A" + _setting + "\n";
  servoLoopEnableCommand = "SER A" + _setting + "\n"; 

  if (SendCommand(getStatusCommand,answer))
  {
    QLOG_DEBUG() << "Operation Complete failed for A" << QString(_setting.c_str());
    retStatus = -1;
  }
  else 
  {
    unsigned int statusIndice;
    rstateData    = answer;
    statusIndice = answer.find(DELIMITER,0);
    if (statusIndice != string::npos)
    {
      // status reading
      statusIndice++;
      streamStatus.str(answer.substr(statusIndice,answer.length()-statusIndice));
      streamStatus.setf(ios_base::hex, ios_base::basefield);
      streamStatus >> driverStatus;

      // reached limit checking
      if (!(driverStatus & UPPER_LIMIT_STATE)  && 
	  !(driverStatus & LOWER_LIMIT_STATE)) 
      {
	// end of motion status reading   
	retStatus = driverStatus & 0x01;
	rlimitSwitch = IN_BETWEEN;
      }
      // driver may be in open loop mode
      // better to switch again in the closed loop one
      else if (SendCommand(servoLoopEnableCommand,answer) != WRITE_ERROR )
      {
            
	if ((driverStatus & UPPER_LIMIT_STATE)  && 
	    (driverStatus & LOWER_LIMIT_STATE))
	{
	  rlimitSwitch = UNAVAILABLE;
	}
	else if (driverStatus & UPPER_LIMIT_STATE)
	{
	  rlimitSwitch = UPPER_LIMIT;
	  retStatus = MOTION_PERFORMED;
	}
	else if (driverStatus & LOWER_LIMIT_STATE)
	{
	  rlimitSwitch = LOWER_LIMIT;
	  retStatus = MOTION_PERFORMED;
	} 
      }
    }
  }
 
  return retStatus;
}

// -----------------------------------------------------------------------------
/// Operation : GetActuatorFeature
//    Gives the closed loop picomotor features
//
/// @param ractuatorFeature
///    actuator setting
// -----------------------------------------------------------------------------
int DriverNewFocus_8750_Cl::GetActuatorFeature(ADActuatorFeature& ractuatorFeature) const
{
  QLOG_DEBUG() << "DriverNewFocus_8750_Cl::GetActuatorFeature";

  int retStatus = 0;
  
  ractuatorFeature = NEWFOCUS_8750_CL_FEATURE;
 
  return retStatus;
}

// -----------------------------------------------------------------------------
/// Operation : SendCommand
///   Send a command via the communication channel
///   A distinction is made between read and write error as a readout error
///   does not matter for some commands. 
//
/// @param command
///    command to send
/// @param answer
///    answer to the command
// -----------------------------------------------------------------------------
int DriverNewFocus_8750_Cl::SendCommand(string command,
                                        string& answer) const
{
  QLOG_DEBUG() << "DriverNewFocus_8750_Cl::SendCommand";

  int retStatus = 0;
  if (_pcommChannel->Write(command,NULL) < (int)command.length())
  {
    QLOG_DEBUG() << "Unable to write to port";
    _lastError = COMMUNICATION_ERROR;
    retStatus = WRITE_ERROR;
  }
  else if (!_pcommChannel->Read(answer,NULL))
  {
    QLOG_DEBUG() << "Unable to read from port";
    _lastError = COMMUNICATION_ERROR;
    retStatus = READ_ERROR;
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
int DriverNewFocus_8750_Cl::ConvertUnit(int unit, 
                                        float valueToConvert, 
                                        float& rconvertedValue,
                                        float& rrange) const
{
  QLOG_DEBUG() << "DriverNewFocus_8750_Cl::ConvertUnit";

  int retStatus = 0;

  if (unit == CUSTOM_UNIT)
  {
    rrange  = NEWFOCUS_8750_CL_FEATURE.customRange;
    rconvertedValue = valueToConvert; 
  }
  else if (unit == DEFAULT_UNIT)
  {
    rrange  =  NEWFOCUS_8750_CL_FEATURE.range;
    rconvertedValue = valueToConvert; 
  }
  else if (unit == FROM_DEFAULT_TO_CUSTOM)
  {
    rrange = NEWFOCUS_8750_CL_FEATURE.customRange;
    rconvertedValue =  valueToConvert;
  }    
  else if (unit == FROM_DEFAULT_TO_CUSTOM)
  {
    rrange = NEWFOCUS_8750_CL_FEATURE.customRange;
    rconvertedValue =  valueToConvert ;
  }    
  else
  {
    retStatus = -1;
  }

  return retStatus;
}
