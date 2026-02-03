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


#if !defined(_ACETHCOM_H_)
#define _ACETHCOM_H_

#include <sys/select.h>
#include "ACCom.h"

using namespace std;

/// Ethernet-based communication channel.
class ACEthCom:public ACCom
{
public:

  /** Constructor
   */
  ACEthCom (string device, string settings):
    ACCom (device, settings),
    _readDelay (DEFAULT_READ_DELAY), _writeDelay (DEFAULT_WRITE_DELAY)
  {
    FD_ZERO (&_master);
  }

  /** Constructor
   */
  ACEthCom (const ACEthCom & channel):
    ACCom (channel),
    _sock (channel._sock),
    _master (channel._master),
    _readDelay (channel._readDelay), _writeDelay (channel._writeDelay)
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
       - "DELAY",delay (usec) : using a specific delay before reading
    */
  virtual int Read (string & message, ...);
  
  /**
       The Write method allows to write in the channel
       @param message
       The message to be read
       @param ...
       The parametrization must be either of the following:\n
       - NULL : default mode (default includes NULL terminal character)\n
       - "RAW" : does not include NULL terminal character\n
       - "ECHO" : echo mode writes character by character, each write 
       followed by a single character read.
       The message string must be finished with "\r" character
    */
  virtual int Write (string & message, ...);
  
  /**
     The Close method allows to close the channel
  */
  virtual int Close ();
      
private:
      
  /** The ReadDelay reads with a delay
      @param message
      The read message string
      @param usec
      Delay in microseconds
   */
  virtual int ReadDelay (string & message, int usec);

  /** The WriteEcho methods writes to the channel character by character, 
      each write followed by a single character read.
      @param message
      The written message string
  */
  virtual int WriteEcho (string message);

  /** The WriteRaw methods writes to the channel without additional NULL
      character added.
      @param message
      The written message string
  */
  virtual int WriteRaw (string message);
  
  /**
     The default read delay
  */
  static const float DEFAULT_READ_DELAY;
  /**
     The default write delay
  */
  static const float DEFAULT_WRITE_DELAY;
   
  /**
     The initial socket file descriptor
  */
  int _sock;
  
  /**
     The internal file descriptor set
  */
  fd_set _master;
  /**
     The internal read delay
  */
  float _readDelay;
  /**
     The internal write delay
  */
  float _writeDelay;
    
};

#endif // !defined(_ACETHCOM_H_)
