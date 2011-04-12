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


#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ACRSCom.h"


const int ACRSCom::DEFAULT_READ_DELAY = 20;

// ----------------------------------------------------------------------------
/// Operation : Open
/// Opens the communication channel by opening 
/// the file associated to the serial port 
/// and performs a setup of the communication according 
/// to the selected settings.
// ----------------------------------------------------------------------------
int
ACRSCom::Open ()
{
  int status = -1;

  sscanf (_settings.c_str (), "%d,%d,%s", &_ispeed, &_readDelay,_flow);
  _hCom = open (_device.c_str (), O_RDWR);
  if (_hCom == -1)
    {
      _state = CLOSED;
    }
  else
    {
      if (Setup () != -1)
	{
	  status = 0;
	  _state = OPEN;
	}
    }
  return status;
}
// ----------------------------------------------------------------------------
/// Operation : WriteEcho
///    sends data through the socket
/// @param message
///    message to send
// ----------------------------------------------------------------------------
int
ACRSCom::WriteEcho (string message)
{
  int i = 0;
  char Reply[STRLENGTH];
  char inMessage[STRLENGTH];
  strcpy(inMessage,message.c_str());
  do 
    {
      write(_hCom, &inMessage[i], 1);
      read(_hCom, &Reply[i], 1);
    }
  while(inMessage[i++] != '\r');
  return i;
}
// ----------------------------------------------------------------------------
/// Operation : Write
/// Writes datas on the serial port. Returns the number of bytes written
/// @param message
///    data to send on the serial port
// ----------------------------------------------------------------------------
int
ACRSCom::Write (string & message, ...)
{
  va_list args;
  /*--------------------------------------------------------------------------*/
  va_start( args, (&message)[0] );
  char * typewrite = va_arg(args,char*);
  if (typewrite && (sizeof(typewrite) == sizeof(char*))) {
   if (!strcmp(typewrite,"ECHO")) {
     QLOG_DEBUG ( ) << "ACRSCom::Write> WRITE type = " <<  QString(typewrite);
      va_end(args);
      return (WriteEcho(message));
    }
  }
  va_end(args);
  return (write (_hCom, message.c_str (), (message.length ())));
}

// ----------------------------------------------------------------------------
/// Operation : Read
/// Reads datas on the serial port. Returns the number of read characters.
/// @param message
///    data read on the serial port
// ----------------------------------------------------------------------------
int
ACRSCom::Read (string & message, ...)
{
  char dataChar;
  int nbCharRead = 0;
  
  message.erase();
  do
    {
      if (read (_hCom, &dataChar, 1) > 0)
	{
	  message += dataChar;
	  nbCharRead++;
	}
      else
	{
	  break;
	}
    }
  while ((dataChar != '\n'));
  return nbCharRead;

}

// ----------------------------------------------------------------------------
/// Operation : Close
/// Closes the communication by closing 
/// the file associated to the serial port. 
// ----------------------------------------------------------------------------
int
ACRSCom::Close ()
{
  int status = 0;
  if (_state == OPEN)
    {
      tcsetattr (_hCom, TCSANOW, &_commOldSetup);
    }
  if (close (_hCom))
    {
      status = -1;
    }
  else
    {
      _state = CLOSED;
    }
  return status;
}

