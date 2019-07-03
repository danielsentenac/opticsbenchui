/// ----------------------------------------------------------------------------
/// @file DriverNewPort_AGUC2.cpp
///
/// @brief
/// This file provides a AGUC2 class implementing a driver for a non specified controller 
/// only the Custom command allows to communicate with the driver from the super class.
/// To allow permission in proc/bus/usb at autoconnect modify in /etc/fstab (root)
/// none                    /proc/bus/usb           usbfs   defaults,devmode=0666   0 0
///

// who      when     what
// -----------------------------------------------------------------------------
// sentenac 06/10/08 AGUC2 driver implementation
// -----------------------------------------------------------------------------

#include "DriverNewPort_AGUC2.h"

using namespace DriverDefinition;

const int   DriverNewPort_AGUC2::BUFFER_SIZE          = 0;  // buffer size for serial messages
const int   DriverNewPort_AGUC2::MAX_DEVICES          = 0;  // maximum number of AGUC2 controllers
const float DriverNewPort_AGUC2::MAX_VEL              = 0; // maximum vel of AGUC2 controllers
const int   DriverNewPort_AGUC2::NB_ITEM_INIT_SETTING = 1;  // number of items at the actuator (#channel, velocity)
const int   DriverNewPort_AGUC2::NB_ITEM_DRV_SETTING  = 0;  // number of items at the driver (axisnumber)
const int   DriverNewPort_AGUC2::MAX_TRIES            = 10;  // Number of read tries
const int   DriverNewPort_AGUC2::MIN_BYTES_TRANS      = 7;  // Minimum bytes transferred

const DriverFeature DriverNewPort_AGUC2::AGUC2_FEATURE = 
  { "step","step",1,1,OPEN_LOOP,NULL };

// 
// Init implementation
//
int DriverNewPort_AGUC2::Init(string& rstateData) const 
{
  QLOG_DEBUG () <<  "DriverNewPort_AGUC2::Init";
  int retStatus = 0;
  string command;
  string answer;
  int cnt = 0;
  command = "unused";
 
    // Send initialization message to AG-UC2 device (USB port)
  if (_pcommChannel->Write(command,"CONTROL",0xC0,0x00,0x00,0x00,0x00,1,0) < 1)
    {
      QLOG_ERROR () << "DriverNewPort_AGUC2::Init> Unable to write to port";
      _lastError = COMMUNICATION_ERROR;
      retStatus = -1;
    }
  if (_pcommChannel->Write(command,"CONTROL",0xC0,0x04,0x08,0x00,0x00,1,0) < 1)
    {
      QLOG_ERROR () << "DriverNewPort_AGUC2::Init> Unable to write to port";
      _lastError = COMMUNICATION_ERROR;
      retStatus = -1;
    }
  if (_pcommChannel->Write(command,"CONTROL",0xC0,0x03,0x8003,0x00,0x00,1,0) < 1)
    {
      QLOG_ERROR () << "DriverNewPort_AGUC2::Init> Unable to write to port";
      _lastError = COMMUNICATION_ERROR;
      retStatus = -1;
    }
  if (_pcommChannel->Write(command,"CONTROL",0xC0,0x02,0x00,0x00,0x00,1,0) < 1)
    {
      QLOG_ERROR () << "DriverNewPort_AGUC2::Init> Unable to write to port";
      _lastError = COMMUNICATION_ERROR;
      retStatus = -1;
    }
  if (retStatus == -1) return retStatus;
  // Ask device name
  command = "VE\r\n";
  int transferred, atransferred = 0;
  while ( cnt < MAX_TRIES && atransferred < MIN_BYTES_TRANS) {
    cnt++;
    retStatus = 0;
    usleep(10000);
    if (_pcommChannel->Write(command,"BULK",0x02,&transferred,100) < (int)command.length())
      {
	QLOG_ERROR () << "DriverNewPort_AGUC2::Init> Unable to write to port";
	_lastError = COMMUNICATION_ERROR;
	retStatus = -1;
      }
    QLOG_DEBUG () << "DriverNewPort_AGUC2::Init> transferred " << transferred;
    usleep(10000);
    if (_pcommChannel->Read(answer,"BULK",0x81,&atransferred,100) < MIN_BYTES_TRANS)
      {
	QLOG_ERROR () << "DriverNewPort_AGUC2::Init> Unable to read from port";
	QLOG_ERROR () << "DriverNewPort_AGUC2::Init> answer = " << answer.c_str() << " bytes transferred "
		      << atransferred;
	_lastError = COMMUNICATION_ERROR;
	retStatus = -1;
      }
  }
   QLOG_DEBUG () << "DriverNewPort_AGUC2::Init> answer " << answer.c_str() << " bytes transferred "
		 << atransferred;
   usleep(10000);
  // Set to remote mode
  command = "MR\r\n";
  if (_pcommChannel->Write(command,"BULK",0x02,&transferred,100) < (int)command.length())
    {
      QLOG_ERROR () << "DriverNewPort_AGUC2::Init> Unable to write to port";
      _lastError = COMMUNICATION_ERROR;
      retStatus = -1;
    }
  QLOG_DEBUG () << 
		  "DriverNewPort_AGUC2::Init> transferred " << transferred;

  return retStatus;
}

