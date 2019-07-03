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

#ifndef _DRIVERPI_C509_H_
#define _DRIVERPI_C509_H_

#include <string>
#include <vector>
#include "Driver.h"

// Set namespace
using namespace std;

class DriverPI_C509 : public Driver
{
  public:
 
    // Constructor
    DriverPI_C509(){}
    DriverPI_C509(ACCom* commChannel):
       Driver(commChannel) 
    {}
    DriverPI_C509(const DriverPI_C509& actuatorDrvPI_C509,
                       ACCom* commChannel):
       Driver(actuatorDrvPI_C509, commChannel) 
    {}

    // Destructor
    virtual ~DriverPI_C509(){}

    //Copy Constructor
    DriverPI_C509(const DriverPI_C509& actuatorDrvPI_C509 ):
       Driver(actuatorDrvPI_C509)
    {
    }
  
    //Affectation Operator 
    virtual DriverPI_C509& operator = (const DriverPI_C509& actuatorDrvPI_C509) 
    {
       _pcommChannel = actuatorDrvPI_C509._pcommChannel;
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
  
    static const string VALID_ACTUATOR_LIST;  //List of valid actuator setting
    static const DriverDefinition::DriverFeature PI_C509_FEATURE; 
                                                // PI_C509 actuator features
};
#endif //_DRIVERPI_C509_H_
