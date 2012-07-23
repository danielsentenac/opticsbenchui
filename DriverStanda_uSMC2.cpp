/// ----------------------------------------------------------------------------
/// @file DriverStanda_uSMC.cpp
///
/// @brief
/// This file provides a USMC class implementing a driver for a non specified controller 
/// only the Custom command allows to communicate with the driver from the super class.
/// To allow permission in proc/bus/usb at autoconnect modify in /etc/fstab (root)
/// none                    /proc/bus/usb           usbfs   defaults,devmode=0666   0 0
///

// who      when     what
// -----------------------------------------------------------------------------
// sentenac 06/10/08 USMC driver implementation
// -----------------------------------------------------------------------------

#include "DriverStanda_uSMC2.h"

using namespace DriverDefinition;

const int   DriverStanda_uSMC::BUFFER_SIZE          = 0;  // buffer size for serial messages
const int   DriverStanda_uSMC::MAX_DEVICES          = 0;  // maximum number of USMC controllers
const float DriverStanda_uSMC::MAX_VEL              = 0; // maximum vel of USMC controllers
const int   DriverStanda_uSMC::NB_ITEM_INIT_SETTING = 1;  // number of items at the actuator (#channel, velocity)
const int   DriverStanda_uSMC::NB_ITEM_DRV_SETTING  = 0;  // number of items at the driver (axisnumber)
const int   DriverStanda_uSMC::MAX_TRIES            = 10;  // Number of read tries
const int   DriverStanda_uSMC::MIN_BYTES_TRANS      = 7;  // Minimum bytes transferred

const DriverFeature DriverStanda_uSMC::USMC_FEATURE = 
  {"step","step",1,1,CLOSED_LOOP,NULL };

// 
// Init implementation
//
int DriverStanda_uSMC::Init(string& rstateData) const 
{
  QLOG_DEBUG () <<  "DriverStanda_uSMC::Init";
  int retStatus = 0;
  string command;
  string answer;
  int cnt = 0;
  command = "unused";
  unsigned char buffer[16];
  int serial = 0;
     
  // Send initialization message to STANDA device (USB port)
  if (_pcommChannel->Write(command,"CONTROL",0xC0,0xC9,0x00,0x00,&buffer[0],16,1000) < 16)
    {
      QLOG_ERROR () << "DriverStanda_uSMC::Init> Unable to write to port";
      _lastError = COMMUNICATION_ERROR;
      return -1;
    }
  QLOG_INFO () << " Controller serial number " << buffer; 
 
  return retStatus;
}

