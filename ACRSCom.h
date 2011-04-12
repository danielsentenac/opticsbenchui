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


#if !defined(_ACRSCOM_H)
#define _ACRSCOM_H

#include <termios.h>
#include "ACCom.h"

using namespace std;

class ACRSCom:public ACCom
{
public:
  
  /** Constructor
   */
  ACRSCom (string device, string settings):
    ACCom (device, settings),
    _readDelay (DEFAULT_READ_DELAY)
  {
  }
  
  /** Constructor
   */
  ACRSCom (const ACRSCom & channel):
    ACCom (channel),
    _hCom (channel._hCom),
    _commSetup (channel._commSetup),
    _commOldSetup (channel._commOldSetup), _readDelay (channel._readDelay)
  {
  }

  /**
     The Open method allows to open the channel
  */
  virtual int Open ();

  /**
     The Read method allows to read in the channel
     @param message
     The message to be read
     @param ...
     The parametrization must be either of the following:\n
     - NULL : default mode\n
  */
  virtual int Read (string & message, ...);
  
  /**
     The Write method allows to write in the channel
     @param message
     The message to be read
     @param ...
     The parametrization must be either of the following:\n
     - NULL : default mode\n
     - "ECHO" : echo mode writes character by character, each write 
     followed by a single character read.\n 
     The message string must be finished with "\r" character
  */
  virtual int Write (string & message, ...);
  
  /**
     The Close method allows to close the channel
  */
  virtual int Close ();

private:
  
  /** The WriteEcho methods writes to the channel character by character, 
      each write followed by a single character read.
      @param message
      The written message string
  */
  virtual int WriteEcho (string message);
  
  /**
     The default read delay
  */
  static const int DEFAULT_READ_DELAY;

  /**
     The Setup method initialize the RS232 properties
  */
  int Setup ();

  /**
     Internal file descriptor
  */
  int _hCom;

  /**
     current communication setup
  */
  struct termios _commSetup;

  /**
     former communication setup
  */
  struct termios _commOldSetup;

  /** 
      The internal baud rate (can be set by the server configuration)
  */
  int _ispeed;

  /**
     The internal read delay known as vtime parameter 
     (can be set by the server configuration)
  */
  int _readDelay;

  /**
     The internal flow type can be : \n
     - NONE : default flow control\n
     - XONXOFF
  */
  char _flow[32];

};

#endif // !defined(_ACRSCOM_H)
