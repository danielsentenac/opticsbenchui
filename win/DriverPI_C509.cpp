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

#include "DriverPI_C509.h"

using namespace DriverDefinition;

static const string PI_C509_DRIVER_STATE_DATA = "NONE";
const string DriverPI_C509::VALID_ACTUATOR_LIST = "A B C";
const DriverFeature DriverPI_C509::PI_C509_FEATURE = 
  { "volts","volts",100,100,CLOSED_LOOP,NULL};

// -----------------------------------------------------------------------------
/// Operation : Init
///    Initializes the driver by putting it in online mode to perform remote 
///    control
/// @param rstateData
///   state data  of the driver
// -----------------------------------------------------------------------------
int DriverPI_C509::Init(string& rstateData) const 
{
  QLOG_DEBUG() << "DriverPI_C509::Init";

  int retStatus = 0;
  string commandLine;
  string answer;

  commandLine = "ONL 1\n";

  if (_pcommChannel->Write(commandLine,NULL) < (int)commandLine.length())
  {
    QLOG_DEBUG() << "Unable to write to port";
    retStatus = -1;
  }
  else 
  {
    commandLine = "*IDN?\n";
    if (_pcommChannel->Write(commandLine,NULL) < (int)commandLine.length())
    {
      QLOG_DEBUG() << "Unable to write to port";
      retStatus = -1;
    }
    else if (!_pcommChannel->Read(answer,NULL))
    {
      QLOG_DEBUG() << "Unable to read from port";
      retStatus = -1;
    }
    else
    {
      QLOG_DEBUG() << "Answer:" << QString(answer.c_str());
      rstateData = PI_C509_DRIVER_STATE_DATA;
    }
  }
  return retStatus;
}

// -----------------------------------------------------------------------------
/// Operation : InitActuator
///    Initializes the actuator whose setting is given in parameter
///    by disabling the servo mode.
//
/// @param actuatorSetting
///   actuator setting
/// @param position
///   required position at initialization (useless for PI_C509
///
// -----------------------------------------------------------------------------
int DriverPI_C509::InitActuator(string actuatorSetting,
                                     float  position) const 
{
  QLOG_DEBUG() << "DriverPI_C509::InitActuator";

  int retStatus = 0;
  string commandLine;

  if (VALID_ACTUATOR_LIST.find(actuatorSetting,0) == string::npos)
  {
    QLOG_ERROR () << "Bad actuator setting";
  }

  commandLine = "SVO " + actuatorSetting + "0" + "\n";

  if (_pcommChannel->Write(commandLine,NULL) < (int)commandLine.length())
  {
    QLOG_DEBUG() << "Cannot put " << QString(actuatorSetting.c_str()) << " in ServoMode";
    retStatus = -1;
  }

  return retStatus;
}

// -----------------------------------------------------------------------------
/// Operation : GetPos
///    Returns the voltage applied on the selected PI_C509 actuator
//
/// @param actuatorSetting
///    actuator setting
/// @param position
///    Position of the actuator
// -----------------------------------------------------------------------------
int DriverPI_C509::GetPos(string  actuatorSetting,
                               float&  position) const 
{
  QLOG_DEBUG() << "DriverPI_C509::GetPos";

  int retStatus = 0;
  string answer;
  string commandLine;
     

  if (VALID_ACTUATOR_LIST.find(actuatorSetting,0) == string::npos)
  {
    QLOG_ERROR () << "Bad actuator setting";
  }

  commandLine = "VOL? " + actuatorSetting  + "\n";

  if (_pcommChannel->Write(commandLine,NULL) < (int)commandLine.length())
  {
    QLOG_DEBUG() << "Unable to write to port";
    retStatus = -1;
  }
  else if (!_pcommChannel->Read(answer,NULL))
  {
    QLOG_DEBUG() << "Unable to read from port";
    retStatus = -1;
  }
  else 
  {
    istringstream iss(answer);
    if (!(iss >> position))
    {
      QLOG_DEBUG() << "Can not get position";
      retStatus = -1;
    }
  }
 
  return retStatus;
}

// -----------------------------------------------------------------------------
/// Operation : Move
///    Moves the selected PI_C509 actuator by changing the voltage applied to it 
///    of nbSteps volts. 
//
/// @param actuatorSetting
///    actuator setting
/// @param nbSteps
///    number of microrad to perform
/// @param unit
///   unit used for the motion.0 = default unit,  1 = custom one
//    
// -----------------------------------------------------------------------------
int DriverPI_C509::Move(string actuatorSetting,
                             float  nbSteps,
                             int    unit) const 
{
  QLOG_DEBUG() << "DriverPI_C509::Move";

  int retStatus = 0;
  float range;
  float motion;
  string commandLine;
  ostringstream oss;

  if (VALID_ACTUATOR_LIST.find(actuatorSetting,0) == string::npos)
  {
    QLOG_ERROR () << "Bad actuator setting";
  }

  if (ConvertUnit(unit,nbSteps,motion,range) ||
       (fabs(motion) > range))
  {
    retStatus  = DISPLACEMENT_EXCEEDS_RANGE;
    _lastError = OUT_OF_RANGE_ERROR;
  }
  else
  {
    oss << "SVR " << actuatorSetting << motion; 
    commandLine = oss.str() + "\n";
   
    if (_pcommChannel->Write(commandLine,NULL) < (int)commandLine.length())
    {
      QLOG_DEBUG() << "Unable to write to port";
      retStatus = -1;
    }
  }
  return retStatus;
}

