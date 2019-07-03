/// ----------------------------------------------------------------------------
/// @file DriverStanda_uSMC2.cpp
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

const int   DriverStanda_uSMC2::BUFFER_SIZE          = 0;  // buffer size for serial messages
const int   DriverStanda_uSMC2::MAX_DEVICES          = 0;  // maximum number of USMC controllers
const float DriverStanda_uSMC2::MAX_VEL              = 0; // maximum vel of USMC controllers
const int   DriverStanda_uSMC2::NB_ITEM_INIT_SETTING = 2;  // number of items at the actuator (#channel, velocity)
const int   DriverStanda_uSMC2::NB_ITEM_DRV_SETTING  = 0;  // number of items at the driver (axisnumber)
const int   DriverStanda_uSMC2::MAX_TRIES            = 10;  // Number of read tries
const int   DriverStanda_uSMC2::MIN_BYTES_TRANS      = 7;  // Minimum bytes transferred

const DriverFeature DriverStanda_uSMC2::USMC2_FEATURE = 
  {"step","step",1,1,CLOSED_LOOP,NULL };

// 
// Init implementation
//
int DriverStanda_uSMC2::Init(string& rstateData) const 
{
  QLOG_DEBUG () <<  "DriverStanda_uSMC2::Init";
  int retStatus = 0;
  int cnt = 0;
  char ximc_version_str[32];
  char device_name[256];
  int probe_flags = 0;

  ximc_version( ximc_version_str );
  QLOG_INFO () << ximc_version_str ; 
 
//	Device enumeration function. Returns an opaque pointer to device enumeration data.
  _devenum = enumerate_devices( probe_flags, 0 );
//	Gets device count from device enumeration data
  cnt = get_device_count( _devenum );
  QLOG_INFO () << "Found " << cnt << " devices";
//      Terminate if there are no connected devices
  if (cnt <= 0) {
   QLOG_WARN() << "No devices found";
//      Free memory used by device enumeration data
   free_enumerate_devices( _devenum );
   return -1;
  }
//	 get devices name into a string
  for (int i = 0; i < cnt; i++) {
    strcpy( device_name, get_device_name( _devenum, i ) );
    
    QLOG_INFO () << device_name;
  }

  return retStatus;
}

//
// InitActuator implementation
//
// use company delivered optimized settings
//
int DriverStanda_uSMC2::InitActuator(string actuatorSetting,
				     float  position) const 
{
  QLOG_DEBUG () <<  "DriverStanda_uSMC2::InitActuator";
  int axisNumber;
  int speed;
  int retStatus = 0;

  if (sscanf(actuatorSetting.c_str(),"axisNumber=%d speed=%d",&axisNumber,&speed) != NB_ITEM_INIT_SETTING)
  {
    QLOG_ERROR () << "Bad Actuator setting";
  }
// Open device
  device_t dev = open_device(get_device_name(_devenum,axisNumber-1));
  if (dev != -1){
    QLOG_INFO() << "Open device " << get_device_name(_devenum,axisNumber-1);
    _device.insert(std::pair<int,device_t>(axisNumber,dev));
  }
  else {
   QLOG_ERROR() << "Error opening device with axis number " << axisNumber;
   return -1;
  }
// Set speed settings
  retStatus = get_move_settings(_device.find(axisNumber)->second,&_move_settings);
  _move_settings.Speed = speed;
  retStatus = set_move_settings(_device.find(axisNumber)->second,&_move_settings);

  return retStatus;
}

//
// GetPos implementation
//
// 1. send "xxTP%d?" (tell position) to get the current position
// 2. read reply
// 3. parse position from reply
//
int DriverStanda_uSMC2::GetPos(string  actuatorSetting,
			       float&  position) const 
{
  QLOG_DEBUG () <<  "DriverStanda_uSMC2::GetPos";
  int retStatus = 0;
  int axisNumber;
  int speed;
  status_t status;

  if (sscanf(actuatorSetting.c_str(),"axisNumber=%d speed=%d",&axisNumber,&speed) != NB_ITEM_INIT_SETTING)
  {
    QLOG_ERROR () << "Bad Actuator setting";
  }
  retStatus = (int) get_status( _device.find(axisNumber)->second, &status);
  
  QLOG_DEBUG() << " Device:" << get_device_name(_devenum,axisNumber-1) << " Position:" << status.CurPosition;

  position = (float) status.CurPosition;
 
  return retStatus;
}

