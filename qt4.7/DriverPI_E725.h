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

#ifndef _DRIVERPI_E725_H_
#define _DRIVERPI_E725_H_

#include "Driver.h"

// Set namespace
using namespace std;

class DriverPI_E725 : public Driver
{
  public:
 
    // Constructor
    DriverPI_E725(){}
    DriverPI_E725(ACCom* commChannel):
       Driver(commChannel) 
    {}
    DriverPI_E725(const DriverPI_E725& actuatorDrvPI_E725,
                       ACCom* commChannel):
       Driver(actuatorDrvPI_E725, commChannel) 
    {}

    // Destructor
    virtual ~DriverPI_E725(){}

    //Copy Constructor
    DriverPI_E725(const DriverPI_E725& actuatorDrvPI_E725 ):
       Driver(actuatorDrvPI_E725)
    {
    }
  
    //Affectation Operator 
    virtual DriverPI_E725& operator = (const DriverPI_E725& actuatorDrvPI_E725) 
    {
       _pcommChannel = actuatorDrvPI_E725._pcommChannel;
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
       DriverDefinition::DriverFeature& ractuatorFeature) const;
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
