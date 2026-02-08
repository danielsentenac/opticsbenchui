/// ----------------------------------------------------------------------------
/// @file DriverStanda_uSMC.cpp
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

#include "DriverStanda_uSMC.h"

using namespace DriverDefinition;

const int   DriverStanda_uSMC::BUFFER_SIZE          = 0;  // buffer size for serial messages
const int   DriverStanda_uSMC::MAX_DEVICES          = 0;  // maximum number of AGUC2 controllers
const float DriverStanda_uSMC::MAX_VEL              = 0; // maximum vel of AGUC2 controllers
const int   DriverStanda_uSMC::NB_ITEM_INIT_SETTING = 4;  // number of items at the actuator (#channel, velocity)
const int   DriverStanda_uSMC::NB_ITEM_DRV_SETTING  = 0;  // number of items at the driver (axisnumber)
const int   DriverStanda_uSMC::MAX_TRIES            = 10;  // Number of read tries
const int   DriverStanda_uSMC::MIN_BYTES_TRANS      = 7;  // Minimum bytes transferred

const DriverFeature DriverStanda_uSMC::STANDA_USMC_FEATURE = 
  { "step","step",1,1,CLOSED_LOOP,NULL };

// 
// Init implementation
//
int DriverStanda_uSMC::Init(string& rstateData) const 
{
  QLOG_DEBUG () << "DriverStanda_uSMC::Init";
  int retStatus = 0;
  if ( USMC_Init ( &_devices ) )
    {
	QLOG_ERROR () << " Error retrieving devices";
        return -1;
    }
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
  QLOG_DEBUG () << "DriverStanda_uSMC::InitActuator";
  int retStatus = 0;
  if (sscanf(actuatorSetting.c_str(),"axisNumber=%u rotator=%d speed=%f divisor=%d",
             &_cur_dev, &_rotator, &_speed, &_divisor) != NB_ITEM_INIT_SETTING)
    {
      ReportSettingError("Bad actuator setting");
      return -1;
    }
   // Update controller mode respect to rotator case
   if ( USMC_GetMode ( _cur_dev, &_mode ) )
   {
      QLOG_ERROR () << "Error retrieving mode for device " << _cur_dev;
      return -1;
   }
  if (_rotator == 1 )
  {
      QLOG_INFO () << " Disable limit switches for rotator device " << _cur_dev; 
      _mode.Tr1En = !_mode.Tr1En;
      _mode.Tr2En = !_mode.Tr2En;
      if ( USMC_SetMode ( _cur_dev, &_mode ) )
      {
          QLOG_ERROR () << "Error setting mode for device " << _cur_dev;
          return -1;
      }
  }
  // Check controller mode 
   if ( USMC_GetMode ( _cur_dev, &_mode ) )
   {
      QLOG_ERROR () << "Error retrieving mode for device " << _cur_dev;
      return -1;
   }
   QLOG_INFO () << " Limit switches are " << _mode.Tr1En << " " << _mode.Tr2En << " for device " << _cur_dev;
  // Get actuator parameters
  if ( USMC_GetParameters ( _cur_dev, &_params ) )
  {
      QLOG_ERROR () << " Error retrieving parameters for device " << _cur_dev;
      return -1;
  }
  // Save last position in controller
    QLOG_DEBUG () << " Save last position for device " << (int) position;
    // Set actuator position
    if ( USMC_SetCurrentPosition ( _cur_dev, (int) position ) )
      {
        QLOG_ERROR () << " Error setting position in controller " << _cur_dev;
        return -1;
      }
  retStatus = GetPos(actuatorSetting,position);
  QLOG_INFO () << " Device " << _cur_dev << " initial position from controller is to " << position;
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
  QLOG_DEBUG () << "DriverStanda_uSMC::GetPos";
  int retStatus = 0;
  if (sscanf(actuatorSetting.c_str(),"axisNumber=%u rotator=%d speed=%f divisor=%d",
             &_cur_dev, &_rotator, &_speed, &_divisor) != NB_ITEM_INIT_SETTING)
    {
      ReportSettingError("Bad actuator setting");
      return -1;
    }
  if ( USMC_GetState ( _cur_dev, &_state ) )
    {
        QLOG_ERROR () << " Error retrieving state for device " << _cur_dev;
        return -1;
    }
    position = _state.CurPos;
    QLOG_DEBUG() << " Current position " << position
                 << " Rotator settings " << _rotator 
                 << " Speed settings " << _speed 
                 << " Divisor settings " << _divisor;
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
  QLOG_DEBUG () << "DriverStanda_uSMC::Move";
  int retStatus = 0;
  if (sscanf(actuatorSetting.c_str(),"axisNumber=%u rotator=%d speed=%f divisor=%d",
             &_cur_dev, &_rotator, &_speed, &_divisor) != NB_ITEM_INIT_SETTING)
    {
      ReportSettingError("Bad actuator setting");
      return -1;
    }
   // Get current position
   float curpos;
   GetPos(actuatorSetting, curpos);
   int dest_pos  = (int)curpos + (int)nbSteps;
   QLOG_DEBUG () << "DriverStanda_uSMC::Move> to position " << dest_pos;
   // Update start parameters
   if ( USMC_GetStartParameters ( _cur_dev, &_start_params ) )
    {   
      QLOG_ERROR () << " USMC_GetStartParameters failed ";
      return -1;
    }

   _start_params.SDivisor = _divisor;
   _start_params.SlStart = FALSE;
   _start_params.LoftEn = FALSE;
   if ( USMC_Start ( _cur_dev, dest_pos, &_speed, &_start_params ) )
    {
      QLOG_ERROR () << " USMC_Start failed ";
      return -1;
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
  QLOG_DEBUG () << "DriverStanda_uSMC::MoveAbs";
  int retStatus = 0;
  if (sscanf(actuatorSetting.c_str(),"axisNumber=%u rotator=%d speed=%f divisor=%d",
             &_cur_dev, &_rotator, &_speed, &_divisor) != NB_ITEM_INIT_SETTING)
    {
      ReportSettingError("Bad actuator setting");
      return -1;
    }
   // Get current position
   float curpos;
   GetPos(actuatorSetting, curpos);
   int dest_pos  = (int)absPos;
   QLOG_DEBUG () << "DriverStanda_uSMC::MoveAbs> to position " << dest_pos;
   // Update start parameters
   if ( USMC_GetStartParameters ( _cur_dev, &_start_params ) )
    {
      QLOG_ERROR () << " USMC_GetStartParameters failed ";
      return -1;
    }

   _start_params.SDivisor = _divisor;
   _start_params.SlStart = FALSE;
   _start_params.LoftEn = FALSE;
   if ( USMC_Start ( _cur_dev, dest_pos, &_speed, &_start_params ) )
    {
      QLOG_ERROR () << " USMC_Start failed ";
      return -1;
    }

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
  QLOG_DEBUG () << "DriverStanda_uSMC::Stop";

  int retStatus = 0;
 
  if (sscanf(actuatorSetting.c_str(),"axisNumber=%u rotator=%d speed=%f divisor=%d",
             &_cur_dev, &_rotator, &_speed, &_divisor) != NB_ITEM_INIT_SETTING)
    {
      ReportSettingError("Bad actuator setting");
      return -1;
    }

  if ( USMC_Stop ( _cur_dev ) )
  {
      QLOG_ERROR () << " USMC_Stop failed ";
      return -1;
  }
  QLOG_DEBUG () << "Motor stopped " << _cur_dev;

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
  QLOG_DEBUG () << "DriverStanda_uSMC::OperationComplete";
  
  int  retStatus = 1;
  if (sscanf(actuatorSetting.c_str(),"axisNumber=%u rotator=%d speed=%f divisor=%d",
             &_cur_dev, &_rotator, &_speed, &_divisor) != NB_ITEM_INIT_SETTING)
    {
      ReportSettingError("Bad actuator setting");
      return -1;
    }

  if ( USMC_GetState ( _cur_dev, &_state ) )
    {
        QLOG_ERROR () << " Error retrieving state for device " << _cur_dev;
        return -1;
    }
  if ( _state.RUN ) retStatus = 0;
  else { 
    retStatus = 1; 
    // Save last position in controller 
    float position;
    GetPos(actuatorSetting,position);
    QLOG_DEBUG () << " Save last position for device " << (int) position;
    // Set actuator position
    if ( USMC_SetCurrentPosition ( _cur_dev, (int) position ) )
      {
        QLOG_ERROR () << " Error setting position in controller " << _cur_dev;
        return -1;
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
  QLOG_DEBUG () << "DriverStanda_uSMC::GetActuatorFeature";
  
  int retStatus = 0;
  
  ractuatorFeature = STANDA_USMC_FEATURE;
  
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
// Use indifferently default units or custom units configuration (STEPS == MM)
int DriverStanda_uSMC::ConvertUnit(int unit, 
				   float valueToConvert, 
				   float& rconvertedValue,
				   float& rrange) const 
{
  QLOG_DEBUG () << "DriverStanda_uSMC::ConvertUnit";
  int retStatus = 0;  
  rconvertedValue = valueToConvert;
  rrange = 0;
  return retStatus;
}
int DriverStanda_uSMC::Exit(string actuatorSetting)
{
  if (sscanf(actuatorSetting.c_str(),"axisNumber=%u rotator=%d speed=%f divisor=%d",
             &_cur_dev, &_rotator, &_speed, &_divisor) != NB_ITEM_INIT_SETTING)
    {
      ReportSettingError("Bad actuator setting");
      return -1;
    }

  QLOG_INFO() << " Saving last position in flash memory for device" << (int) _cur_dev;
  // Go to Full Step (automaticaly), then Turn Off
 /* _mode.ResetD = TRUE;

  if ( USMC_SetMode ( _cur_dev, &_mode ) )
       return  -1;

  // Wait until Previous Comand is Done
  do {
     usleep ( 50 );

     if ( USMC_GetState ( _cur_dev, &_state ) )
          return -1;
     } while ( _state.Power == TRUE );
*/
  float position;
  GetPos(actuatorSetting,position);
  QLOG_INFO () << " Device " << _cur_dev << " last position " << position;

  // Remember CurPos in Parameters Only While state.Power - FALSE
  QLOG_INFO () << " Device " << _cur_dev << " last controller position " << _state.CurPos;

  _params.StartPos = _state.CurPos;

  if ( USMC_SetParameters ( _cur_dev, &_params ) )
       return  -1;

  // Then of Course You Need to SaveToFlash
  if ( USMC_SaveParametersToFlash ( _cur_dev ) )
       return  -1;
   return 0;
} 
