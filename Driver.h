/*******************************************************************
This file is part of OpticsBenchUI.

OpticsBenchUI is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <string>
#include <vector>
#include <iostream>
#include <math.h>
#include <sstream>

#include "ACCom.h"
#include "DriverDefinition.h"

#include "QsLog.h"
#include "QString"

#define ACTUATORSRVDEBUG 1
// Set namespace
using namespace std;

/// The Class Driver defines an abstract interface for actuator
/// driver implementations. 
///
class Driver
{
  public:

    // 
    // Constant declaration: 
    //
    /// Identifier for the PIezo driver
    static const string PI_C862;
    /// Identifier for the Ethernet-PIcomotor driver
    static const string NEWFOCUS_8750_OL;
    /// Identifier for the Ethernet-Closed-Loop-PIcomotor driver
    static const string NEWFOCUS_8750_CL;
    /// Identifier for the Translation Stage driver
    static const string PI_C509;
    /// Identifier for the Rotor driver
    static const string MICOS_POLLUX;
    /// Identifier for the NSA driver
    static const string NEWPORT_NSC200;
    /// Identifier for the NSA driver
    static const string STANDA_USMC;
    /// Identifier for the NULL driver
    static const string NULL_DRV_ID;
    /// Identifier for the PI_E725 driver
    static const string PI_E725;
    /// Identifier for the NEWPORT_AGUC2
    static const string NEWPORT_AGUC2;

    /// Constructor
    Driver()
    {
       _lastError = DriverDefinition::NO_ERROR;
    }

    /// Constructor with communication channel
    Driver(ACCom* commChannel):
       _pcommChannel(commChannel)
    {
    }

    /// Constructor with driver reference and communication channel
    Driver(const Driver& actuatorDrv,
                  ACCom* commChannel):
       _lastError(actuatorDrv._lastError),
       _setting(actuatorDrv._setting),
       _pcommChannel(commChannel)
    {
    }

    /// Copy Constructor
    Driver(const Driver& actuatorDrv):
       _lastError(actuatorDrv._lastError),
       _setting(actuatorDrv._setting),
       _pcommChannel(actuatorDrv._pcommChannel)
    {}

    /// Destructor
    virtual ~Driver() {}
  
    /// Affectation Operator 
    Driver& operator = (Driver& actuatorDrv) 
    {
       _setting      = actuatorDrv._setting;
       _pcommChannel = actuatorDrv._pcommChannel;
       _lastError    = actuatorDrv._lastError;
       return *this;
    }

    //
    // Methods:
    //

    /// Creates a new driver instance
    static Driver* Create(string driverType,
                                 string setting,
                                 ACCom* channel);
    /// Creates a new driver instance
    static Driver* Create(const Driver* prefActuatorDrv,
                                 ACCom* prefChannel );

    /// Initializes the driver
    virtual int Init(string& rstateData) const = 0;
 
    /// @brief Initializes the hardware device
    /// @param actuatorSetting
    /// @param position
    ///   This string contains the information for the setting to be
    ///   applied during the harwdare initialization
    virtual int InitActuator(string actuatorSetting,float position) const = 0;

    /// @brief Retrieves the current position of the actuator
    /// @param actuatorSetting
    ///   This string contains the settings that have to be applied for
    ///   getting the position
    /// @param position
    ///   Returns the current position
    virtual int GetPos(string actuatorSetting,
		       float& position) const = 0;

    /// @brief Performs a relative movement of the actuator
    /// @param actuatorSetting
    ///   This string contains the settings that have to be applied for
    ///   the moving action
    /// @param nbSteps
    ///   Defines the relative displacement that should be applied
    /// @param unit
    ///   unit used for the motion.0 = default unit,  1 = custom one
    virtual int Move(string actuatorSetting,
		     float nbSteps,
                     int   unit) const = 0;

    /// @brief Performs an absolute movement of the actuator
    /// @param actuatorSetting
    ///   This string contains the settings that have to be applied for
    ///   the moving action
    /// @param absPos
    ///   Defines the absolute position where the actuator should be
    ///   moved to
    /// @param unit
    ///   unit used for the motion.0 = default unit,  1 = custom one
    virtual int MoveAbs(string actuatorSetting,
			float  absPos,
                        int    unit) const = 0;

    /// @brief Stops the current movement of the actuator
    /// @param actuatorSetting
    ///   This string contains the settings that have to be applied for
    ///   stopping the movement
    virtual int Stop(string actuatorSetting) const = 0;

    /// @brief Checks if an operation has been finished or not
    /// @param rstateData
    ///   state data of the driver 
    /// @param actuatorSetting
    ///   This string contains the settings that have to be applied
    /// @param switchLimit
    ///   Reached limit flag
    virtual int OperationComplete(
                 string& rstateData,
                 string actuatorSetting,
                 DriverDefinition::ADLimitSwitch& rlimitSwitch) const = 0;

    /// @gives the code of the last problem
    /// @param lastError
    ///    code of the last error 
    virtual int GetLastError(DriverDefinition::ADErrorCode &lastError);

    /// @gives the features of the steered actuator
    /// @param ractuatorFeature
    ///    steered actuator feature
    virtual int GetActuatorFeature(
       DriverDefinition::DriverFeature& ractuatorFeature) const = 0;

    /// @converts a default unit value into a custom one
    virtual int ConvertUnit(int unit, 
                            float valueToConvert, 
                            float& rconvertedValue,
                            float& rrange) const = 0;
    virtual int SendGeneralCommand(char* buffer,string& rply) const ;
 
 protected :
    
    mutable DriverDefinition::ADErrorCode _lastError; // code of the last error
    string              _setting;         // Driver setting
    ACCom*             _pcommChannel;    // Actuators Communication channel 
    
    void Tokenize(const string& str,
		  vector<string>& tokens,
		  const string& delimiters = " ") const;
};
#endif //_DRIVER_H_
