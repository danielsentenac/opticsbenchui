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
#ifndef NO_USB
#include "ACUsbCom.h"
#endif
#include "Utils.h"


const string ACCom::RS_COM  = "SERIAL_PORT";
const string ACCom::ETH_COM = "ETHERNET";
const string ACCom::MB_COM  = "MB_COM";
const string ACCom::CAN_COM = "CAN_COM";
const string ACCom::USB_COM = "USB_COM";
const string ACCom::NULL_COM = "NULL_COM";

ACCom::ACCom (string device, string settings): _device (device), _settings (settings)
{
    _state = CLOSED;
}

void ACCom::ReportWarning(const QString& message) const
{
    Utils::ReportWarning(NULL, message);
}

void ACCom::ReportError(const QString& message) const
{
    Utils::ReportError(NULL, message);
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
#ifndef NO_USB
    else if (channelType == USB_COM)
    {
        channel = new ACUsbCom (device, settings);
    }
#else
    else if (channelType == USB_COM)
    {
        Utils::ReportWarning(NULL, "ACCom::Create> USB_COM requested but USB support is disabled.");
    }
#endif
    else if (channelType == NULL_COM)
    {
        channel = (ACCom*)(-1);
    }
    else
    {
        Utils::ReportWarning(NULL, QString("ACCom::Create> Unknown channel type %1")
                                   .arg(QString(channelType.c_str())));
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
#ifndef NO_USB
  else if (const ACUsbCom * pethChannel
           = dynamic_cast < const ACUsbCom * >(prefChannel))
    {
      pchannel = new ACUsbCom (*pethChannel);
    }
#endif

  return pchannel;
}
