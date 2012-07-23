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

#ifndef _DRIVERNEWFOCUS_8750_OL_H_
#define _DRIVERNEWFOCUS_8750_OL_H_

#include <string>
#include <vector>
#include "Driver.h"


// Set namespace
using namespace std;

class DriverNewFocus_8750_Ol : public Driver
{
  public:
 
    // Constructor
   DriverNewFocus_8750_Ol():
      _lastNumberOfPulses(0)
   {}
    DriverNewFocus_8750_Ol(ACCom* commChannel):
       Driver(commChannel)
    {}
    DriverNewFocus_8750_Ol(const DriverNewFocus_8750_Ol& actuatorDrvNewFocus_8750_Ol,
                         ACCom* commChannel):
       Driver(actuatorDrvNewFocus_8750_Ol, commChannel),
       _lastNumberOfPulses(actuatorDrvNewFocus_8750_Ol._lastNumberOfPulses)
    {}

    // Destructor
    virtual ~DriverNewFocus_8750_Ol(){}

    //Copy Constructor
    DriverNewFocus_8750_Ol(const DriverNewFocus_8750_Ol& actuatorDrvNewFocus_8750_Ol ):
       Driver(actuatorDrvNewFocus_8750_Ol),
       _lastNumberOfPulses(actuatorDrvNewFocus_8750_Ol._lastNumberOfPulses) 
    {
    }
  
    //Affectation Operator 
    virtual DriverNewFocus_8750_Ol& operator = (const DriverNewFocus_8750_Ol& actuatorDrvNewFocus_8750_Ol) 
    {
       _pcommChannel       = actuatorDrvNewFocus_8750_Ol._pcommChannel;
       _lastNumberOfPulses = actuatorDrvNewFocus_8750_Ol._lastNumberOfPulses;
       return *this;
    }

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
                    string actuatorSetting,
                    DriverDefinition::ADLimitSwitch& rlimitSwitch) const;
    virtual int GetActuatorFeature(
       DriverDefinition::DriverFeature& ractuatorFeature) const;

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