// ----------------------------------------------------------------------------
/// Operation : Setup
/// Setup the serial communication 
// ----------------------------------------------------------------------------
int
ACRSCom::Setup ()
{
  int status = 0;
  speed_t speed;

  switch (_ispeed)
    {
    case 1200:
      speed = B1200;
      break;
    case 2400:
      speed = B2400;
      break;
    case 4800:
      speed = B4800;
      break;
    case 9600:
      speed = B9600;
      break;
    case 19200:
      speed = B19200;
      break;
    case 38400:
      speed = B38400;
      break;
    case 115200:
      speed = B115200;
      break;
    default:
      throw "Bad speed!";
    }
  tcgetattr (_hCom, &_commOldSetup);	/* save current port settings */
  _commSetup = _commOldSetup;

  if (cfsetispeed (&_commSetup, speed) == -1 ||
      cfsetospeed (&_commSetup, speed) == -1)
    {
      Close ();
      status = -1;
    }

  else
    {
      /* input modes */

      _commSetup.c_iflag &= ~INPCK;	/* parity checking disabled */
      _commSetup.c_iflag &= ~IGNPAR;	/* don't ignore bytes with bad parity */
      _commSetup.c_iflag &= ~PARMRK;	/* don't mark bytes with bad parity */
      _commSetup.c_iflag &= ~ISTRIP;	/* use 8 bit bytes */
      _commSetup.c_iflag |= IGNBRK;	/* ignore break conditions */
      _commSetup.c_iflag &= ~BRKINT;	/* don't generate signals on break cond. */
      _commSetup.c_iflag &= ~IGNCR;	/* don't ignore carriage return */
      _commSetup.c_iflag &= ~ICRNL;	/* don't translate \r to \n */
      _commSetup.c_iflag &= ~INLCR;	/* don't translate \n to \r */
      if (!strcmp(_flow,"XONXOFF")) {
	QLOG_DEBUG ( ) << "RS232 uses XONXOFF flow control";
	_commSetup.c_iflag |= ~IXOFF;	/* enable */
	_commSetup.c_iflag |= ~IXON;	/* enable */
      }
      else {
	QLOG_DEBUG ( ) << "RS232 uses defaut (NONE) flow control";
	_commSetup.c_iflag &= ~IXOFF;	/* disable */
	_commSetup.c_iflag &= ~IXON;	/* disable */
      }
      _commSetup.c_iflag &= ~IXANY;	/* disable */
      _commSetup.c_iflag &= ~IMAXBEL;	/* disable */

      /* output modes */

      _commSetup.c_oflag &= ~OPOST;	/* don't process output characters */

      /* control modes */

      _commSetup.c_cflag &= ~HUPCL;	/* don't generate a modem disconnect */
      _commSetup.c_cflag |= CLOCAL;	/* ignore modem status lines */
      _commSetup.c_cflag |= CREAD;	/* input can be read from the terminal */
      QLOG_DEBUG ( ) << "RS232 uses 8 data bits";
      _commSetup.c_cflag |= CS8;	/* use 8 bits per byte */
      QLOG_DEBUG ( ) << "RS232 uses 1 stop bit";
      _commSetup.c_cflag &= ~CSTOPB;	/* use only one stop bit */
      QLOG_DEBUG ( ) << "RS232 uses no parity";
      _commSetup.c_cflag &= ~PARENB;	/* disable parity bit */
      _commSetup.c_cflag &= ~PARODD;	/* use even parity */
      /* local modes */

      _commSetup.c_lflag &= ~ICANON;	/* disable canonical input process. mode */
      _commSetup.c_lflag &= ~ECHO;	/* disable echoing of input characters */
      _commSetup.c_lflag &= ~ECHOE;	/* don't echo erase */
      _commSetup.c_lflag &= ~ECHOPRT;
      _commSetup.c_lflag &= ~ECHOK;	/* disable kill character */
      _commSetup.c_lflag &= ~ECHOKE;	/* disable kill character */
      _commSetup.c_lflag &= ~ECHONL;	/* don't echo \n */
      _commSetup.c_lflag &= ~ECHOCTL;	/* don't echo control characters */
      _commSetup.c_lflag &= ~ISIG;	/* don't generate INTR, QUIT and SUSP */
      _commSetup.c_lflag &= ~IEXTEN;	/* disable other special characters */
      _commSetup.c_lflag &= ~NOFLSH;	/* don't flush buffers on signals */
      _commSetup.c_lflag &= ~TOSTOP;
      _commSetup.c_lflag &= ~FLUSHO;
      _commSetup.c_lflag &= ~PENDIN;

      _commSetup.c_cc[VMIN] = 0;	        // Non blocking read
      _commSetup.c_cc[VTIME] = _readDelay;	//  timeout (tenth of second)

      tcflush (_hCom, TCIFLUSH);
      if (tcsetattr (_hCom, TCSANOW, &_commSetup) == -1)
	{
	  Close ();
	  status = -1;
	}
    }
  return status;
}
