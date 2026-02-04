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

/**
 * @file DriverPI_E725.h
 * @brief Driver implementation for PI E 725.
 */

#ifndef DRIVERPI_E725_H
#define DRIVERPI_E725_H

#include "Driver.h"

// Set namespace
using namespace std;

/// \ingroup motors
/// PI E-725 actuator driver implementation.
class DriverPI_E725 : public Driver
{
  public:
 
    /// Default constructor.
    DriverPI_E725(){}
    /// Construct with a communication channel.
    /// \param commChannel Communication channel instance.
    DriverPI_E725(ACCom* commChannel):
       Driver(commChannel) 
    {}
    /// Copy-construct with a new communication channel.
    /// \param actuatorDrvPI_E725 Reference driver to copy.
    /// \param commChannel Communication channel instance.
    DriverPI_E725(const DriverPI_E725& actuatorDrvPI_E725,
                       ACCom* commChannel):
       Driver(actuatorDrvPI_E725, commChannel) 
    {}

    /// Destructor.
    virtual ~DriverPI_E725(){}

    /// Copy constructor.
    /// \param actuatorDrvPI_E725 Reference driver to copy.
    DriverPI_E725(const DriverPI_E725& actuatorDrvPI_E725 ):
       Driver(actuatorDrvPI_E725)
    {
    }
  
    /// Assignment operator.
    /// \param actuatorDrvPI_E725 Reference driver to copy.
    virtual DriverPI_E725& operator = (const DriverPI_E725& actuatorDrvPI_E725) 
    {
       _pcommChannel = actuatorDrvPI_E725._pcommChannel;
       return *this;
    }

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

  private :
    static const int NB_ITEM_DRV_SETTING;
    static const int NB_ITEM_INIT_SETTING;
    static const DriverDefinition::DriverFeature PI_E725_FEATURE; 
                                                // PI_E725 actuator features
    mutable int _delay;
};
#endif //_DRIVERPI_E725_H_
