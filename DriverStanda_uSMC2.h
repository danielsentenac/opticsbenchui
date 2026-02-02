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

class DriverStanda_uSMC2 : public Driver
{
  public:
 
    // Constructor
    DriverStanda_uSMC2(){}
    
    DriverStanda_uSMC2(ACCom* commChannel):
      Driver(commChannel)
      {};
      DriverStanda_uSMC2(const DriverStanda_uSMC2& actuatorDrvUSMC,
			 ACCom* commChannel):
	Driver(actuatorDrvUSMC, commChannel)
	{};

    // Destructor
      virtual ~DriverStanda_uSMC2(){};

    // Copy Constructor
    DriverStanda_uSMC2(const DriverStanda_uSMC2& actuatorDrvUSMC ):
       Driver(actuatorDrvUSMC)
      {};
  
    // Affectation Operator 
    virtual DriverStanda_uSMC2& operator = (
       const DriverStanda_uSMC2& actuatorDrvUSMC) 
    {
       _pcommChannel = actuatorDrvUSMC._pcommChannel;
       return *this;
    };

    //
    // Methods:
    //
    virtual int Init(string& rstateData) const;
    virtual int InitActuator(string actuatorSetting,float position) const ;
    virtual int GetPos(string actuatorSetting, float& position) const ;
    virtual int Move(string actuatorSetting,float nbSteps,int unit) const;
    virtual int MoveAbs(string actuatorSetting, float absPos, int unit) const;
    virtual int Stop(string actuatorSetting) const;
    virtual int OperationComplete(
                     string& rstateData,
                     string  actuatorSetting,
                     DriverDefinition::ADLimitSwitch& rlimitSwitch) const;
    virtual int GetActuatorFeature(
       DriverDefinition::DriverFeature& ractuatorFeature) const;
    virtual int ConvertUnit(int unit, 
                            float valueToConvert, 
                            float& rconvertedValue,
                            float& rrange) const;
    virtual int Exit();
    
  protected : 
    static const int BUFFER_SIZE;
    static const int MAX_DEVICES; 
    static const float MAX_VEL; 
    static const int NB_ITEM_INIT_SETTING;
    static const int NB_ITEM_DRV_SETTING;
    static const int MAX_TRIES;
    static const int MIN_BYTES_TRANS;
    static const DriverDefinition::DriverFeature USMC2_FEATURE; 

    mutable map<int,device_t> _device;
    mutable device_enumeration_t _devenum;
    mutable stage_name_t _stage_name;
    mutable move_settings_t _move_settings;
    mutable engine_settings_t _engine_settings;
    mutable status_t _status;
    mutable status_calb_t _status_calb;
    mutable calibration_t _calibration;
};
#endif //_DRIVERNEWPORTUSMC_H_
