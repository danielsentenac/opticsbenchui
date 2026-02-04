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
 * @file DriverNewFocus_8750_Ol.h
 * @brief Driver implementation for New Focus 8750 Ol.
 */

#ifndef DRIVERNEWFOCUS_8750_OL_H
#define DRIVERNEWFOCUS_8750_OL_H

#include <string>
#include <vector>
#include "Driver.h"


// Set namespace
using namespace std;

/// \ingroup motors
/// NewFocus 8750 open-loop Picomotor driver implementation.
class DriverNewFocus_8750_Ol : public Driver
{
  public:
 
    /// Default constructor.
   DriverNewFocus_8750_Ol():
      _lastNumberOfPulses(0)
   {}
    /// Construct with a communication channel.
    /// \param commChannel Communication channel instance.
    DriverNewFocus_8750_Ol(ACCom* commChannel):
       Driver(commChannel)
    {}
    /// Copy-construct with a new communication channel.
    /// \param actuatorDrvNewFocus_8750_Ol Reference driver to copy.
    /// \param commChannel Communication channel instance.
    DriverNewFocus_8750_Ol(const DriverNewFocus_8750_Ol& actuatorDrvNewFocus_8750_Ol,
                         ACCom* commChannel):
       Driver(actuatorDrvNewFocus_8750_Ol, commChannel),
       _lastNumberOfPulses(actuatorDrvNewFocus_8750_Ol._lastNumberOfPulses)
    {}

    /// Destructor.
    virtual ~DriverNewFocus_8750_Ol(){}

    /// Copy constructor.
    /// \param actuatorDrvNewFocus_8750_Ol Reference driver to copy.
    DriverNewFocus_8750_Ol(const DriverNewFocus_8750_Ol& actuatorDrvNewFocus_8750_Ol ):
       Driver(actuatorDrvNewFocus_8750_Ol),
       _lastNumberOfPulses(actuatorDrvNewFocus_8750_Ol._lastNumberOfPulses) 
    {
    }
  
    /// Assignment operator.
    /// \param actuatorDrvNewFocus_8750_Ol Reference driver to copy.
    virtual DriverNewFocus_8750_Ol& operator = (const DriverNewFocus_8750_Ol& actuatorDrvNewFocus_8750_Ol) 
    {
       _pcommChannel       = actuatorDrvNewFocus_8750_Ol._pcommChannel;
       _lastNumberOfPulses = actuatorDrvNewFocus_8750_Ol._lastNumberOfPulses;
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
                    string actuatorSetting,
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
    static const int NB_ITEM_SETTING;
    static const int WRITE_ERROR;
    static const int READ_ERROR;
    static const DriverDefinition::DriverFeature NEWFOCUS_8750_OL_FEATURE; 
                                             // open loop Picomotor features
    
    mutable int _lastNumberOfPulses;   // used to emulate absolute position readout

    int SendCommand(string  command,
                    string& answer) const;
};
#endif //_DRIVERNEWFOCUS_8750_OL_H_
