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
#include "DriverNewPort_NSC200.h"

using namespace DriverDefinition;

const int   DriverNewPort_NSC200::BUFFER_SIZE          = 32;  // buffer size for serial messages
const int   DriverNewPort_NSC200::MAX_DEVICES          = 16;  // maximum number of NewPort_NSC200 controllers
const float DriverNewPort_NSC200::MAX_VEL              = 100000; // maximum vel of NewPort_NSC200 controllers
const int   DriverNewPort_NSC200::NB_ITEM_INIT_SETTING =  2;  // number of items at the actuator (#channel, velocity)
const int   DriverNewPort_NSC200::NB_ITEM_DRV_SETTING  =  1;  // number of items at the driver (axisnumber)
const int   DriverNewPort_NSC200::MAX_TRIES            = 10;  // Number of read tries

const ADActuatorFeature DriverNewPort_NSC200::NEWPORT_NSC200_FEATURE = 
  { "steps","mm",110866,11.,CLOSED_LOOP,NULL };

// 
// Init implementation
//
int DriverNewPort_NSC200::Init(string& rstateData) const 
{
  QLOG_DEBUG() << "DriverNewPort_NSC200::Init";
  int retStatus = 0;
  char buffer[BUFFER_SIZE];
  string command;
  string answerTE;
  int tries = 0;
  //
  // Driver initialization
  // 
  // parse driverSetting string for axis number
  if (sscanf(_setting.c_str(),"axisNumber=%d",
	     &_axisNumber) != NB_ITEM_DRV_SETTING) {
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> driverSetting string incorrect";
    retStatus = -1;
    return retStatus;
  }
  //
  // check for axisNumber validity
  //
  QLOG_DEBUG() << "axis number = " << _axisNumber;
  
  if ((_axisNumber < 1) || (_axisNumber > MAX_DEVICES)) {
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> axis number out of range";
    retStatus = -1;
    return retStatus;
  }  
  
  ////////////////////////////////////////////////////////////////////////////////////
  // Check error of the controller
  ////////////////////////////////////////////////////////////////////////////////////
  while (answerTE == "") {
    usleep(10000);
    memset(buffer,0,BUFFER_SIZE);
    sprintf(buffer, "%dTE?\r", _axisNumber);
    QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> INIT : send message : " << QString(buffer); 
    // send command
    command = buffer;
    if (_pcommChannel->Write(command,NULL) < (int)command.length()) { 
      QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << ": unable to write to port";
    }
    usleep(10000);
    // read reply
    if (!_pcommChannel->Read(answerTE,NULL)) {
      QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << ": unable to read from port";
      QLOG_DEBUG() << "ADActuatorNewPort_NSC200> Error code reply expected, but got reply " 
		   <<  QString(answerTE.c_str());
    }
    //usleep(10000);
    QLOG_DEBUG() << "Error code = " << QString(answerTE.c_str());
    if (answerTE == "") {
	QLOG_DEBUG() << "Trying again...";
	 }	
    tries++;
    if (tries == MAX_TRIES) {
      QLOG_DEBUG() << "Giving Up !";
      retStatus = -1;
      return retStatus;
    }
  }
  return retStatus;
}

