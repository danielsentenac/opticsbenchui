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

	/// \ingroup motors
	/// Standa uSMC actuator driver implementation.
	class DriverStanda_uSMC : public Driver
	{
	  public:
	 
	    /// Default constructor.
	    DriverStanda_uSMC(){}
	    
	    /// Construct with a communication channel.
	    /// \param commChannel Communication channel instance.
	    DriverStanda_uSMC(ACCom* commChannel):
	      Driver(commChannel)
	      {};
	      /// Copy-construct with a new communication channel.
	      /// \param driverStanda_uSMC Reference driver to copy.
	      /// \param commChannel Communication channel instance.
	      DriverStanda_uSMC(const DriverStanda_uSMC& driverStanda_uSMC,
				 ACCom* commChannel):
		Driver(driverStanda_uSMC, commChannel)
		{};

	    /// Destructor.
	      virtual ~DriverStanda_uSMC(){};

	    /// Copy constructor.
	    /// \param driverStanda_uSMC Reference driver to copy.
	    DriverStanda_uSMC(const DriverStanda_uSMC& driverStanda_uSMC ):
	       Driver(driverStanda_uSMC)
	      {};
	  
	    /// Assignment operator.
	    /// \param driverStanda_uSMC Reference driver to copy.
	    virtual DriverStanda_uSMC& operator = (
	       const DriverStanda_uSMC& driverStanda_uSMC) 
	    {
	       _pcommChannel = driverStanda_uSMC._pcommChannel;
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
            /// \param actuatorSetting Settings string.
            virtual int Exit(string actuatorSetting);

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
	    /// Standa USMC driver features.
	    static const DriverDefinition::DriverFeature STANDA_USMC_FEATURE;
	    /// Devices handle.
	    mutable USMC_Devices _devices;
	    /// Current device index.
	    mutable DWORD _cur_dev;
	    /// Current device state.
	    mutable USMC_State _state;
	    /// Start parameters.
	    mutable USMC_StartParameters _start_params;
	    /// Device parameters.
	    mutable USMC_Parameters _params;
	    /// Device mode.
	    mutable USMC_Mode _mode;
	    /// Encoder state.
	    mutable USMC_EncoderState _enc_state;
	    /// Current speed value.
	    mutable float _speed;
            /// Rotator flag/value.
            mutable int _rotator;
	    /// Divisor value.
	    mutable int _divisor;

	};
	#endif //_DRIVERSTANDAUSMC_H_
