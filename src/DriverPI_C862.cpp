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
 * @file DriverPI_C862.cpp
 * @brief Driver implementation for PI C 862.
 */

#include "DriverPI_C862.h"

using namespace DriverDefinition;

const int DriverPI_C862::BUFFER_SIZE          = 32; // buffer size for serial messages
const int DriverPI_C862::MAX_DEVICES          = 16; // maximum number of Mercury controllers
const int DriverPI_C862::NB_ITEM_INIT_SETTING =  6; // number of items at the actuator
                                                          // setting string
const int DriverPI_C862::NB_ITEM_DRV_SETTING  =  1; // number of items at the driver
                                                          //setting string

const DriverFeature DriverPI_C862::PI_C862_FEATURE = 
      { "steps","mm",3.0e6,25.,CLOSED_LOOP,NULL };

// 
// Init implementation
//
int DriverPI_C862::Init(string& rstateData) const 
{
  QLOG_DEBUG() << "DriverPI_C862::Init";

  int retStatus = 0;
  ADLimitSwitch limitSwitch;

  if (OperationComplete(rstateData, "NO_PI_C862", limitSwitch) < 0)
  {
    retStatus = -1;
  }
  QLOG_DEBUG() << "DriverPI_C862::Init status = " << retStatus;
  return retStatus;
}

//
// InitActuator implementation
//
// 1. parse configuration parameters from actuatorSetting string
// 2. send the controller's address code
// 3. send "DPxxx<CR>" to define the proportional gain
// 4. send "DIxxx<CR>" to define the integral gain
// 5. send "DDxxx<CR>" to define the derivative gain
// 6. send "DLxxx<CR>" to define the integral limit
// 7. send "SAxxx<CR>" to set the acceleration
// 8. send "SVxxx<CR>" to set the velocity
// 9. send "MN<CR>" to turn the motor on
//
int DriverPI_C862::InitActuator(string actuatorSetting,
                                      float  position) const 
{
  QLOG_DEBUG() << "DriverPI_C862::InitActuator";

  int retStatus = 0;
  string command = "";
  char buffer[BUFFER_SIZE];
  int pidP = 0;
  int pidI = 0;
  int pidD = 0;
  int integralLimit = 0;
  int acc = 0;
  int vel = 0;

  // parse configuration parameters
  //   pidP=xxx pidI=xxx pidD=xxx integralLimit=xxx acc=xxx vel=xxx
  if (sscanf(actuatorSetting.c_str(),
	     "pidP=%d pidI=%d pidD=%d integralLimit=%d acc=%d vel=%d",
	     &pidP,
	     &pidI,
	     &pidD,
	     &integralLimit,
	     &acc, 
	     &vel) != NB_ITEM_INIT_SETTING) {
    ReportSettingError("Actuator setting string format incorrect");
    retStatus = -1;
    return retStatus;
  }

  // first send the address code
  if (SendAddressCode() != 0) {
    QLOG_DEBUG() << "Could not send address code";
    retStatus = -1;
    return retStatus;
  }

  // send DPxxx<CR> to define the proportional gain
  sprintf(buffer, "DP%d\r", pidP);
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) {
    ReportCommError(QString("Unable to write to port, command %1")
                    .arg(QString(command.c_str())));
    retStatus = -1;
    return retStatus;
  }

  // send DIxxx<CR> to define the integral gain
  sprintf(buffer, "DI%d\r", pidI);
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) {
    ReportCommError(QString("Unable to write to port, command %1")
                    .arg(QString(command.c_str())));
    retStatus = -1;
    return retStatus;
  }

  // send DDxxx<CR> to define the derivative gain
  sprintf(buffer, "DD%d\r", pidD);
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) {
    ReportCommError(QString("Unable to write to port, command %1")
                    .arg(QString(command.c_str())));
    retStatus = -1;
    return retStatus;
  }

  // send DLxxx<CR> to define the integral limit
  sprintf(buffer, "DL%d\r", integralLimit);
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) {
    ReportCommError(QString("Unable to write to port, command %1")
                    .arg(QString(command.c_str())));
    retStatus = -1;
    return retStatus;
  }

  // send SAxxx<CR> to set the acceleration
  sprintf(buffer, "SA%d\r", acc);
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) {
    ReportCommError(QString("Unable to write to port, command %1")
                    .arg(QString(command.c_str())));
    retStatus = -1;
    return retStatus;
  }

  // send SVxxx<CR> to set the velocity
  sprintf(buffer, "SV%d\r", vel);
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) {
    ReportCommError(QString("Unable to write to port, command %1")
                    .arg(QString(command.c_str())));
    retStatus = -1;
    return retStatus;
  }

  // create buffer with command MN<CR> to turn the motor on
  sprintf(buffer, "MN\r");
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) { 
    ReportCommError(QString("Unable to write to port, command %1")
                    .arg(QString(command.c_str())));
    retStatus = -1;
    return retStatus;
  }

  // computation of the offset to apply to get a position
  float readPosition;
  _originOffset = 0.;
  GetPos(actuatorSetting,readPosition);
  _originOffset = position - readPosition;

  return retStatus;
}