//
// InitActuator implementation
//
// use company delivered optimized settings
//
int DriverNewPort_NSC200::InitActuator(string actuatorSetting,
				       float  position) const 
{
  QLOG_DEBUG() << "DriverNewPort_NSC200::InitActuator";
  int retStatus = 0;
  char buffer[BUFFER_SIZE];
  string command = "";
  string answer = "";
  int hwStatusReply;
  string answerTE;
  string sub;
  const char* realAnswer;
  ////////////////////////////////////////////////////////////////////////////////////
  // Select switch box channel NOT NECESSARY IN PRINCIPLE !
  ////////////////////////////////////////////////////////////////////////////////////
  if (!SelectedSwitchBoxChannel(actuatorSetting)) {
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> No motor connected !";
    return (-1);
  }
  if ((_vel < 1 ) || (_vel > MAX_VEL)) {
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> vel setting out of range";
    retStatus = -1;
    return retStatus;
  }
  //
  // send xxVAnn to set the velocity of the controller (applies to each actuator)
  //
  memset(buffer,0,BUFFER_SIZE);
  sprintf(buffer, "%dVA%f\r",_axisNumber,_vel);
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) {
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> Unable to write to port, command = " << QString(command.c_str());
    retStatus = -1;
  }
  usleep(10000);
  /* Save non volatile memory*/
  memset(buffer,0,BUFFER_SIZE);
  sprintf(buffer, "%dSM\r", _axisNumber);
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) {
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> Unable to write to port, command = " << QString(command.c_str());
    retStatus = -1;
  }
  sleep(3);
  ////////////////////////////////////////////////////////////////////////////////////
  // Check led of the controller
  ////////////////////////////////////////////////////////////////////////////////////
  memset(buffer,0,BUFFER_SIZE);
  sprintf(buffer, "%dPH?\r", _axisNumber);
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) { 
    QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << ": unable to write to port";
    retStatus = -1;
  }
  usleep(10000);
  // read reply
  if (!_pcommChannel->Read(answer,NULL)) {
    QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << ": unable to read from port";
    retStatus = -1;
    QLOG_DEBUG() <<
      "ADActuatorNewPort_NSC200> #Channel reply expected, but got reply = " << QString(answer.c_str());
    return retStatus;
  }
  //usleep(10000);
  QLOG_DEBUG() << "ADActuatorNewPort_NSC200> answer = " << QString(answer.c_str());
  // evaluate HW status
  if (answer.length() > 5) 
    sub = answer.substr(5);
  realAnswer = sub.c_str();
  QLOG_DEBUG() << "Reply to PH? = " << QString(realAnswer);
  sscanf (realAnswer, "%d",&hwStatusReply);
  QLOG_DEBUG() << "ADActuatorNewPort_NSC200> #HW status reply = " << hwStatusReply;
  hwStatusReply = hwStatusReply & 1;
  if ( hwStatusReply == 0 ) {
    // Led is red !
    QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << " has red led status !";
    retStatus = -1;
  }
  ////////////////////////////////////////////////////////////////////////////////////
  // Check error of the controller
  ////////////////////////////////////////////////////////////////////////////////////
  memset(buffer,0,BUFFER_SIZE);
  sprintf(buffer, "%dTE?\r", _axisNumber);
  
  QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> send message : " << QString(buffer); 
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) { 
    QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << ": unable to write to port";
    retStatus = -1;
  }
  usleep(10000);
  // read reply
  if (!_pcommChannel->Read(answerTE,NULL)) {
    QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << ": unable to read from port";
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> Error code reply expected, but got reply = " 
		 << QString(answerTE.c_str());
    retStatus = -1;
  }
  //usleep(10000);
  QLOG_DEBUG() << "Error code = "  << QString(answerTE.c_str());
  return retStatus;
}

//
// GetPos implementation
//
// 1. send "xxTP%d?" (tell position) to get the current position
// 2. read reply
// 3. parse position from reply
//
int DriverNewPort_NSC200::GetPos(string  actuatorSetting,
				 float&  position) const 
{
  QLOG_DEBUG() << "DriverNewPort_NSC200::GetPos";
  
  int retStatus = 0;
  string command = "";
  string answer = "";
  char buffer[BUFFER_SIZE];
  // init position to zero
  position = 0;
  
  ////////////////////////////////////////////////////////////////////////////////////
  // Select switch box channel NOT NECESSARY IN PRINCIPLE !
  ////////////////////////////////////////////////////////////////////////////////////
  if (!SelectedSwitchBoxChannel(actuatorSetting)) {
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> No motor connected !";
    return (-1);
  }
  // create buffer with command xxTPnn
  memset(buffer,0,BUFFER_SIZE);
  if (_channel > 0 )
    sprintf(buffer, "%dTP%d?\r", _axisNumber,_channel);
  else
    sprintf(buffer, "%dTP?\r", _axisNumber);
  QLOG_DEBUG() << "ADActuatorNewPort_NSC200> GetPos> send command buffer = " << QString(buffer);
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) {
    QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << ": unable to write to port";
    return (-1);
  }
  usleep(10000);
  // read reply
  if (!_pcommChannel->Read(answer,NULL)) {
    QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << ": unable to read from port";
    return (-1);
  }
  //usleep(10000);
  QLOG_DEBUG() << "ADActuatorNewPort_NSC200> Getting answer = " << QString(answer.c_str());
  // parse and extract position
  string sub ="";
  if (answer.length() > 5) {
    if (_channel > 0 )
      sub = answer.substr(6);
    else
      sub = answer.substr(5);
    const char* realAnswer = sub.c_str();
    QLOG_DEBUG() << "Reply to TP? = " << QString(realAnswer);
    sscanf (realAnswer, "%f",&position);
  }  
  QLOG_DEBUG() << "ADActuatorNewPort_NSC200> GetPos gives position = " <<  position;
  return retStatus;
}