// -----------------------------------------------------------------------------
/// Operation : MoveAbs
///    Moves the selected PI_C509 actuator by putting the voltage applied to it 
///    to absPos volts.
//
/// @param actuatorSetting
///    actuator setting
/// @param absPos
///     position(microRad) to reach
/// @param unit
///   unit used for the motion.0 = default unit,  1 = custom one
//    
// -----------------------------------------------------------------------------
int DriverPI_C509::MoveAbs(string actuatorSetting,
                                float  absPos,
                                int    unit) const 
{
  QLOG_DEBUG() << "DriverPI_C509::MoveAbs";

  int retStatus = -1;
  float convertedAbsPos;
  float range;
  string commandLine;
  ostringstream oss;

  if (VALID_ACTUATOR_LIST.find(actuatorSetting,0) == string::npos)
  {
    QLOG_ERROR () << "Bad actuator setting";
  }

  if (!ConvertUnit(unit,absPos,convertedAbsPos,range))
  {
    oss << "SVA " << actuatorSetting << convertedAbsPos; 
    commandLine = oss.str() + "\n";
   
    if (_pcommChannel->Write(commandLine,NULL) < (int)commandLine.length())
    {
      QLOG_DEBUG() << "Unable to write to port";
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
///    Stops the motion of the selected PI_C509 actuator
//
/// @param actuatorSetting
///    actuator setting
// -----------------------------------------------------------------------------
int DriverPI_C509::Stop(string actuatorSetting) const 
{
  QLOG_DEBUG() << "DriverPI_C509::Stop";

  int retStatus = 0;
  string commandLine;

  if (VALID_ACTUATOR_LIST.find(actuatorSetting,0) == string::npos)
  {
    QLOG_ERROR () << "Bad actuator setting";
  }

  commandLine = "STP " + actuatorSetting + "\n";

  if (_pcommChannel->Write(commandLine,NULL) < (int)commandLine.length())
  {
    QLOG_DEBUG() << "Unable to write to port";
    retStatus = -1;
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
///    Reached limit flag
// -----------------------------------------------------------------------------
int DriverPI_C509::OperationComplete(string& rstateData,
                                          string actuatorSetting,
                                          ADLimitSwitch& rlimitSwitch) const 
{
  QLOG_DEBUG() << "DriverPI_C509::OperationComplete";

  const float UPPER_TENSION = 115.;
  const float LOWER_TENSION = -15.;
  int retStatus = 0;
  string   answer;
  string commandLine;

  if (VALID_ACTUATOR_LIST.find(actuatorSetting,0) == string::npos)
  {
    QLOG_ERROR () << "Bad actuator setting";
  }

  commandLine = "ONT? " + actuatorSetting + "\n";

  if (_pcommChannel->Write(commandLine,NULL) < (int)commandLine.length())
  {
    QLOG_DEBUG() << "Unable to write to port";
    retStatus = -1;
  }
  else if (!_pcommChannel->Read(answer,NULL))
  {
    QLOG_DEBUG() << "Unable to read from port";
    retStatus = -1;
  }
  else 
  {
    rstateData = PI_C509_DRIVER_STATE_DATA;
    float tension;
    istringstream iss(answer);
    if (!(iss >> retStatus))
    {
      QLOG_DEBUG() << "Can not get motion status";
      retStatus = -1;
    }
    else if (GetPos(actuatorSetting,tension))
    {
      QLOG_DEBUG() << "Can not read tension";
      retStatus = -1;
    }
    else if (tension > UPPER_TENSION)
    {
      rlimitSwitch =  UPPER_LIMIT;    
    }
    else if (tension < LOWER_TENSION)
    {
      rlimitSwitch =  LOWER_LIMIT;    
    }
    else 
    {
      rlimitSwitch = IN_BETWEEN;
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
int DriverPI_C509::GetActuatorFeature(DriverFeature& ractuatorFeature) const
{
  QLOG_DEBUG() << "DriverPI_C509::GetActuatorFeature";

  int retStatus = 0;
  
  ractuatorFeature = PI_C509_FEATURE;
 
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
int DriverPI_C509::ConvertUnit(int unit, 
                                    float valueToConvert, 
                                    float& rconvertedValue,
                                    float& rrange) const 
{
  QLOG_DEBUG() << "DriverPI_C509::ConvertUnit";

  int retStatus = 0;

  if (unit == CUSTOM_UNIT)
  {
    rrange  = PI_C509_FEATURE.customRange;
    rconvertedValue = valueToConvert; 
  }
  else if (unit == DEFAULT_UNIT)
  {
    rrange  =  PI_C509_FEATURE.range;
    rconvertedValue = valueToConvert; 
  }
  else if (unit == FROM_DEFAULT_TO_CUSTOM)
  {
    rrange = PI_C509_FEATURE.customRange;
    rconvertedValue =  valueToConvert ;
  }    
  else
  {
    retStatus = -1;
  }

  return retStatus;
}
