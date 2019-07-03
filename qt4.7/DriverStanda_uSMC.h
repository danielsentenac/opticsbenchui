	/// @file DriverStanda_uSMC.h
	/// Class DriverStanda_uSMC
	///
	/// @brief
	///  This file provides a class implementing a AGUC2 driver 
	///  for a non specified controller using only the custom command  
	///  provided by the super class
	// who      when     what
	// -----------------------------------------------------------------------------
	// sentenac   06/10/08 created


	#ifndef _DRIVERSTANDAUSMC_H_
	#define _DRIVERSTANDAUSMC_H_

	extern "C" {
	#include "libusmc.h"
	}
	#include "Driver.h"

	class DriverStanda_uSMC : public Driver
	{
	  public:
	 
	    // Constructor
	    DriverStanda_uSMC(){}
	    
	    DriverStanda_uSMC(ACCom* commChannel):
	      Driver(commChannel)
	      {};
	      DriverStanda_uSMC(const DriverStanda_uSMC& driverStanda_uSMC,
				 ACCom* commChannel):
		Driver(driverStanda_uSMC, commChannel)
		{};

	    // Destructor
	      virtual ~DriverStanda_uSMC(){};

	    // Copy Constructor
	    DriverStanda_uSMC(const DriverStanda_uSMC& driverStanda_uSMC ):
	       Driver(driverStanda_uSMC)
	      {};
	  
	    // Affectation Operator 
	    virtual DriverStanda_uSMC& operator = (
	       const DriverStanda_uSMC& driverStanda_uSMC) 
	    {
	       _pcommChannel = driverStanda_uSMC._pcommChannel;
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
            virtual int Exit(string actuatorSetting);

	  protected : 
	    static const int BUFFER_SIZE;
	    static const int MAX_DEVICES; 
	    static const float MAX_VEL; 
	    static const int NB_ITEM_INIT_SETTING;
	    static const int NB_ITEM_DRV_SETTING;
	    static const int MAX_TRIES;
	    static const int MIN_BYTES_TRANS;
	    static const DriverDefinition::DriverFeature STANDA_USMC_FEATURE;
	    mutable USMC_Devices _devices;
	    mutable DWORD _cur_dev;
	    mutable USMC_State _state;
	    mutable USMC_StartParameters _start_params;
	    mutable USMC_Parameters _params;
	    mutable USMC_Mode _mode;
	    mutable USMC_EncoderState _enc_state;
	    mutable float _speed;
            mutable int _rotator;
	    mutable int _divisor;

	};
	#endif //_DRIVERSTANDAUSMC_H_
