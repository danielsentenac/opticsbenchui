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
 * @file DriverPI_C862.h
 * @brief Driver implementation for PI C 862.
 */

#ifndef DRIVERPI_C862_H
#define DRIVERPI_C862_H

#include <string>
#include <vector>
#include "Driver.h"


// Set namespace
using namespace std;

/// \ingroup motors
/// PI C-862 actuator driver implementation.
class DriverPI_C862 : public Driver
{
  public:
 
    /// Default constructor.
    DriverPI_C862(){}

    /// Construct with a communication channel.
    /// \param commChannel Communication channel instance.
    DriverPI_C862(ACCom* commChannel):
       Driver(commChannel)
    {}
    /// Copy-construct with a new communication channel.
    /// \param actuatorDrvPI_C862 Reference driver to copy.
    /// \param commChannel Communication channel instance.
    DriverPI_C862(const DriverPI_C862& actuatorDrvPI_C862,
                        ACCom* commChannel):
       Driver(actuatorDrvPI_C862, commChannel),
       _originOffset(actuatorDrvPI_C862._originOffset)
    {}

    /// Destructor.
    virtual ~DriverPI_C862(){}

    /// Copy constructor.
    /// \param actuatorDrvPI_C862 Reference driver to copy.
    DriverPI_C862(const DriverPI_C862& actuatorDrvPI_C862 ):
       Driver(actuatorDrvPI_C862),
       _originOffset(actuatorDrvPI_C862._originOffset)
    {
    }
  
    /// Assignment operator.
    /// \param actuatorDrvPI_C862 Reference driver to copy.
    virtual DriverPI_C862& operator = (
       const DriverPI_C862& actuatorDrvPI_C862) 
    {
       _pcommChannel = actuatorDrvPI_C862._pcommChannel;
       _originOffset = actuatorDrvPI_C862._originOffset;
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
    /// Translation stage features.
    static const DriverDefinition::DriverFeature PI_C862_FEATURE; 

    /// Send device address code.
    int SendAddressCode() const;

    /// Origin offset value.
    mutable float _originOffset;

};
#endif //_DRIVERPI_C862_H_
