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
 * @file DriverPI_E725.cpp
 * @brief Driver implementation for PI E 725.
 */

#include "DriverPI_E725.h"

using namespace DriverDefinition;

const int   DriverPI_E725::NB_ITEM_DRV_SETTING  =  1;  // number of items at the driver (delay)
const int   DriverPI_E725::NB_ITEM_INIT_SETTING = 1;  // number of items at the actuator (#channel address)
const DriverFeature DriverPI_E725::PI_E725_FEATURE = 
  { "step","step",0,0,CLOSED_LOOP,NULL};

// -----------------------------------------------------------------------------
/// Operation : Init
///    Initializes the driver by putting it in online mode to perform remote 
///    control
/// @param rstateData
///   state data  of the driver
// -----------------------------------------------------------------------------
int DriverPI_E725::Init(string& rstateData) const 
{
  QLOG_DEBUG() << "DriverPI_E725::Init";

  int retStatus = 0;
  string commandLine;
  string answer;

  if (sscanf(_setting.c_str(),"delay=%d",
             &_delay) != NB_ITEM_DRV_SETTING) {
    ReportSettingError("DriverPI_E725::Init> driverSetting string incorrect");
    retStatus = -1;
    return retStatus;
  }
  commandLine = "\n*IDN?\n";
  if (_pcommChannel->Write(commandLine,NULL) < (int)commandLine.length())
  {
    ReportCommError("Unable to write to port");
    retStatus = -1;
  }
  else if (!_pcommChannel->Read(answer,NULL))
  {
    ReportCommError("Unable to read from port");
    retStatus = -1;
  }
  else
  {
    QLOG_INFO() << "Controller :" << answer.c_str();
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
///   required position at initialization (useless for PI_E725
///
// -----------------------------------------------------------------------------
int DriverPI_E725::InitActuator(string actuatorSetting,
                                float  position) const 
{
  QLOG_DEBUG() << "DriverPI_E725::InitActuator";

  int retStatus = 0;
  string commandLine, answer;
  char axisNumber[8];

  if (sscanf(actuatorSetting.c_str(),"axisNumber=%s",(char*)&axisNumber) != NB_ITEM_INIT_SETTING)
  {
    ReportSettingError("DriverPI_E725::InitActuator> Bad actuator setting");
  }
  usleep(_delay);
  commandLine = "\nSVO " + string(axisNumber) + " 1\n";

  if (_pcommChannel->Write(commandLine,NULL) < (int)commandLine.length())
  {
    ReportCommError(QString("DriverPI_E725::InitActuator> Cannot put %1 in ServoMode").arg(axisNumber));
    retStatus = -1;
  }
  GetPos(actuatorSetting,position);
  QLOG_INFO () << " Actuator " << axisNumber << " position " << QString::number(position); 

  return retStatus;
}

// -----------------------------------------------------------------------------
/// Operation : GetPos
///    Returns the voltage applied on the selected PI_E725 actuator
//
/// @param actuatorSetting
///    actuator setting
/// @param position
///    Position of the actuator
// -----------------------------------------------------------------------------
int DriverPI_E725::GetPos(string  actuatorSetting,
                          float&  position) const 
{
  QLOG_DEBUG() << "DriverPI_E725::GetPos";

  int retStatus = 0;
  string answer;
  string commandLine;
  char axisNumber[8];
  int _delay;

  if (sscanf(actuatorSetting.c_str(),"axisNumber=%s",(char*)&axisNumber) != NB_ITEM_INIT_SETTING)
  {
    ReportSettingError("DriverPI_E725::GetPos> Bad actuator setting");
  }
  usleep(_delay);
  commandLine = "\nPOS? " + string(axisNumber)  + "\n";

  if (_pcommChannel->Write(commandLine,NULL) < (int)commandLine.length())
  {
    ReportCommError("DriverPI_E725::GetPos> Unable to write to port");
    retStatus = -1;
  }
  if (!_pcommChannel->Read(answer,NULL))
  {
    ReportCommError("DriverPI_E725::GetPos> Unable to read from port");
    retStatus = -1;
  }
  else 
  {
   QLOG_DEBUG() << "Position reply " << answer.c_str();
   vector<string>  tokens;
   Tokenize(answer,tokens,"=");
   if (tokens.size() > 0 && tokens.at(0) != axisNumber) {
    ReportWarning(OTHER_ERROR,
                  QString("DriverPI_E725::GetPos> Position reply expected, but got reply = %1")
                  .arg(answer.c_str()));
    retStatus = -1;
    return retStatus;
   }
   QString positionStr = QString::fromStdString(tokens.at(1));
   QLOG_DEBUG () << " token Position" << positionStr;
   position = positionStr.toFloat();
   QLOG_DEBUG () << " Position " << QString::number(position);
  }
  return retStatus;
}

// -----------------------------------------------------------------------------
/// Operation : Move
///    Moves the selected PI_E725 actuator by changing the voltage applied to it 
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
int DriverPI_E725::Move(string actuatorSetting,
                        float  nbSteps,
                        int    unit) const 
{
  QLOG_DEBUG() << "DriverPI_E725::Move";

  int retStatus = 0;
  string commandLine;
  ostringstream oss;
  char axisNumber[8];
  int _delay;

  if (sscanf(actuatorSetting.c_str(),"axisNumber=%s",(char*)&axisNumber) != NB_ITEM_INIT_SETTING)
  {
    ReportSettingError("DriverPI_E725::Move> Bad actuator setting");
  }
  usleep(_delay);
  oss << "\nMVR " << string(axisNumber) << " " << nbSteps; 
  commandLine = oss.str() + "\n";
  QLOG_DEBUG () << "commandLine " << commandLine.c_str(); 
  if (_pcommChannel->Write(commandLine,NULL) < (int)commandLine.length())
  {
    ReportCommError("DriverPI_E725::Move> Unable to write to port");
    retStatus = -1;
  }
  return retStatus;
}

// -----------------------------------------------------------------------------
/// Operation : MoveAbs
///    Moves the selected PI_E725 actuator by putting the voltage applied to it 
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
int DriverPI_E725::MoveAbs(string actuatorSetting,
                           float  absPos,
                           int    unit) const 
{
  QLOG_DEBUG() << "DriverPI_E725::MoveAbs";

  int retStatus = -1;
  string commandLine;
  ostringstream oss;
  char axisNumber[8];
  int _delay;

  if (sscanf(actuatorSetting.c_str(),"axisNumber=%s",(char*)&axisNumber) != NB_ITEM_INIT_SETTING)
  {
    ReportSettingError("DriverPI_E725::MoveAbs> Bad actuator setting");
  }
  usleep(_delay);
  oss << "\nMOV " << string(axisNumber) << " " << absPos;
  commandLine = oss.str() + "\n";
  QLOG_DEBUG () << "commandLine " << commandLine.c_str();
  if (_pcommChannel->Write(commandLine,NULL) < (int)commandLine.length())
  {
    ReportCommError("DriverPI_E725::MoveAbs> Unable to write to port");
    retStatus = -1;
  }
  return retStatus;
}

// -----------------------------------------------------------------------------
/// Operation : Stop
///    Stops the motion of the selected PI_E725 actuator
//
/// @param actuatorSetting
///    actuator setting
// -----------------------------------------------------------------------------
int DriverPI_E725::Stop(string actuatorSetting) const 
{
  QLOG_DEBUG() << "DriverPI_E725::Stop";

  int retStatus = 0;
  string commandLine;
  char axisNumber[8];
  int _delay;

  if (sscanf(actuatorSetting.c_str(),"axisNumber=%s",(char*)&axisNumber) != NB_ITEM_INIT_SETTING)
  {
    ReportSettingError("DriverPI_E725::Stop> Bad actuator setting");
  }
  usleep(_delay);
  commandLine = "\nSTP\n";

  if (_pcommChannel->Write(commandLine,NULL) < (int)commandLine.length())
  {
    ReportCommError("DriverPI_E725::Stop> Unable to write to port");
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
int DriverPI_E725::OperationComplete(string& rstateData,
                                     string actuatorSetting,
                                     ADLimitSwitch& rlimitSwitch) const 
{
  QLOG_DEBUG() << "DriverPI_E725::OperationComplete";

  int retStatus = -1;
  string   answer;
  string commandLine;
  char axisNumber[8];
  int _delay;

  if (sscanf(actuatorSetting.c_str(),"axisNumber=%s",(char*)&axisNumber) != NB_ITEM_INIT_SETTING)
  {
    ReportSettingError("DriverPI_E725::OperationComplete> Bad actuator setting");
  }
  usleep(_delay);
  commandLine = "\n#5\n";

  if (_pcommChannel->Write(commandLine,NULL) < (int)commandLine.length())
  {
    ReportCommError("DriverPI_E725::OperationComplete> Unable to write to port");
    retStatus = -1;
  }
  else if (!_pcommChannel->Read(answer,NULL))
  {
    ReportCommError("DriverPI_E725::OperationComplete> Unable to read from port");
    retStatus = -1;
  }
  else 
  {
   QLOG_DEBUG () << "OperationComplete> " << answer.c_str();
   if (answer.find("0") == string::npos)  // motion not completed
     retStatus = 0;
   else                                   // motion completed
     retStatus = 1;
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
int DriverPI_E725::GetActuatorFeature(DriverFeature& ractuatorFeature) const
{
  QLOG_DEBUG() << "DriverPI_E725::GetActuatorFeature";

  int retStatus = 0;
  
  ractuatorFeature = PI_E725_FEATURE;
 
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
/// @param rconvertedValue
///    conversion result
/// @param rrange
///    output range for the selected unit
// -----------------------------------------------------------------------------
int DriverPI_E725::ConvertUnit(int unit, 
                                    float valueToConvert, 
                                    float& rconvertedValue,
                                    float& rrange) const 
{
  QLOG_DEBUG() << "DriverPI_E725::ConvertUnit";

  int retStatus = 0;

  if (unit == CUSTOM_UNIT)
  {
    rrange  = PI_E725_FEATURE.customRange;
    rconvertedValue = valueToConvert; 
  }
  else if (unit == DEFAULT_UNIT)
  {
    rrange  =  PI_E725_FEATURE.range;
    rconvertedValue = valueToConvert; 
  }
  else if (unit == FROM_DEFAULT_TO_CUSTOM)
  {
    rrange = PI_E725_FEATURE.customRange;
    rconvertedValue =  valueToConvert ;
  }    
  else
  {
    retStatus = -1;
  }

  return retStatus;
}