//
// GetPos implementation
//
// 1. send "TP<CR>" (tell position) to get the current position
// 2. read reply
// 3. parse position from reply
//
int DriverPI_C862::GetPos(string  actuatorSetting,
				float&  position) const 
{
  QLOG_DEBUG() << "DriverPI_C862::GetPos";

  int retStatus = 0;

  string command = "";
  string answer = "";
  char buffer[BUFFER_SIZE];

  // init position to zero
  position = 0;
    
  // create buffer with command TP<CR>
  sprintf(buffer, "TP\r");
    
  // first send the address code
  if (SendAddressCode() != 0) {
    QLOG_DEBUG() << "Could not send address code";
    retStatus = -1;
    return retStatus;
  }
    
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) {
    ReportCommError(QString("Unable to write to port, command %1")
                    .arg(QString(command.c_str())));
    retStatus = -1;
    return retStatus;
  }

  // read reply
  if (!_pcommChannel->Read(answer,NULL)) {
    ReportCommError("Unable to read from port");
    retStatus = -1;
    return retStatus;
  }

  // check that reply format is correct
  // format should be: P:+/-0000000000
  if (answer.find("P:") == string::npos) {
    QLOG_DEBUG() << "Position reply expected, but got reply = " << QString(answer.c_str());
    retStatus = -1;
    return retStatus;
  }
    
  // parse and extract position
  // note: due to eventual special characters at the beginning we
  //       use strchr to point to the beginning of the information
  char* realAnswer = strchr((char*)answer.c_str(),'P');
  sscanf (realAnswer, "P:%f", &position);

  position += _originOffset;

  return retStatus;
}

