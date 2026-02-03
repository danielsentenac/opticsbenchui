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

#ifndef DRIVERNEWFOCUS_8750_CL_H
#define DRIVERNEWFOCUS_8750_CL_H

#include <string>
#include <vector>
#include "Driver.h"

// Set namespace
using namespace std;

/// \ingroup motors
/// NewFocus 8750 closed-loop Picomotor driver implementation.
class DriverNewFocus_8750_Cl : public Driver
{
  public:
 
    /// Default constructor.
    DriverNewFocus_8750_Cl(){}
    /// Construct with a communication channel.
    /// \param commChannel Communication channel instance.
    DriverNewFocus_8750_Cl(ACCom* commChannel):
       Driver(commChannel) 
    {}
    /// Copy-construct with a new communication channel.
    /// \param actuatorDrvNewFocus_8750_Cl Reference driver to copy.
    /// \param commChannel Communication channel instance.
    DriverNewFocus_8750_Cl(const DriverNewFocus_8750_Cl& actuatorDrvNewFocus_8750_Cl ,
                           ACCom* commChannel):
       Driver(actuatorDrvNewFocus_8750_Cl, commChannel) 
    {}

    /// Destructor.
    virtual ~DriverNewFocus_8750_Cl(){}

    /// Copy constructor.
    /// \param actuatorDrvNewFocus_8750_Cl Reference driver to copy.
    DriverNewFocus_8750_Cl(const DriverNewFocus_8750_Cl& actuatorDrvNewFocus_8750_Cl ):
       Driver(actuatorDrvNewFocus_8750_Cl)
    {
    }
  
    /// Assignment operator.
    /// \param actuatorDrvNewFocus_8750_Cl Reference driver to copy.
    virtual DriverNewFocus_8750_Cl& operator = 
         (const DriverNewFocus_8750_Cl& actuatorDrvNewFocus_8750_Cl) 
    {
       _pcommChannel = actuatorDrvNewFocus_8750_Cl._pcommChannel;
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
    virtual int MoveAbs(string actuatorSetting, float absPos,int unit) const;
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
    static const int NB_ITEM_SETTING;   
    static const int WRITE_ERROR;
    static const int READ_ERROR;
    static const DriverDefinition::DriverFeature 
                    NEWFOCUS_8750_CL_FEATURE; // closed loop Picomotor features

    int SendCommand(string  command,
                    string& answer) const;

};
#endif //_DRIVERETHNEWFOCUS_8750_CL_H_
