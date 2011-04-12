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

#ifndef _DRIVERMICOS_POLLUX_H_
#define _DRIVERMICOS_POLLUX_H_

#include <string>
#include <vector>
#include "Driver.h"


// Set namespace
using namespace std;

class DriverMicos_Pollux : public Driver
{
  public:
 
    // Constructor
    DriverMicos_Pollux(){}

    DriverMicos_Pollux(ACCom* commChannel):
       Driver(commChannel)
    {}
    DriverMicos_Pollux(const DriverMicos_Pollux& actuatorDrvMicos_Pollux,
                        ACCom* commChannel):
       Driver(actuatorDrvMicos_Pollux, commChannel),
       _originOffset(actuatorDrvMicos_Pollux._originOffset)
    {}

    // Destructor
    virtual ~DriverMicos_Pollux(){}

    // Copy Constructor
    DriverMicos_Pollux(const DriverMicos_Pollux& actuatorDrvMicos_Pollux ):
       Driver(actuatorDrvMicos_Pollux),
       _originOffset(actuatorDrvMicos_Pollux._originOffset)
    {
    }
  
    // Affectation Operator 
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
       DriverDefinition::ADActuatorFeature& ractuatorFeature) const;
    virtual int ConvertUnit(int unit, 
                            float valueToConvert, 
                            float& rconvertedValue,
                            float& rrange) const;

  protected : 
    static const int BUFFER_SIZE;
    static const int MAX_DEVICES; 
    static const int NB_ITEM_INIT_SETTING;
    static const int NB_ITEM_DRV_SETTING;
    static const DriverDefinition::ADActuatorFeature MICOS_POLLUX_FEATURE; 
                                              // Micos_Pollux features

    int SendAddressCode() const;

    mutable float _originOffset;
    mutable int _axisNumber;

};
#endif //_DRIVERMICOS_POLLUX_H_