//
// InitActuator implementation
//
// use company delivered optimized settings
//
int DriverStanda_uSMC::InitActuator(string actuatorSetting,
				     float  position) const 
{
  QLOG_DEBUG () <<  "DriverStanda_uSMC::InitActuator";
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
int DriverStanda_uSMC::GetPos(string  actuatorSetting,
			       float&  position) const 
{
  QLOG_DEBUG () <<  "DriverStanda_uSMC::GetPos";
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

  QLOG_DEBUG () <<  "DriverStanda_uSMC::GetPos> command "
	         << command.c_str();
  while ( cnt < MAX_TRIES && atransferred < MIN_BYTES_TRANS) {
    cnt++;
    retStatus = 0;
    usleep(10000);
    if (_pcommChannel->Write(command,"BULK",0x02,&transferred,100) < (int)command.length())
      {
	QLOG_ERROR () << "DriverStanda_uSMC::GetPos> Unable to write to port";
	_lastError = COMMUNICATION_ERROR;
	retStatus = -1;
      }
    usleep(10000);
    if (_pcommChannel->Read(answer,"BULK",0x81,&atransferred,100) < MIN_BYTES_TRANS)
      {
	QLOG_ERROR () << "DriverStanda_uSMC::GetPos> Unable to read from port";
	_lastError = COMMUNICATION_ERROR;
	position  = 0;
      retStatus = -1;
      }
    else {
      subAns = answer.substr(5,answer.length());
      position = atoi(subAns.c_str());
    }
    QLOG_DEBUG () << "DriverStanda_uSMC::Init> answer " << answer.c_str() << " bytes transferred "
                 << transferred;
  }
  return retStatus;
}

//
// Move implementation
//
// 1. send "xxPRnn" (move relative) to move xxxx steps
//
int DriverStanda_uSMC::Move(string actuatorSetting,
			     float  nbSteps,
			     int    unit) const 
{
  QLOG_DEBUG () <<  "DriverStanda_uSMC::Move";

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
	       "DriverStanda_uSMC::Move> command " << command.c_str();
  if (_pcommChannel->Write(command,"BULK",0x02,&transferred,100) < (int)command.length())
    {
      QLOG_ERROR () << "DriverStanda_uSMC::Move> Unable to write to port";
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
int DriverStanda_uSMC::MoveAbs(string actuatorSetting,
			      float  absPos,
			      int    unit) const 
{
  QLOG_DEBUG () <<  "DriverStanda_uSMC::MoveAbs";
  int retStatus = NOT_CLOSED_LOOP;
  return retStatus;
}

//
// Stop implementation
//
// 1. send "xxST" (stop smoothly) to stop the
//    current movement
//
int DriverStanda_uSMC::Stop(string actuatorSetting) const 
{
  QLOG_DEBUG () <<  "DriverStanda_uSMC::Stop";

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
      
      QLOG_DEBUG () <<  "DriverStanda_uSMC::Stop> command = %s"
		    << command.c_str();
      if (_pcommChannel->Write(command,"BULK",0x02,&transferred,100) < (int)command.length())
	{
	  QLOG_ERROR () << "DriverStanda_uSMC::Stop> Unable to write to port";
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
int DriverStanda_uSMC::OperationComplete(string& rstateData,
					  string actuatorSetting,
					  ADLimitSwitch& rlimitSwitch) const
{
  QLOG_DEBUG () <<  "DriverStanda_uSMC::OperationComplete";
  
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
		   "DriverStanda_uSMC::OperationComplete> command "
		    << command.c_str();
      if (_pcommChannel->Write(command,"BULK",0x02,&transferred,100) < (int)command.length())
	{
	  QLOG_ERROR () << "DriverStanda_uSMC::OperationComplete> Unable to write to port";
	  _lastError = COMMUNICATION_ERROR;
	  retStatus = -1;
	}
      usleep(10000);
      if (_pcommChannel->Read(answer,"BULK",0x81,&atransferred,100) < MIN_BYTES_TRANS)
	{
	  QLOG_ERROR () << "DriverStanda_uSMC::OperationComplete> Unable to read from port";
	  _lastError = COMMUNICATION_ERROR;
	  retStatus = -1;
	}
      else {
        QLOG_DEBUG () << "DriverStanda_uSMC::Init> answer " << answer.c_str() << " bytes transferred "
                 << atransferred;
	subAns = answer.substr(5,1);
	status = atoi(subAns.c_str());
	QLOG_DEBUG () << 
			"DriverStanda_uSMC::OperationComplete> status TS " << status;
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
int DriverStanda_uSMC::GetActuatorFeature(DriverFeature& ractuatorFeature) const
{
  QLOG_DEBUG () <<  "DriverStanda_uSMC::GetActuatorFeature";
  
  int retStatus = 0;
  
  ractuatorFeature = USMC_FEATURE;
  
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
int DriverStanda_uSMC::ConvertUnit(int unit, 
				   float valueToConvert, 
				   float& rconvertedValue,
				   float& rrange) const 
{
  QLOG_DEBUG () <<  "DriverStanda_uSMC::ConvertUnit";
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
DriverStanda_uSMC::SendGeneralCommand(char *buffer,string& rply) const
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

  QLOG_DEBUG () <<  "DriverStanda_uSMC::SendGeneralCommand> reSttatus " << retStatus;
  
  if (!mBufferVect.empty()) mBufferVect.erase(mBufferVect.begin(),mBufferVect.end());  
  Tokenize(strbuffer,mBufferVect,",");
  for (unsigned int i = 0 ; i < mBufferVect.size(); i++ ) {
    sprintf(mybuffer,"%s\r",mBufferVect[i].c_str());
    command = mybuffer;
    commandPrefix = command.substr(0,3);
    clength = command.length();
    QLOG_DEBUG () <<  "DriverStanda_uSMC::SendGeneralCommand> Send Command " << command.c_str() << " length " 
		  << command.length();
    //////////////////////////////////////////////////////////////////////////
    // Apply Command
    //////////////////////////////////////////////////////////////////////////
    if (_pcommChannel->Write(command,"BULK",0x02,&transferred,100) < (int)command.length()) { 
      QLOG_DEBUG () <<  
		   "DriverStanda_uSMC::SendGeneralCommand> Unable to write to port";
      retStatus = (1 << 0);
      QLOG_DEBUG () <<  "DriverStanda_uSMC::SendGeneralCommand> reSttatus " << retStatus;
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
    QLOG_DEBUG () <<  "DriverStanda_uSMC::SendGeneralCommand> retStatus " << retStatus;
  }
  QLOG_DEBUG () << "DriverStanda_uSMC::Init> answer " << answer.c_str() << " bytes transferred "
                 << atransferred;
  rply = answer;
  QLOG_DEBUG () <<  
	       "DriverStanda_uSMC::SendGeneralCommand> returning retStatus " << retStatus;
  return retStatus;
}

