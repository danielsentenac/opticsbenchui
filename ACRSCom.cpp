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
  
  QString str = QString(_settings.c_str());
  QStringList  list = str.split(",");
  _ispeed = list.at(0).toInt();
  _numBits = list.at(1).toInt();
  _parity = list.at(2);
  _stopBits = list.at(3).toInt();
  _vtime = list.at(4).toInt();
  _flow = list.at(5);

  _hCom = open (_device.c_str (), O_RDWR | O_NOCTTY );
  if (_hCom == -1)
    {
      QLOG_INFO ( ) << "ACRSCom::Open failed at open" << endl;
      _state = CLOSED;
    }
  else
    {
      if (Setup () != -1)
	{
	  status = 0;
	  _state = OPEN;
	}
      else
	 QLOG_INFO ( ) << "ACRSCom::Open failed at Setup " << endl;
    }
  QLOG_INFO ( ) << "ACRSCom::Open status : " << status << endl;
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
  while ((dataChar != '\n') || (dataChar != '\r'));
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
      tcsetattr (_hCom, TCSANOW, &_commSetup);
      tcflush(_hCom, TCOFLUSH);
      tcflush(_hCom, TCIFLUSH);
    }
  if (close (_hCom))
    {
      status = -1;
    }
  else
    {
      _state = CLOSED;
      _hCom = -1;
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
  speed_t speed = B9600;
  struct termios termios_p;
  //
  // Init structure
  //
  tcgetattr(_hCom, &termios_p);
  memcpy(&_commSetup, &termios_p, sizeof(struct termios));

  switch (_ispeed)
    {
    case 1200:
      termios_p.c_cflag = B1200;
      break;
    case 2400:
      termios_p.c_cflag = B2400;
      break;
    case 4800:
      termios_p.c_cflag = B4800;
      break;
    case 9600:
      termios_p.c_cflag = B9600;
      break;
    case 19200:
      termios_p.c_cflag = B19200;
      break;
    case 38400:
      termios_p.c_cflag = B38400;
      break;
    case 57600:
      termios_p.c_cflag = B57600;
      break;
    case 115200:
      termios_p.c_cflag = B115200;
      break;
    default:
      QLOG_ERROR () << "Bad speed!";
    }
    QLOG_DEBUG ( ) << "RS232 uses " << _ispeed << " baudrate";
    //
    // Number of bits
    //
       switch(_numBits)
    {
	case 5:
	    termios_p.c_cflag |= CS5;
	    break;
	case 6:
	    termios_p.c_cflag |= CS6;
	    break;
	case 7:
	    termios_p.c_cflag |= CS7;
	    break;
	case 8:
	    termios_p.c_cflag |= CS8;
	    break;
        default:
            termios_p.c_cflag |= CS8;
    }
    QLOG_DEBUG ( ) << "RS232 uses " << _numBits << " bits";
    //
    // Parity
    //
    if (!_parity.compare("odd", Qt::CaseInsensitive)) 
        termios_p.c_cflag |= PARODD | PARENB;
    else if (!_parity.compare("even", Qt::CaseInsensitive)) 
        termios_p.c_cflag |= PARENB;
    QLOG_DEBUG ( ) << "RS232 uses " << _parity << " parity";
    //
    // Stop bits
    //
    if ( _stopBits == 2 ) {
	termios_p.c_cflag |= CSTOPB;
        QLOG_DEBUG ( ) << "RS232 uses 2 stop bits";
    }
    else
    QLOG_DEBUG ( ) << "RS232 uses " << _stopBits << " stop bits";
    termios_p.c_cflag |= PARENB;
    termios_p.c_cflag |= CREAD;
    termios_p.c_iflag = IGNPAR | IGNBRK;
    //
    // Flow control
    //
    if (!_flow.compare("software", Qt::CaseInsensitive)) {
        termios_p.c_iflag |= IXON | IXOFF;
        QLOG_DEBUG ( ) << "RS232 uses " << _flow <<  " flow control (Xon/Xoff)";
    }
    else if (!_flow.compare("hardware", Qt::CaseInsensitive)) {
        termios_p.c_cflag |= CRTSCTS;
        QLOG_DEBUG ( ) << "RS232 uses " << _flow << " flow control (RTS/CTS)";
    }
    else  {
        termios_p.c_cflag |= CLOCAL;
        QLOG_DEBUG ( ) << "RS232 uses " << _flow << " flow control";
    }
    termios_p.c_oflag = 0;
    termios_p.c_lflag = 0;
    termios_p.c_cc[VTIME] = _vtime;
    termios_p.c_cc[VMIN] = 0;
    QLOG_DEBUG ( ) << "RS232 uses VMIN=0,VTIME=" << _vtime;
   
    tcflush(_hCom, TCOFLUSH);
    tcflush(_hCom, TCIFLUSH);
    if (tcsetattr (_hCom, TCSANOW, &termios_p) == -1) {
       Close ();
       status = -1;
    }
  return status;
}
