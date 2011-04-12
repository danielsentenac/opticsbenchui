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


#ifndef ACCom_H
#define ACCom_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdarg.h>

#include "QsLog.h"
#include <QString>

#define ADVCOMDEBUG 2
#define STRLENGTH 1024

using namespace std;

class ACCom
{
public:

  /**
     The state values of the channel communication
  */
  enum { OPEN, CLOSED };
  /** The serial port protocol code name */
  static const string RS_COM;
  /** The ethernet protocol code name */
  static const string ETH_COM;
  /** The modbus protocol code name */
  static const string MB_COM;
  /** The canbus protocol code name */
  static const string CAN_COM;

  /** Constructor
      @param device
      The device name or address
      @param settings
      The setting string as set in the configuration file
  */
  ACCom (string device, string settings);

  /** Copy Constructor
      @param channel
      The object to be copyied
  */
  ACCom (const ACCom & channel):
    _state (channel._state),
    _device (channel._device), _settings (channel._settings)
  {
  }

  /**
     Destructor
  */
  virtual ~ ACCom ()
  {
  };

  /**
     The Create method is the factory instantiator of the communication object
     @param channelType
     The type of channel to be instantiated (SERIAL,ETHERNET MB_COM,CAN_COM)
     @param device
     The device name or address as set in the configuration file
     @param settings
     The settings string as set in the configuration file
  */
  static ACCom *Create (string channelType, string device, string settings);

  /**
     Copy factory instantiator
     @param prefChannel
     The communication object to be instantiated
  */
  static ACCom *Create (const ACCom * prefChannel);

  /**
     The Open method allows to open the channel communication port
  */
  virtual int Open () = 0;

  /**
     The Read method allows to read in the channel communication port
     @param message
     The message to be read
     @param ...
     Various parameters depending on the type of protocol used
  */
  virtual int Read (string & message, ...) = 0;
  
  /**
     The Write method allows to write in the channel communication port
     @param message
     The message to be written
     @param ...
     Various parameters depending on the type of protocol used
     
  */
  virtual int Write (string & message, ...) = 0;

  /**
     The Close method allows to close the channel communication port
  */
  virtual int Close () = 0;

  /**
     The GetStatus returns the channel status (OPEN or CLOSE)
  */
  virtual int GetStatus () const
  {
    return _state;
  }

protected:

  /**
     The internal state of the channel communication
  */
  int _state;

  /**
     The internal device name of the channel communication
  */
  string _device;

  /**
     The internal settings of the channel communication
  */
  string _settings;
};
#endif                                            // ACCom_H
