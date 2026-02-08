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
 * @file DriverMicos_Pollux.h
 * @brief Driver implementation for Micos Pollux.
 */

#ifndef DRIVERMICOS_POLLUX_H
#define DRIVERMICOS_POLLUX_H

#include <string>
#include <vector>
#include "Driver.h"


// Set namespace
using namespace std;

/// \ingroup motors
/// Micos Pollux actuator driver implementation.
class DriverMicos_Pollux : public Driver
{
  public:
 
    /// Default constructor.
    DriverMicos_Pollux(){}

    /// Construct with a communication channel.
    /// \param commChannel Communication channel instance.
    DriverMicos_Pollux(ACCom* commChannel):
       Driver(commChannel)
    {}
    /// Copy-construct with a new communication channel.
    /// \param actuatorDrvMicos_Pollux Reference driver to copy.
    /// \param commChannel Communication channel instance.
    DriverMicos_Pollux(const DriverMicos_Pollux& actuatorDrvMicos_Pollux,
                        ACCom* commChannel):
       Driver(actuatorDrvMicos_Pollux, commChannel),
       _originOffset(actuatorDrvMicos_Pollux._originOffset)
    {}

    /// Destructor.
    virtual ~DriverMicos_Pollux(){}

    /// Copy constructor.
    /// \param actuatorDrvMicos_Pollux Reference driver to copy.
    DriverMicos_Pollux(const DriverMicos_Pollux& actuatorDrvMicos_Pollux ):
       Driver(actuatorDrvMicos_Pollux),
       _originOffset(actuatorDrvMicos_Pollux._originOffset)
    {
    }
  
    /// Assignment operator.
    /// \param actuatorDrvMicos_Pollux Reference driver to copy.
    virtual DriverMicos_Pollux& operator = (
       const DriverMicos_Pollux& actuatorDrvMicos_Pollux) 
    {
       _pcommChannel = actuatorDrvMicos_Pollux._pcommChannel;
       _originOffset = actuatorDrvMicos_Pollux._originOffset;
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

  protected : 
    /// Serial buffer size.
    static const int BUFFER_SIZE;
    /// Maximum device count.
    static const int MAX_DEVICES; 
    /// Number of init settings items.
    static const int NB_ITEM_INIT_SETTING;
    /// Number of driver settings items.
    static const int NB_ITEM_DRV_SETTING;
    /// Micos Pollux features.
    static const DriverDefinition::DriverFeature MICOS_POLLUX_FEATURE; 

    /// Send device address code.
    int SendAddressCode() const;

    /// Origin offset value.
    mutable float _originOffset;
    /// Axis number.
    mutable int _axisNumber;

};
#endif //_DRIVERMICOS_POLLUX_H_
