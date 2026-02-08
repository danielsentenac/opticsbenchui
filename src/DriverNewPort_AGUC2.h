/// @file DriverNewPort_AGUC2.h
/// Class DriverNewPort_AGUC2
///
/// @brief
///  This file provides a class implementing a AGUC2 driver 
///  for a non specified controller using only the custom command  
///  provided by the super class
// who      when     what
// -----------------------------------------------------------------------------
// sentenac   06/10/08 created


#ifndef DRIVERNEWPORT_AGUC2_H
#define DRIVERNEWPORT_AGUC2_H


#include "Driver.h"

/// \ingroup motors
/// Newport AG-UC2 actuator driver implementation.
class DriverNewPort_AGUC2 : public Driver
{
  public:
 
    /// Default constructor.
    DriverNewPort_AGUC2(){}
    
    /// Construct with a communication channel.
    /// \param commChannel Communication channel instance.
    DriverNewPort_AGUC2(ACCom* commChannel):
      Driver(commChannel)
      {};
    /// Copy-construct with a new communication channel.
    /// \param actuatorDrvAGUC2 Reference driver to copy.
    /// \param commChannel Communication channel instance.
      DriverNewPort_AGUC2(const DriverNewPort_AGUC2& actuatorDrvAGUC2,
			 ACCom* commChannel):
	Driver(actuatorDrvAGUC2, commChannel)
	{};

    /// Destructor.
      virtual ~DriverNewPort_AGUC2(){};

    /// Copy constructor.
    /// \param actuatorDrvAGUC2 Reference driver to copy.
    DriverNewPort_AGUC2(const DriverNewPort_AGUC2& actuatorDrvAGUC2 ):
       Driver(actuatorDrvAGUC2)
      {};
  
    /// Assignment operator.
    /// \param actuatorDrvAGUC2 Reference driver to copy.
    virtual DriverNewPort_AGUC2& operator = (
       const DriverNewPort_AGUC2& actuatorDrvAGUC2) 
    {
       _pcommChannel = actuatorDrvAGUC2._pcommChannel;
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

    /// Send a device-specific command string and return the reply.
    /// \param buffer Command buffer.
    /// \param rply Output reply string.
    virtual int SendGeneralCommand(char* buffer,string& rply) const ;
    
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
    /// AGUC2 driver features.
    static const DriverDefinition::DriverFeature AGUC2_FEATURE; 
    
};
#endif //_DRIVERNEWPORTAGUC2_H_
