/**
 * @file ACRSCom.h
 * @brief AC communication backend for ACRS Com.
 */
/// &file ACRSCom.hpp
/// Class ACRSCom
/// This class provides a set of operations to perform communications
/// via a serial port.

// who      when     what
// ----------------------------------------------------------------
// C.MOINS  20/07/06  created

#if !defined(_ACRSCOM_H)
#define _ACRSCOM_H

#include <cstring>

#ifdef Q_OS_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <termios.h>
#endif
#include "ACCom.h"

using namespace std;

/// RS-232 serial communication channel.
class ACRSCom:public ACCom
{
public:
  
  /** Constructor
   */
  ACRSCom (string device, string settings):
    ACCom (device, settings),
    _readDelay (DEFAULT_READ_DELAY)
  {
    _ispeed = 0;
    _flow[0] = '\0';
#ifdef Q_OS_WIN
    _hCom = INVALID_HANDLE_VALUE;
#else
    _hCom = -1;
#endif
  }
  
  /** Constructor
   */
  ACRSCom (const ACRSCom & channel):
    ACCom (channel),
#ifdef Q_OS_WIN
    _hCom (channel._hCom),
    _readDelay (channel._readDelay),
    _ispeed (channel._ispeed)
#else
    _hCom (channel._hCom),
    _commSetup (channel._commSetup),
    _commOldSetup (channel._commOldSetup),
    _readDelay (channel._readDelay),
    _ispeed (channel._ispeed)
#endif
  {
    _flow[0] = '\0';
    strncpy(_flow, channel._flow, sizeof(_flow) - 1);
    _flow[sizeof(_flow) - 1] = '\0';
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
#ifdef Q_OS_WIN
  HANDLE _hCom;
#else
  int _hCom;
#endif

  /**
     current communication setup
  */
#ifndef Q_OS_WIN
  struct termios _commSetup;

  /**
     former communication setup
  */
  struct termios _commOldSetup;
#endif

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