//
// Move implementation
//
// 1. send "xxPRnn" (move relative) to move xxxx steps
//
int DriverNewPort_NSC200::Move(string actuatorSetting,
		    float  nbSteps,
		    int    unit) const 
{
  QLOG_DEBUG() << "DriverNewPort_NSC200::Move";

  int retStatus = 0;
  float range;
  float motion;
  string answer = "";
  string command = "";
  char buffer[BUFFER_SIZE];
 
  ////////////////////////////////////////////////////////////////////////////////////
  // Select switch box channel
  ////////////////////////////////////////////////////////////////////////////////////
  if (!SelectedSwitchBoxChannel(actuatorSetting)) {
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> No motor connected !";
    return (-1);
  }
  if ( ConvertUnit(unit,nbSteps,motion,range) || (fabs(motion) > range)) {
    retStatus  = DISPLACEMENT_EXCEEDS_RANGE;
    _lastError = OUT_OF_RANGE_ERROR;
    return retStatus;
  }
 
  //////////////////////////////////////////////////////////////////////////
  // Move to relative position
  //////////////////////////////////////////////////////////////////////////
  // create buffer with command xxPAnn
  memset(buffer,0,BUFFER_SIZE);
  sprintf(buffer, "%dPR%d\r", _axisNumber,(int)motion);
  QLOG_DEBUG() << "ADActuatorNewPort_NSC200> Send MoveAbs Command " <<  QString(buffer);
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) { 
     QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << ": unable to write to port";
    return (-1);
  }
  usleep(10000);
  return retStatus;
}

//
// MoveAbs implementation
//
// 1. send "xxPAnn" move absolute command to move to position absPos
//
int DriverNewPort_NSC200::MoveAbs(string actuatorSetting,
		       float  absPos,
		       int    unit) const 
{
  QLOG_DEBUG() << "DriverNewPort_NSC200::MoveAbs";
  
  int retStatus = 0;
  float convertedAbsPos;
  float range;
  string answer = "";
  string command = "";
  char buffer[BUFFER_SIZE];

  ////////////////////////////////////////////////////////////////////////////////////
  // Select switch box channel
  ////////////////////////////////////////////////////////////////////////////////////
  if (!SelectedSwitchBoxChannel(actuatorSetting)) {
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> No motor connected !";
    return (-1);
  }
  if (!ConvertUnit(unit,absPos,convertedAbsPos,range)) {
    
    //////////////////////////////////////////////////////////////////////////
    // Move to absolute position
    //////////////////////////////////////////////////////////////////////////
    // create buffer with command xxPAnn
    memset(buffer,0,BUFFER_SIZE);
    sprintf(buffer, "%dPA%d\r", _axisNumber,(int)convertedAbsPos);
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> Send MoveAbs Command " << QString( buffer);
    // send command
    command = buffer;
    if (_pcommChannel->Write(command,NULL) < (int)command.length()) { 
       QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << ": unable to write to port";
      return (-1);
    }
    usleep(10000);
  }
  return retStatus;
}

//
// Stop implementation
//
// 1. send "xxST" (stop smoothly) to stop the
//    current movement
//
int DriverNewPort_NSC200::Stop(string actuatorSetting) const  
{
  QLOG_DEBUG() << "DriverNewPort_NSC200::Stop";

  int retStatus = 0;
  string command = "";
  string answer = "";
  char buffer[BUFFER_SIZE];
 
  ////////////////////////////////////////////////////////////////////////////////////
  // Select switch box channel
  ////////////////////////////////////////////////////////////////////////////////////
  if (!SelectedSwitchBoxChannel(actuatorSetting)) {
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> No motor connected !";
    return (-1);
  }
  memset(buffer,0,BUFFER_SIZE);
  sprintf(buffer, "%dST\r", _axisNumber);
  QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> Stop>send message : " << QString(buffer); 
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) { 
     QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << ": unable to write to port";
    retStatus = -1;
  }
  usleep(10000);
  return retStatus;
}

