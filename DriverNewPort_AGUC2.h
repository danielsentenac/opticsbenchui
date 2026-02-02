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

class DriverNewPort_AGUC2 : public Driver
{
  public:
 
    // Constructor
    DriverNewPort_AGUC2(){}
    
    DriverNewPort_AGUC2(ACCom* commChannel):
      Driver(commChannel)
      {};
      DriverNewPort_AGUC2(const DriverNewPort_AGUC2& actuatorDrvAGUC2,
			 ACCom* commChannel):
	Driver(actuatorDrvAGUC2, commChannel)
	{};

    // Destructor
      virtual ~DriverNewPort_AGUC2(){};

    // Copy Constructor
    DriverNewPort_AGUC2(const DriverNewPort_AGUC2& actuatorDrvAGUC2 ):
       Driver(actuatorDrvAGUC2)
      {};
  
    // Affectation Operator 
    virtual DriverNewPort_AGUC2& operator = (
       const DriverNewPort_AGUC2& actuatorDrvAGUC2) 
    {
       _pcommChannel = actuatorDrvAGUC2._pcommChannel;
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

    virtual int SendGeneralCommand(char* buffer,string& rply) const ;
    
  protected : 
    static const int BUFFER_SIZE;
    static const int MAX_DEVICES; 
    static const float MAX_VEL; 
    static const int NB_ITEM_INIT_SETTING;
    static const int NB_ITEM_DRV_SETTING;
    static const int MAX_TRIES;
    static const int MIN_BYTES_TRANS;
    static const DriverDefinition::DriverFeature AGUC2_FEATURE; 
    
};
#endif //_DRIVERNEWPORTAGUC2_H_