//
// InitActuator implementation
//
// use company delivered optimized settings
//
int DriverNewPort_AGUC2::InitActuator(string actuatorSetting,
				     float  position) const 
{
  QLOG_DEBUG () <<  "DriverNewPort_AGUC2::InitActuator";
  int retStatus = 0;
  return retStatus;
}

//
// GetPos implementation
//
// 1. send "xxTP%d?" (tell position) to get the current position
// 2. read reply
// 3. parse position from reply
//
int DriverNewPort_AGUC2::GetPos(string  actuatorSetting,
			       float&  position) const 
{
  QLOG_DEBUG () <<  "DriverNewPort_AGUC2::GetPos";
  int retStatus = 0;
  string command;
  char commandC[64];
  string answer = "";
  string subAns;
  int axisNumber;
  int transferred, atransferred = 0;
  int cnt = 0;
  if (sscanf(actuatorSetting.c_str(),"axisNumber=%d",&axisNumber) != NB_ITEM_INIT_SETTING)
  {
    QLOG_ERROR () << "Bad Actuator setting";
  }
  sprintf(commandC,"%dTP\r\n",axisNumber);
  command = commandC;

  QLOG_DEBUG () <<  "DriverNewPort_AGUC2::GetPos> command "
	         << command.c_str();
  while ( cnt < MAX_TRIES && atransferred < MIN_BYTES_TRANS) {
    cnt++;
    retStatus = 0;
    usleep(10000);
    if (_pcommChannel->Write(command,"BULK",0x02,&transferred,100) < (int)command.length())
      {
	QLOG_ERROR () << "DriverNewPort_AGUC2::GetPos> Unable to write to port";
	_lastError = COMMUNICATION_ERROR;
	retStatus = -1;
      }
    usleep(10000);
    if (_pcommChannel->Read(answer,"BULK",0x81,&atransferred,100) < MIN_BYTES_TRANS)
      {
	QLOG_ERROR () << "DriverNewPort_AGUC2::GetPos> Unable to read from port";
	_lastError = COMMUNICATION_ERROR;
	position  = 0;
      retStatus = -1;
      }
    else {
      subAns = answer.substr(5,answer.length());
      position = atoi(subAns.c_str());
    }
    QLOG_DEBUG () << "DriverNewPort_AGUC2::Init> answer " << answer.c_str() << " bytes transferred "
                 << transferred;
  }
  return retStatus;
}