//
// Move implementation
//
// 1. send "MRxxxx<CR>" (move relative) to move xxxx steps
//
int DriverPI_C862::Move(string actuatorSetting,
			      float  nbSteps,
                              int    unit) const 
{
  QLOG_DEBUG() << "DriverPI_C862::Move";

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
    // create buffer with command MRxxxx<CR>
    sprintf(buffer, "MR%.0f\r", motion);
   
    // first send the address code
    if (SendAddressCode() != 0) {
      QLOG_DEBUG() << "Could not send address code";
      retStatus = -1;
      return retStatus;
    }
      
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
// 1. send "MAxxxx<CR>" (move absolute) to move to position absPos
//
int DriverPI_C862::MoveAbs(string actuatorSetting,
				 float  absPos,
                                 int    unit) const 
{
  QLOG_DEBUG() << "DriverPI_C862::MoveAbs";

  int retStatus = -1;
  float convertedAbsPos;
  float range;
  string command = "";
  char buffer[BUFFER_SIZE];

  if (!ConvertUnit(unit,absPos,convertedAbsPos,range))
  {
    convertedAbsPos -= _originOffset;
    // create buffer with command MAxxxx<CR>
    sprintf(buffer, "MA%.0f\r", convertedAbsPos);
   
    // first send the address code
    if (SendAddressCode() != 0) {
      QLOG_DEBUG() << "Could not send address code";
      return retStatus;
    }
       
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
// 1. send "ST,AB<CR>" (stop smoothly) to stop the
//    current movement
//
int DriverPI_C862::Stop(string actuatorSetting) const 
{
  QLOG_DEBUG() << "DriverPI_C862::Stop";

  int retStatus = 0;

  string command = "";
  char buffer[BUFFER_SIZE];
    
  // create buffer with command ST,AB<CR>
  sprintf(buffer, "ST,AB\r");
    
  // first send the address code
  if (SendAddressCode() != 0) {
    QLOG_DEBUG() << "Could not send address code";
    retStatus = -1;
    return retStatus;
  }
      
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
// 1. send "TS<CR>" to retrieve the status
// 2. get the reply
// 3. parse the reply
// 4. compare the bits
//
// Note: the reply for the status has the following format:
//    S: 1F 2F 3F 4F 5F 6F
//    See the Mercury controller documentation for details.
//    Here we use:
//      1F bit 2: trajectory complete
//    If this bit is set we return 1 (complete), otherwise we
//    return 0 (busy).
//
int DriverPI_C862::OperationComplete(string& rstateData,
                                           string actuatorSetting,
                                           ADLimitSwitch& rlimitSwitch) const
{
  QLOG_DEBUG() << "DriverPI_C862::OperationComplete";

  const unsigned int UPPER_LIMIT_STATE = 0x04;
  const unsigned int LOWER_LIMIT_STATE = 0x08;

  int retStatus = -1;

  string       command = "";
  string       answer = "";
  char         buffer[BUFFER_SIZE];
  unsigned int status = 0;
  unsigned int switchStatus = 0;
  
  // create buffer with command TS<CR>
  sprintf(buffer, "TS\r");
    
  // first send the address code
  if (SendAddressCode() != 0) {
    QLOG_DEBUG() << "Could not send address code";
    retStatus = -1;
    return retStatus;
  }
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

  // check that reply format is correct
  // format should be: S:1F 2F 3F 4F 5F 6F
  rstateData = answer;
  if (answer.find("S:") == string::npos) {
    QLOG_DEBUG() << "Status reply expected, but got reply " << QString(answer.c_str());
    retStatus = -1;
    return retStatus;
  }
  else 
  {
    QLOG_DEBUG() << "Status reply " << QString(answer.c_str());
  }
    
  // parse and extract status
  // note: due to eventual special characters at the beginning we
  //       use strchr to point to the beginning of the information
  char* realAnswer       = strchr((char*)answer.c_str(),'S');
  const int NB_ITEM_READ = 2;
  if (sscanf (realAnswer,"S:%X %*X %*X %*X %X %*X", 
	      &status, &switchStatus) != NB_ITEM_READ){
    QLOG_DEBUG() << "Status could not be parsed, reply string " <<  QString(answer.c_str());
    retStatus = -1;
    return retStatus;
  }

  // check if bit 2 (trajectory complete flag) is set
  if ((status & 4) == 4) {
    // trajectory complete, so return 1 (complete)
    retStatus = 1;
  } else {
    // trajectory not complete, so return 0 (busy)
    retStatus = 0;
  }

  // Switch limit reached checking
  if ((switchStatus & UPPER_LIMIT_STATE)  && 
      (switchStatus & LOWER_LIMIT_STATE))
    {
      rlimitSwitch = UNAVAILABLE;
    }
  if (switchStatus & UPPER_LIMIT_STATE)
  {
    rlimitSwitch = UPPER_LIMIT;
  }
  else if (switchStatus & LOWER_LIMIT_STATE)
  {
    rlimitSwitch = LOWER_LIMIT;
  }
  else
  {
    rlimitSwitch = IN_BETWEEN;
  }
  QLOG_DEBUG () << " rlimitSwitch = " << rlimitSwitch;
  return retStatus;
}

//
// GetActuatorFeature implementation
//
//    Gives the translation stages features
//
int DriverPI_C862::GetActuatorFeature(DriverFeature& ractuatorFeature) const
{
  QLOG_DEBUG() << "DriverPI_C862::GetActuatorFeature";

  int retStatus = 0;
  
  ractuatorFeature = PI_C862_FEATURE;
 
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
int DriverPI_C862::SendAddressCode() const
{
  QLOG_DEBUG() << "DriverPI_C862::SendAddressCode";

  int retStatus = 0;
  int deviceNumber = 0;
  string command = "";
  char buffer[BUFFER_SIZE];

  // parse driverSetting string for device number
  if (sscanf(_setting.c_str(),"deviceNumber=%d",
	     &deviceNumber) != NB_ITEM_DRV_SETTING) {
    ReportSettingError("driverSetting string incorrect");
    retStatus = -1;
    return retStatus;
  }

  // check for range
  if ((deviceNumber < 1) || (deviceNumber > MAX_DEVICES)) {
    ReportRangeError("driverNumber out of range");
    retStatus = -1;
    return retStatus;
  }
    
  // create device address code:
  //   1st character: 0x01
  //   2nd character: 0x30 for device #1... 0x46 for device #16
  buffer[0] = 0x01;
  buffer[1] = 0x30 + (deviceNumber - 1);
  buffer[2] = '\0';
  QLOG_DEBUG() << "SendAddressCode> current address " << QString(buffer);
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length())
  {
    ReportCommError("Unable to write to port");
    retStatus = -1;
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
/// @param rconvertedValue
///    conversion result
/// @param rrange
///    output range for the selected unit
// -----------------------------------------------------------------------------
int DriverPI_C862::ConvertUnit(int unit, 
                                     float valueToConvert, 
                                     float& rconvertedValue,
                                     float& rrange) const 
{
  QLOG_DEBUG() << "DriverPI_C862::ConvertUnit";

  unit = 0;
  int retStatus = 0;
  const float STEPS_FOR_ONE_MM = 16690;

  if (unit == CUSTOM_UNIT)
  {
    rrange  = PI_C862_FEATURE.customRange * STEPS_FOR_ONE_MM;
    rconvertedValue = valueToConvert * STEPS_FOR_ONE_MM; 
  }
  else if (unit == DEFAULT_UNIT)
  {
    rrange  = PI_C862_FEATURE.range;
    rconvertedValue = valueToConvert; 
  }
  else if (unit == FROM_DEFAULT_TO_CUSTOM)
  {
    rrange = PI_C862_FEATURE.customRange;
    rconvertedValue =  valueToConvert / STEPS_FOR_ONE_MM;
  }    
  else
  {
    retStatus = -1;
  }

  return retStatus;
}
