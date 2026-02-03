/// @file DriverStanda_uSMC2.h
/// Class DriverStanda_uSMC2
///
/// @brief
///  This file provides a class implementing a USMC driver 
///  for a non specified controller using only the custom command  
///  provided by the super class
// who      when     what
// -----------------------------------------------------------------------------
// sentenac   06/10/08 created


#ifndef DRIVERSTANDA_USMC2_H
#define DRIVERSTANDA_USMC2_H


#include "Driver.h"
#include "ximc.h"

/// \ingroup motors
/// Standa uSMC2 actuator driver implementation.
class DriverStanda_uSMC2 : public Driver
{
  public:
 
    /// Default constructor.
    DriverStanda_uSMC2(){}
    
    /// Construct with a communication channel.
    /// \param commChannel Communication channel instance.
    DriverStanda_uSMC2(ACCom* commChannel):
      Driver(commChannel)
      {};
      /// Copy-construct with a new communication channel.
      /// \param actuatorDrvUSMC Reference driver to copy.
      /// \param commChannel Communication channel instance.
      DriverStanda_uSMC2(const DriverStanda_uSMC2& actuatorDrvUSMC,
			 ACCom* commChannel):
	Driver(actuatorDrvUSMC, commChannel)
	{};

    /// Destructor.
      virtual ~DriverStanda_uSMC2(){};

    /// Copy constructor.
    /// \param actuatorDrvUSMC Reference driver to copy.
    DriverStanda_uSMC2(const DriverStanda_uSMC2& actuatorDrvUSMC ):
       Driver(actuatorDrvUSMC)
      {};
  
    /// Assignment operator.
    /// \param actuatorDrvUSMC Reference driver to copy.
    virtual DriverStanda_uSMC2& operator = (
       const DriverStanda_uSMC2& actuatorDrvUSMC) 
    {
       _pcommChannel = actuatorDrvUSMC._pcommChannel;
       return *this;
    };

    //
    // Methods:
    //
    /// Initialize the driver.
    virtual int Init(string& rstateData) const;
    /// Initialize the actuator with settings and an optional position.
    virtual int InitActuator(string actuatorSetting,float position) const ;
    /// Read the current actuator position.
    virtual int GetPos(string actuatorSetting, float& position) const ;
    /// Perform a relative motion.
    virtual int Move(string actuatorSetting,float nbSteps,int unit) const;
    /// Perform an absolute motion.
    virtual int MoveAbs(string actuatorSetting, float absPos, int unit) const;
    /// Stop the actuator.
    virtual int Stop(string actuatorSetting) const;
    /// Check whether the last operation has completed.
    virtual int OperationComplete(
                     string& rstateData,
                     string  actuatorSetting,
                     DriverDefinition::ADLimitSwitch& rlimitSwitch) const;
    /// Get actuator feature metadata.
    virtual int GetActuatorFeature(
       DriverDefinition::DriverFeature& ractuatorFeature) const;
    /// Convert between default and custom units.
    virtual int ConvertUnit(int unit, 
                            float valueToConvert, 
                            float& rconvertedValue,
                            float& rrange) const;
    /// Shutdown hook for the driver.
    virtual int Exit();
    
  protected : 
    /// Serial buffer size.
    static const int BUFFER_SIZE;
    /// Maximum device count.
    static const int MAX_DEVICES; 
    /// Maximum velocity.
    static const float MAX_VEL; 
    /// Number of init settings items.
    static const int NB_ITEM_INIT_SETTING;
    /// Number of driver settings items.
    static const int NB_ITEM_DRV_SETTING;
    /// Maximum retry count.
    static const int MAX_TRIES;
    /// Minimum bytes per transfer.
    static const int MIN_BYTES_TRANS;
    /// USMC2 driver features.
    static const DriverDefinition::DriverFeature USMC2_FEATURE; 

    /// Device map.
    mutable map<int,device_t> _device;
    /// Device enumeration handle.
    mutable device_enumeration_t _devenum;
    /// Stage name.
    mutable stage_name_t _stage_name;
    /// Move settings.
    mutable move_settings_t _move_settings;
    /// Engine settings.
    mutable engine_settings_t _engine_settings;
    /// Status.
    mutable status_t _status;
    /// Calibration status.
    mutable status_calb_t _status_calb;
    /// Calibration data.
    mutable calibration_t _calibration;
};
#endif //_DRIVERNEWPORTUSMC_H_
