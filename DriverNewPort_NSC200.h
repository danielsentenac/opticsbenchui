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

#ifndef _DRIVERNewPort_NSC200_H_
#define _DRIVERNewPort_NSC200_H_

#include <string>
#include <vector>
#include "Driver.h"


// Set namespace
using namespace std;

class DriverNewPort_NSC200 : public Driver
{
  public:
 
    // Constructor
    DriverNewPort_NSC200(){}

    DriverNewPort_NSC200(ACCom* commChannel):
       Driver(commChannel)
    {}
    DriverNewPort_NSC200(const DriverNewPort_NSC200& actuatorDrvNewPort_NSC200,
                        ACCom* commChannel):
       Driver(actuatorDrvNewPort_NSC200, commChannel),
       _originOffset(actuatorDrvNewPort_NSC200._originOffset)
    {}

    // Destructor
    virtual ~DriverNewPort_NSC200(){}

    // Copy Constructor
    DriverNewPort_NSC200(const DriverNewPort_NSC200& actuatorDrvNewPort_NSC200 ):
       Driver(actuatorDrvNewPort_NSC200),
       _originOffset(actuatorDrvNewPort_NSC200._originOffset)
    {
    }
  
    // Affectation Operator 
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
    virtual int SelectedSwitchBoxChannel(string actuatorSetting) const;
  

  protected : 
    static const int BUFFER_SIZE;
    static const int MAX_DEVICES; 
    static const float MAX_VEL; 
    static const int NB_ITEM_INIT_SETTING;
    static const int NB_ITEM_DRV_SETTING;
    static const int MAX_TRIES;
    static const DriverDefinition::ADActuatorFeature NEWPORT_NSC200_FEATURE; 
  
    // NewPort_NSC200 features

    mutable float _originOffset;
    mutable int   _axisNumber;
    mutable float _vel;
    mutable int   _channel;

};
#endif //_DRIVERNewPort_NSC200_H_