//
// OperationComplete implementation
//
// 1. send "xxTS?" to retrieve the status of the controller
// 2. get the reply
// 3. parse the reply
// 4. compare the bits
//
//
int DriverNewPort_NSC200::OperationComplete(string& rstateData,
				 string actuatorSetting,
				 ADLimitSwitch& rlimitSwitch) const
{
  QLOG_DEBUG() << "DriverNewPort_NSC200::OperationComplete";
  
  const int PARAMETER_OUT_OF_RANGE = 7;
  const int UPPER_LIMIT_STATE = 26;
  const int LOWER_LIMIT_STATE = 27;
  const int MOTOR_NOT_ENABLE_STATE = 213;
  const int PARAMETER_CHANGE_STATE = 226;
  const int COMMAND_PARAMETER_MISSING_STATE = 38;
  const int NO_MOTOR_CONNECTED_STATE = 8;
  const int PARAMETER_OUT_OF_RANGE_STATE = 7;
  const int INVALID_COMMAND_STATE = 6;
  const int DRIVER_FAULT_OPEN_LOAD_STATE = 1;
  const int DRIVER_FAULT_THERMAL_STATE = 2;
  const int DRIVER_FAULT_SHORT_STATE = 3;
  const int NO_ERRORS_STATE = 0;
  const int MOTOR_ON_RUNNING_STATE = 80;
  const int MOTOR_ON_STOP_STATE = 81;
  const int MOTOR_OFF_STOP_STATE = 65;
  
  int          retStatus = -1;
  string       command = "";
  string       answerTS = "";
  string       answerTE = "";
  char         buffer[BUFFER_SIZE];
  int          status;
  int          error;
  stringstream axisStr;
  string       subTS = "";
  string       subTE = "";
  const char*  realAnswerTE;
  const char*  realAnswerTS;
  // parse driverSetting string for axis number
  if (sscanf(_setting.c_str(),"axisNumber=%d",
	     &_axisNumber) != NB_ITEM_DRV_SETTING) {
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> driverSetting string incorrect";
    retStatus = -1;
    return retStatus;
  }
  // parse actuator configuration parameters
  if (sscanf(actuatorSetting.c_str(),
	     "channel=%d vel=%f",
	     &_channel,&_vel) != NB_ITEM_INIT_SETTING) {
    _channel = -1 ;/*init case*/
  }
  ////////////////////////////////////////////////////////////////////////////////////////
  // create buffer with command xxTS? for controller Status query
  ////////////////////////////////////////////////////////////////////////////////////////
  memset(buffer,0,BUFFER_SIZE);
  sprintf(buffer, "%dTS?\r", _axisNumber);
  QLOG_DEBUG() << "ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << " channel " << _channel
	       << " buffer = " << QString(buffer);
  
  QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> send message : " <<  QString(buffer); 
  // send command
  command = buffer;
  if (_pcommChannel->Write(command,NULL) < (int)command.length()) { 
    QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << ": unable to write to port";
    return  (-1);
  }
  usleep(10000);
  // read reply
  if (!_pcommChannel->Read(answerTS,NULL)) {
    QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << ": unable to read from port";
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> Status reply expected, but got reply = " << 
      QString(answerTS.c_str());
    return  (-1);
  }
  //usleep(10000);
  QLOG_DEBUG() << "Status reply = " <<  QString(answerTS.c_str());
  
  // Evaluate status of motion
  
  axisStr << _axisNumber;
  if (answerTS.length() > 5) {
    subTS = answerTS.substr(5);
    realAnswerTS = subTS.c_str();
    QLOG_DEBUG() << "Reply to TS? = "  << QString(realAnswerTS);
    sscanf (realAnswerTS, "%d",&status);
  }
  else
    status = -1;
  QLOG_DEBUG() << "axisNumber " << _axisNumber << " channel " << _channel << " has status "
	       << status;
  if ( status == MOTOR_ON_STOP_STATE ) {
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> axisNumber " <<  QString(axisStr.str().c_str())
		 << ": Motor On, Motion not in progress";
    retStatus = 1;
    rstateData = "axisNumber=" + axisStr.str() + ": Motor On, Motion not in progress";
  }
  else if ( status == MOTOR_ON_RUNNING_STATE ) {
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> axisNumber " <<  QString(axisStr.str().c_str())
		 << ": Motor On, Motion in progress";
    rstateData = "axisNumber=" + axisStr.str() + ": Motor On, Motion in progress";
    retStatus = 0;
  }
  else if ( status == MOTOR_OFF_STOP_STATE ) {
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> axisNumber " <<  QString(axisStr.str().c_str())
		 << ": Motor Off, Motion not in progress";
    
    rstateData = "axisNumber=" + axisStr.str() + ": Motor Off, Motion not in progress";
    retStatus = 1;
  }
  usleep(10000);
  
  ///////////////////////////////////////////////////////////////////////////////
    // create buffer with command xxTE? for Motor  Error Code query from controller
    ///////////////////////////////////////////////////////////////////////////////
    
    memset(buffer,0,BUFFER_SIZE);
    sprintf(buffer, "%dTE?\r", _axisNumber);
  
    QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> send message : "  <<  QString(buffer); 
    // send command
    command = buffer;
    if (_pcommChannel->Write(command,NULL) < (int)command.length()) { 
      QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << ": unable to write to port";
      return  (-1);
    }
    usleep(10000);
    // read reply
    if (!_pcommChannel->Read(answerTE,NULL)) {
      QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << ": unable to read from port";
      QLOG_DEBUG() << "ADActuatorNewPort_NSC200> Error code reply expected, but got reply = " 
		   << QString(answerTE.c_str());
      return  (-1);
    }
    //usleep(10000);
    QLOG_DEBUG() << "Error code = " << QString( answerTE.c_str());
    // Evaluate controller Error code
    if (answerTE.length() > 5) {
      subTE = answerTE.substr(5);
      realAnswerTE = subTE.c_str();
      QLOG_DEBUG() << "Reply to TE? = " << QString(realAnswerTE);
      sscanf (realAnswerTE, "%d",&error);
    }
    else
      error = -1;
    QLOG_DEBUG() << "axisNumber " << _axisNumber << " channel " << _channel << " has error code " << error;
    //
    // Motor Error Code checking
    //
    if ( error == MOTOR_NOT_ENABLE_STATE ) {
      QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> Motor not enabled !";
    }
    //else if ( error == PARAMETER_CHANGE_STATE ) {
    //  QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> Change parameter not allowed during motion !");
    // }
    else if ( error == COMMAND_PARAMETER_MISSING_STATE ) {
      QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> Command parameter missing !";
    }
    else if ( error == NO_MOTOR_CONNECTED_STATE ) {
      QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> Motor not connected !";
      rstateData = "Motor not connected !\n";
      retStatus = -1;
    }
    else if ( error == PARAMETER_OUT_OF_RANGE_STATE ) {
      QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> Parameter out of range";
    }
    else if ( error == INVALID_COMMAND_STATE ) {
      QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> Invalid Command !";
    }
    else if ( error == DRIVER_FAULT_OPEN_LOAD_STATE ) {
      QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> Driver Error : Open Load !";
      rstateData = "Driver Error : Open Load !";
      retStatus = -1;
    }
    else if ( error == DRIVER_FAULT_THERMAL_STATE ) {
      QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> Driver Error : Thermal shut down !";
      rstateData = "Driver Error : Thermal shut down !";
      retStatus = -1;
    }
    else if ( error == DRIVER_FAULT_SHORT_STATE ) {
      QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> Driver Error : Short !";
      rstateData = "Driver Error : Short !";
      retStatus = -1;
    }
    else if ( error == NO_ERRORS_STATE) {
      QLOG_DEBUG() << "ADActuatorNewPort_NSC200> No Errors !";
    }
    else if ( error ==  PARAMETER_OUT_OF_RANGE) {
      QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> Parameter out of range !";
    }
    // Switch limit reached checking
    if (error == UPPER_LIMIT_STATE) {
      QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> Hardware Upper Limit reached !";
      rlimitSwitch = UPPER_LIMIT;
    }
    else if (error == LOWER_LIMIT_STATE) {
      QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> Hardware Lower Limit reached !";
      rlimitSwitch = LOWER_LIMIT;
    }
    else {
      rlimitSwitch = IN_BETWEEN;
    }
    return retStatus;
}