//
// Move implementation
//
// 1. send "xxPRnn" (move relative) to move xxxx steps
//
int DriverStanda_uSMC2::Move(string actuatorSetting,
			     float  nbSteps,
			     int    unit) const 
{
  QLOG_DEBUG () <<  "DriverStanda_uSMC2::Move";

  int retStatus = 0;
  int axisNumber;
  int speed;

  if (sscanf(actuatorSetting.c_str(),"axisNumber=%d speed=%d",&axisNumber,&speed) != NB_ITEM_INIT_SETTING)
  {
    QLOG_ERROR () << "Bad Actuator setting";
  }
  retStatus = (int) command_movr(_device.find(axisNumber)->second, (int) nbSteps, 0);
  usleep(500000);
  return retStatus;
}

//
// MoveAbs implementation
//
// 1. send "xxPAnn" move absolute command to move to position absPos
//
int DriverStanda_uSMC2::MoveAbs(string actuatorSetting,
			      float  absPos,
			      int    unit) const 
{
  QLOG_DEBUG () <<  "DriverStanda_uSMC2::MoveAbs";
  int retStatus = 0;
  int axisNumber;
  int speed;

  if (sscanf(actuatorSetting.c_str(),"axisNumber=%d speed=%d",&axisNumber,&speed) != NB_ITEM_INIT_SETTING)
  {
    QLOG_ERROR () << "Bad Actuator setting";
  }
  retStatus = (int) command_move(_device.find(axisNumber)->second, (int) absPos, 0);
  usleep(500000);
  return retStatus;
}

//
// Stop implementation
//
// 1. send "xxST" (stop smoothly) to stop the
//    current movement
//
int DriverStanda_uSMC2::Stop(string actuatorSetting) const 
{
  QLOG_DEBUG () <<  "DriverStanda_uSMC2::Stop";

  int retStatus = 0;
  int axisNumber;
  int speed;

  if (sscanf(actuatorSetting.c_str(),"axisNumber=%d speed=%d",&axisNumber,&speed) != NB_ITEM_INIT_SETTING)
  {
    QLOG_ERROR () << "Bad Actuator setting";
  }
  retStatus = (int) command_stop(_device.find(axisNumber)->second);

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
int DriverStanda_uSMC2::OperationComplete(string& rstateData,
					  string actuatorSetting,
					  ADLimitSwitch& rlimitSwitch) const
{
  QLOG_DEBUG () <<  "DriverStanda_uSMC2::OperationComplete";
  
  int  retStatus = 1;
  int axisNumber;
  int speed;

  if (sscanf(actuatorSetting.c_str(),"axisNumber=%d speed=%d",&axisNumber,&speed) != NB_ITEM_INIT_SETTING)
  {
    QLOG_ERROR () << "Bad Actuator setting";
  }
  retStatus = (int) get_status(_device.find(axisNumber)->second, &_status);
  
  QLOG_DEBUG () << " get_status " << (int) retStatus  << " MoveSts " << _status.MoveSts;
  if ( _status.MoveSts != 0 )
    retStatus = 0;
  else {
    // Save last position in controller
    float position;
    GetPos(actuatorSetting,position);
    QLOG_DEBUG () << " Save last position for device " << (int) position;
    retStatus= 1;
  }

  return (retStatus);
}

//
// GetActuatorFeature implementation
//
//    Gives the translation stages features
//
int DriverStanda_uSMC2::GetActuatorFeature(DriverFeature& ractuatorFeature) const
{
  QLOG_DEBUG () <<  "DriverStanda_uSMC2::GetActuatorFeature";
  
  int retStatus = 0;
  
  ractuatorFeature = USMC2_FEATURE;
  
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
int DriverStanda_uSMC2::ConvertUnit(int unit, 
				   float valueToConvert, 
				   float& rconvertedValue,
				   float& rrange) const 
{
  QLOG_DEBUG () <<  "DriverStanda_uSMC2::ConvertUnit";
  int retStatus = 0;  
  rconvertedValue = valueToConvert;
  rrange = 0;
  return retStatus;
}
//
// Exit funtion
//
int DriverStanda_uSMC2::Exit() 
{
// Close devices
   std::map<int,device_t>::iterator it;
   for ( it = _device.begin() ; it != _device.end(); ++it)
     close_device((device_t*)&(it->second));

   // Free memory used by device enumeration data
   free_enumerate_devices( _devenum );

   return 0;
}