//
// Move implementation
//
// 1. send "xxPRnn" (move relative) to move xxxx steps
//
int DriverNewPort_AGUC2::Move(string actuatorSetting,
			     float  nbSteps,
			     int    unit) const 
{
  QLOG_DEBUG () <<  "DriverNewPort_AGUC2::Move";

  int retStatus = 0;
  string command;
  char commandC[64];
  int axisNumber;
  int transferred;
  int numberOfSteps;
 
  usleep(10000);
  if (sscanf(actuatorSetting.c_str(),"axisNumber=%d",&axisNumber) != NB_ITEM_INIT_SETTING)
  {
    QLOG_ERROR () << "Bad Actuator setting";
  }
 
  numberOfSteps = (int) nbSteps;
  sprintf(commandC,"%dPR%d\r\n",axisNumber,numberOfSteps);
  command = commandC;
  
  QLOG_DEBUG () << 
	       "DriverNewPort_AGUC2::Move> command " << command.c_str();
  if (_pcommChannel->Write(command,"BULK",0x02,&transferred,100) < (int)command.length())
    {
      QLOG_ERROR () << "DriverNewPort_AGUC2::Move> Unable to write to port";
      _lastError = COMMUNICATION_ERROR;
      retStatus = -1;
    }
  
  return retStatus;
}

//
// MoveAbs implementation
//
// 1. send "xxPAnn" move absolute command to move to position absPos
//
int DriverNewPort_AGUC2::MoveAbs(string actuatorSetting,
			      float  absPos,
			      int    unit) const 
{
  QLOG_DEBUG () <<  "DriverNewPort_AGUC2::MoveAbs";
  int retStatus = NOT_CLOSED_LOOP;
  return retStatus;
}

