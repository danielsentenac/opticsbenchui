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
 * @file DriverNewPort_NSC200.h
 * @brief Driver implementation for New Port NSC 200.
 */

#ifndef DRIVERNEWPORT_NSC200_H
#define DRIVERNEWPORT_NSC200_H

#include <string>
#include <vector>
#include "Driver.h"


// Set namespace
using namespace std;

/// \ingroup motors
/// Newport NSC200 actuator driver implementation.
class DriverNewPort_NSC200 : public Driver
{
  public:
 
    /// Default constructor.
    DriverNewPort_NSC200(){}

    /// Construct with a communication channel.
    /// \param commChannel Communication channel instance.
    DriverNewPort_NSC200(ACCom* commChannel):
       Driver(commChannel)
    {}
    /// Copy-construct with a new communication channel.
    /// \param actuatorDrvNewPort_NSC200 Reference driver to copy.
    /// \param commChannel Communication channel instance.
    DriverNewPort_NSC200(const DriverNewPort_NSC200& actuatorDrvNewPort_NSC200,
                        ACCom* commChannel):
       Driver(actuatorDrvNewPort_NSC200, commChannel),
       _originOffset(actuatorDrvNewPort_NSC200._originOffset)
    {}

    /// Destructor.
    virtual ~DriverNewPort_NSC200(){}

    /// Copy constructor.
    /// \param actuatorDrvNewPort_NSC200 Reference driver to copy.
    DriverNewPort_NSC200(const DriverNewPort_NSC200& actuatorDrvNewPort_NSC200 ):
       Driver(actuatorDrvNewPort_NSC200),
       _originOffset(actuatorDrvNewPort_NSC200._originOffset)
    {
    }
  
    /// Assignment operator.
    /// \param actuatorDrvNewPort_NSC200 Reference driver to copy.
    virtual DriverNewPort_NSC200& operator = (
       const DriverNewPort_NSC200& actuatorDrvNewPort_NSC200) 
    {
       _pcommChannel = actuatorDrvNewPort_NSC200._pcommChannel;
       _originOffset = actuatorDrvNewPort_NSC200._originOffset;
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
    /// Select a switchbox channel from the actuator settings.
    /// \param actuatorSetting Settings string.
    virtual int SelectedSwitchBoxChannel(string actuatorSetting) const;
  

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
    /// Newport NSC200 driver features.
    static const DriverDefinition::DriverFeature NEWPORT_NSC200_FEATURE; 
  
    // NewPort_NSC200 features

    /// Origin offset value.
    mutable float _originOffset;
    /// Axis number.
    mutable int   _axisNumber;
    /// Communication delay.
    mutable int   _delay;
    /// Velocity value.
    mutable float _vel;
    /// Switchbox channel.
    mutable int   _channel;

};
#endif //_DRIVERNewPort_NSC200_H_
