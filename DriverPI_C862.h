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

#ifndef DRIVERPI_C862_H
#define DRIVERPI_C862_H

#include <string>
#include <vector>
#include "Driver.h"


// Set namespace
using namespace std;

class DriverPI_C862 : public Driver
{
  public:
 
    // Constructor
    DriverPI_C862(){}

    DriverPI_C862(ACCom* commChannel):
       Driver(commChannel)
    {}
    DriverPI_C862(const DriverPI_C862& actuatorDrvPI_C862,
                        ACCom* commChannel):
       Driver(actuatorDrvPI_C862, commChannel),
       _originOffset(actuatorDrvPI_C862._originOffset)
    {}

    // Destructor
    virtual ~DriverPI_C862(){}

    // Copy Constructor
    DriverPI_C862(const DriverPI_C862& actuatorDrvPI_C862 ):
       Driver(actuatorDrvPI_C862),
       _originOffset(actuatorDrvPI_C862._originOffset)
    {
    }
  
    // Affectation Operator 
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

  protected : 
    static const int BUFFER_SIZE;
    static const int MAX_DEVICES; 
    static const int NB_ITEM_INIT_SETTING;
    static const int NB_ITEM_DRV_SETTING;
    static const DriverDefinition::DriverFeature PI_C862_FEATURE; 
                                              // Translation stage features

    int SendAddressCode() const;

    mutable float _originOffset;

};
#endif //_DRIVERPI_C862_H_