//
// Stop implementation
//
// 1. send "xxST" (stop smoothly) to stop the
//    current movement
//
int DriverNewPort_AGUC2::Stop(string actuatorSetting) const 
{
  QLOG_DEBUG () <<  "DriverNewPort_AGUC2::Stop";

  int retStatus = 0;
  string command;
  string answer;
  char commandC[64];
  int axisNumber;
  int transferred;
  usleep(10000);
  if (sscanf(actuatorSetting.c_str(),"axisNumber=%d",&axisNumber) != NB_ITEM_INIT_SETTING)
    {
      QLOG_ERROR () << "Bad Actuator setting";
    }
  else 
    {
      sprintf(commandC,"%dST\r\n",axisNumber);
      command = commandC;
      
      QLOG_DEBUG () <<  "DriverNewPort_AGUC2::Stop> command = %s"
		    << command.c_str();
      if (_pcommChannel->Write(command,"BULK",0x02,&transferred,100) < (int)command.length())
	{
	  QLOG_ERROR () << "DriverNewPort_AGUC2::Stop> Unable to write to port";
	  _lastError = COMMUNICATION_ERROR;
	  retStatus = -1;
	} 
    }
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
int DriverNewPort_AGUC2::OperationComplete(string& rstateData,
					  string actuatorSetting,
					  ADLimitSwitch& rlimitSwitch) const
{
  QLOG_DEBUG () <<  "DriverNewPort_AGUC2::OperationComplete";
  
  int  retStatus = 1;
  string command;
  char commandC[64];
  string answer;
  string subAns;
  int axisNumber;
  int transferred, atransferred = 0;
  int status;
  int cnt = 0;

  if (sscanf(actuatorSetting.c_str(),"axisNumber=%d",&axisNumber) != NB_ITEM_INIT_SETTING)
  {
    QLOG_ERROR () << "Bad Actuator setting";
  }
  else 
  {
    sprintf(commandC,"%dTS\r\n",axisNumber);
    command = commandC;
    while ( cnt < MAX_TRIES && atransferred < MIN_BYTES_TRANS) {
      cnt++;
      retStatus = 1;
      usleep(10000);
      QLOG_DEBUG () <<  
		   "DriverNewPort_AGUC2::OperationComplete> command "
		    << command.c_str();
      if (_pcommChannel->Write(command,"BULK",0x02,&transferred,100) < (int)command.length())
	{
	  QLOG_ERROR () << "DriverNewPort_AGUC2::OperationComplete> Unable to write to port";
	  _lastError = COMMUNICATION_ERROR;
	  retStatus = -1;
	}
      usleep(10000);
      if (_pcommChannel->Read(answer,"BULK",0x81,&atransferred,100) < MIN_BYTES_TRANS)
	{
	  QLOG_ERROR () << "DriverNewPort_AGUC2::OperationComplete> Unable to read from port";
	  _lastError = COMMUNICATION_ERROR;
	  retStatus = -1;
	}
      else {
        QLOG_DEBUG () << "DriverNewPort_AGUC2::Init> answer " << answer.c_str() << " bytes transferred "
                 << atransferred;
	subAns = answer.substr(5,1);
	status = atoi(subAns.c_str());
	QLOG_DEBUG () << 
			"DriverNewPort_AGUC2::OperationComplete> status TS " << status;
	if (status == 1) // motion not completed
	  retStatus = 0;
	if (status == 0) // motion completed
	  retStatus = 1; 
      }
    }
  }
  return retStatus;
}

//
// GetActuatorFeature implementation
//
//    Gives the translation stages features
//
int DriverNewPort_AGUC2::GetActuatorFeature(DriverFeature& ractuatorFeature) const
{
  QLOG_DEBUG () <<  "DriverNewPort_AGUC2::GetActuatorFeature";
  
  int retStatus = 0;
  
  ractuatorFeature = AGUC2_FEATURE;
  
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
int DriverNewPort_AGUC2::ConvertUnit(int unit, 
				   float valueToConvert, 
				   float& rconvertedValue,
				   float& rrange) const 
{
  QLOG_DEBUG () <<  "DriverNewPort_AGUC2::ConvertUnit";
  int retStatus = 0;  
  rconvertedValue = valueToConvert;
  rrange = 0;
  return retStatus;
}
//------------------------------------------------------------------------------
/// Operation : SendGeneralCommand
///    Sends a command to the motor driver interface
///
/// @param state
///    command line text
/// @param actuatorName
/// actuator name
//------------------------------------------------------------------------------
int 
DriverNewPort_AGUC2::SendGeneralCommand(char *buffer,string& rply) const
{  
  int retStatus = 0;
  string answer = "";
  string command = "";
  int transferred, atransferred = 0;
  char mybuffer[32];
  vector<string>  mBufferVect;
  string strbuffer =  buffer;
  string commandPrefix;
  int clength;

  QLOG_DEBUG () <<  "DriverNewPort_AGUC2::SendGeneralCommand> reSttatus " << retStatus;
  
  if (!mBufferVect.empty()) mBufferVect.erase(mBufferVect.begin(),mBufferVect.end());  
  Tokenize(strbuffer,mBufferVect,",");
  for (unsigned int i = 0 ; i < mBufferVect.size(); i++ ) {
    sprintf(mybuffer,"%s\r",mBufferVect[i].c_str());
    command = mybuffer;
    commandPrefix = command.substr(0,3);
    clength = command.length();
    QLOG_DEBUG () <<  "DriverNewPort_AGUC2::SendGeneralCommand> Send Command " << command.c_str() << " length " 
		  << command.length();
    //////////////////////////////////////////////////////////////////////////
    // Apply Command
    //////////////////////////////////////////////////////////////////////////
    if (_pcommChannel->Write(command,"BULK",0x02,&transferred,100) < (int)command.length()) { 
      QLOG_DEBUG () <<  
		   "DriverNewPort_AGUC2::SendGeneralCommand> Unable to write to port";
      retStatus = (1 << 0);
      QLOG_DEBUG () <<  "DriverNewPort_AGUC2::SendGeneralCommand> reSttatus " << retStatus;
    }
    
    usleep(100000);
  }
  ////////////////////////////////////////////////////////////////////////////////////
  // Read reply
  ////////////////////////////////////////////////////////////////////////////////////
  if (!_pcommChannel->Read(answer,"BULK",0x81,&atransferred,100)) {
    QLOG_DEBUG () <<  
		 "Driver> Unable to read from port";
    retStatus |= (1 << 1);
    QLOG_DEBUG () <<  "DriverNewPort_AGUC2::SendGeneralCommand> retStatus " << retStatus;
  }
  QLOG_DEBUG () << "DriverNewPort_AGUC2::Init> answer " << answer.c_str() << " bytes transferred "
                 << atransferred;
  rply = answer;
  QLOG_DEBUG () <<  
	       "DriverNewPort_AGUC2::SendGeneralCommand> returning retStatus " << retStatus;
  return retStatus;
}