//
// GetActuatorFeature implementation
//
//    Gives the translation stages features
//
int DriverNewPort_NSC200::GetActuatorFeature(ADActuatorFeature& ractuatorFeature) const
{
  QLOG_DEBUG() << "DriverNewPort_NSC200::GetActuatorFeature";
  
  int retStatus = 0;
  
  ractuatorFeature = NEWPORT_NSC200_FEATURE;
  
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
int DriverNewPort_NSC200::ConvertUnit(int unit, 
			   float valueToConvert, 
			   float& rconvertedValue,
			   float& rrange) const 
{
  QLOG_DEBUG() << "DriverNewPort_NSC200::ConvertUnit";

  int retStatus = 0;
  const float STEPS_FOR_ONE_MM = 10095;

  if (unit == CUSTOM_UNIT)
    {
      rrange  = NEWPORT_NSC200_FEATURE.customRange * STEPS_FOR_ONE_MM;
      rconvertedValue = valueToConvert * STEPS_FOR_ONE_MM; 
    }
  else if (unit == DEFAULT_UNIT)
    {
      rrange  = NEWPORT_NSC200_FEATURE.range;
      rconvertedValue = valueToConvert; 
    }
  else if (unit == FROM_DEFAULT_TO_CUSTOM)
    {
      rrange = NEWPORT_NSC200_FEATURE.customRange;
      rconvertedValue =  valueToConvert / STEPS_FOR_ONE_MM;
    }    
  else
    {
      retStatus = -1;
    }
  
  return retStatus;
}

int DriverNewPort_NSC200::SelectedSwitchBoxChannel(string actuatorSetting) const
{
  QLOG_DEBUG() << "DriverNewPort_NSC200::SelectedSwitchBoxChannel";
  int retStatus = 1;
  string answer = "";
  string command = "";
  int channelReply;
  string sub = "";
  const char* realAnswer;
  char buffer[BUFFER_SIZE];

  /* init lastError*/
  _lastError = NO_ERROR;
  // parse driverSetting string for axis number
  if (sscanf(_setting.c_str(),"axisNumber=%d",
	     &_axisNumber) != NB_ITEM_DRV_SETTING) {
    QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> driverSetting string incorrect";
    retStatus = -1;
    return retStatus;
  }
  QLOG_DEBUG() << "SelectedSwitchBoxChannel> axisNumber " << _axisNumber;
  
  // parse actuator configuration parameters
  if (sscanf(actuatorSetting.c_str(),
	     "channel=%d vel=%f",
	     &_channel,&_vel) != NB_ITEM_INIT_SETTING) {
    QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> Actuator setting string format incorrect";
    retStatus = -1;
    return retStatus;
  }
  if (_channel > 0 ) { // motor attached to switch box
    memset(buffer,0,BUFFER_SIZE);
    sprintf(buffer, "%dMX%d\r", _axisNumber,_channel);
    // send command
    command = buffer;
    if (_pcommChannel->Write(command,NULL) < (int)command.length()) { 
       QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << ": unable to write to port";
      retStatus = 0;
    }
    usleep(10000);
    ////////////////////////////////////////////////////////////////////////////////////
    // Check that selected switch box channel is correct
    ////////////////////////////////////////////////////////////////////////////////////
    memset(buffer,0,BUFFER_SIZE);
    sprintf(buffer, "%dMX?\r", _axisNumber);
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << " buffer " << QString(buffer);
    // send command
    command = buffer;
    if (_pcommChannel->Write(command,NULL) < (int)command.length()) { 
       QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << ": unable to write to port";
      retStatus = 0;
    }
    usleep(10000);
    // read reply
    if (!_pcommChannel->Read(answer,NULL)) {
      QLOG_DEBUG() <<"ADActuatorNewPort_NSC200> axisNumber " << _axisNumber << ": unable to read from port";
      retStatus = 0;
      QLOG_DEBUG() << "ADActuatorNewPort_NSC200> #Channel reply expected, but got reply = " 
		   << QString(answer.c_str());
      return retStatus;
    }
//usleep(10000);
QLOG_DEBUG() << "ADActuatorNewPort_NSC200> answer = " << QString(answer.c_str());
    // Evaluate controller #Channel
    if (answer.length() > 5) 
      sub = answer.substr(5);
    realAnswer = sub.c_str();
    QLOG_DEBUG() << "Reply to MX? = " << QString(realAnswer);
    sscanf (realAnswer, "%d",&channelReply);
    QLOG_DEBUG() << "ADActuatorNewPort_NSC200> #Channel reply = " << channelReply;
    if (channelReply != _channel)
      retStatus = 0;
  }

  return (retStatus); 
}
