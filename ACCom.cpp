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


#include "ACRSCom.h"
#include "ACEthCom.h"


const string ACCom::RS_COM  = "SERIAL_PORT";
const string ACCom::ETH_COM = "ETHERNET";
const string ACCom::MB_COM  = "MB_COM";
const string ACCom::CAN_COM = "CAN_COM";

ACCom::ACCom (string device, string settings): _device (device), _settings (settings)
{
    _state = CLOSED;
}


// -----------------------------------------------------------------------------
// Operation : Create
//    Instantiates a subclass and returns a pointer on it
//
// @param channelType
//    Channel type to instantiate
//
// -----------------------------------------------------------------------------
ACCom *ACCom::Create (string channelType, string device, string settings)
{
    ACCom *channel = NULL;
    if (channelType == RS_COM)
    {
        channel = new ACRSCom (device, settings);
    }
    else if (channelType == ETH_COM)
    {
        channel = new ACEthCom (device, settings);
    }
    return channel;
}


// -----------------------------------------------------------------------------
// Operation : Create
//    Instantiates a subclass and returns a pointer on it
//
// @param prefChannel
//    pointer on the reference channel
//
// -----------------------------------------------------------------------------
ACCom *ACCom::Create (const ACCom * prefChannel)
{
  ACCom *pchannel = NULL;
  
  if (const ACRSCom * prsChannel =
      dynamic_cast < const ACRSCom * >(prefChannel))
    {
      pchannel = new ACRSCom (*prsChannel);
    }
  else if (const ACEthCom * pethChannel
	   = dynamic_cast < const ACEthCom * >(prefChannel))
    {
      pchannel = new ACEthCom (*pethChannel);
    }
  
  return pchannel;
}
