/// @file ACRSCom.cpp
/// Class ACRSCom 
/// This class provides a set of operations to perform 
/// a serial port communication

// who      when     what
// ----------------------------------------------------------------------------
// C.Moins 20/07/05 created
// D Sentenac 12/06/07 modify serial port settings

#include <sstream>
#include <cstring>
#include <cstdio>
#include <algorithm>
#ifdef Q_OS_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#endif
#include "ACRSCom.h"
#include "PosixCompat.h"

namespace {
#ifdef Q_OS_WIN
QString WindowsErrorString(DWORD error_code) {
  LPSTR msg_buffer = nullptr;
  const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS;
  const DWORD size = FormatMessageA(
      flags, NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPSTR>(&msg_buffer), 0, NULL);
  QString message;
  if (size && msg_buffer) {
    message = QString::fromLocal8Bit(msg_buffer).trimmed();
    LocalFree(msg_buffer);
  } else {
    message = QString("Error code %1").arg(static_cast<unsigned long>(error_code));
  }
  return message;
}
#endif
}  // namespace

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
  _ispeed = 0;
  _readDelay = DEFAULT_READ_DELAY;
  std::memset(_flow, 0, sizeof(_flow));
  const int parsed = sscanf(_settings.c_str(), "%d,%d,%31s",
                            &_ispeed, &_readDelay, _flow);
  if (parsed <= 0 || _ispeed <= 0) {
    ReportError(QString("ACRSCom::Open> Invalid serial settings: %1")
                .arg(QString(_settings.c_str())));
    _state = CLOSED;
    return status;
  }
  if (_readDelay <= 0) {
    _readDelay = DEFAULT_READ_DELAY;
  }
  if (parsed < 3) {
    std::strncpy(_flow, "NONE", sizeof(_flow) - 1);
    _flow[sizeof(_flow) - 1] = '\0';
  }

#ifdef Q_OS_WIN
  QString serial_port = QString(_device.c_str()).trimmed();
  if (serial_port.startsWith("COM", Qt::CaseInsensitive) &&
      !serial_port.startsWith("\\\\.\\")) {
    serial_port.prepend("\\\\.\\");
  }
  const QByteArray port_name = serial_port.toLocal8Bit();
  _hCom = CreateFileA(port_name.constData(),
                      GENERIC_READ | GENERIC_WRITE,
                      0,
                      NULL,
                      OPEN_EXISTING,
                      0,
                      NULL);
  if (_hCom == INVALID_HANDLE_VALUE)
    {
      ReportError(QString("ACRSCom::Open> Unable to open %1: %2")
                  .arg(serial_port)
                  .arg(WindowsErrorString(GetLastError())));
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
        {
          Close();
        }
    }
#else
  _hCom = open (_device.c_str (), O_RDWR);
  if (_hCom == -1)
    {
      ReportError(QString("ACRSCom::Open> Unable to open %1: %2")
                  .arg(QString(_device.c_str()))
                  .arg(QString(strerror(errno))));
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
#endif
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
  std::memset(Reply, 0, sizeof(Reply));
  std::memset(inMessage, 0, sizeof(inMessage));
  const size_t max_copy = std::min(message.size(), static_cast<size_t>(STRLENGTH - 1));
  std::memcpy(inMessage, message.c_str(), max_copy);
  do 
    {
#ifdef Q_OS_WIN
      DWORD written = 0;
      DWORD bytes_read = 0;
      if (!WriteFile(_hCom, &inMessage[i], 1, &written, NULL) || written != 1) {
        break;
      }
      if (!ReadFile(_hCom, &Reply[i], 1, &bytes_read, NULL) || bytes_read == 0) {
        break;
      }
#else
      write(_hCom, &inMessage[i], 1);
      read(_hCom, &Reply[i], 1);
#endif
    }
  while(i < STRLENGTH - 1 && inMessage[i++] != '\r');
  return i;
}
// ----------------------------------------------------------------------------
/// Operation : Write
/// Writes datas on the serial port. Returns the number of bytes written
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
      QLOG_DEBUG () << "ACRSCom::Write> WRITE type = " << typewrite;
      va_end(args);
      return (WriteEcho(message));
    }
  }
  va_end(args);
#ifdef Q_OS_WIN
  DWORD bytes_written = 0;
  if (!WriteFile(_hCom, message.c_str(),
                 static_cast<DWORD>(message.length()),
                 &bytes_written, NULL))
    {
      ReportWarning(QString("ACRSCom::Write> %1")
                    .arg(WindowsErrorString(GetLastError())));
      return -1;
    }
  return static_cast<int>(bytes_written);
#else
  return (write (_hCom, message.c_str (), (message.length ())));
#endif
}

// ----------------------------------------------------------------------------
/// Operation : Read
/// Reads datas on the serial port. Returns the number of read characters.
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
#ifdef Q_OS_WIN
      DWORD bytes_read = 0;
      if (ReadFile(_hCom, &dataChar, 1, &bytes_read, NULL) && bytes_read > 0)
        {
          message += dataChar;
          nbCharRead++;
        }
      else
        {
          break;
        }
#else
      if (read (_hCom, &dataChar, 1) > 0)
	{
	  message += dataChar;
	  nbCharRead++;
	}
      else
	{
	  break;
	}
#endif
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
#ifdef Q_OS_WIN
  if (_hCom != INVALID_HANDLE_VALUE)
    {
      if (!CloseHandle(_hCom))
        {
          if (_state == OPEN) {
            ReportWarning(QString("ACRSCom::Close> Unable to close device: %1")
                          .arg(WindowsErrorString(GetLastError())));
            status = -1;
          }
        }
      _hCom = INVALID_HANDLE_VALUE;
    }
  _state = CLOSED;
  return status;
#else
  if (_state == OPEN)
    {
      tcsetattr (_hCom, TCSANOW, &_commOldSetup);
    }
  if (close (_hCom))
    {
      if (_state == OPEN) {
        ReportWarning("ACRSCom::Close> Unable to close device");
        status = -1;
      }
    }
  else
    {
      _state = CLOSED;
    }
  return status;
#endif
}

// ----------------------------------------------------------------------------
/// Operation : Setup
/// Setup the serial communication 
// ----------------------------------------------------------------------------
int
ACRSCom::Setup ()
{
#ifdef Q_OS_WIN
  if (_hCom == INVALID_HANDLE_VALUE) {
    ReportError("ACRSCom::Setup> Invalid serial handle");
    return -1;
  }
  DCB dcb;
  std::memset(&dcb, 0, sizeof(dcb));
  dcb.DCBlength = sizeof(dcb);
  if (!GetCommState(_hCom, &dcb))
    {
      ReportError(QString("ACRSCom::Setup> Unable to read current serial settings: %1")
                  .arg(WindowsErrorString(GetLastError())));
      return -1;
    }

  dcb.BaudRate = static_cast<DWORD>(_ispeed);
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;
  dcb.fBinary = TRUE;
  dcb.fParity = FALSE;
  dcb.fOutxCtsFlow = FALSE;
  dcb.fOutxDsrFlow = FALSE;
  dcb.fDtrControl = DTR_CONTROL_ENABLE;
  dcb.fDsrSensitivity = FALSE;
  dcb.fTXContinueOnXoff = TRUE;
  dcb.fOutX = FALSE;
  dcb.fInX = FALSE;
  dcb.fRtsControl = RTS_CONTROL_ENABLE;
  if (!strcmp(_flow, "XONXOFF")) {
    QLOG_INFO() << "RS232 uses XONXOFF flow control";
    dcb.fOutX = TRUE;
    dcb.fInX = TRUE;
  } else {
    QLOG_INFO() << "RS232 uses default (NONE) flow control";
  }

  if (!SetCommState(_hCom, &dcb))
    {
      ReportError(QString("ACRSCom::Setup> Unable to apply serial settings: %1")
                  .arg(WindowsErrorString(GetLastError())));
      return -1;
    }

  COMMTIMEOUTS timeouts;
  std::memset(&timeouts, 0, sizeof(timeouts));
  timeouts.ReadIntervalTimeout = MAXDWORD;
  timeouts.ReadTotalTimeoutMultiplier = 0;
  timeouts.ReadTotalTimeoutConstant =
      static_cast<DWORD>(std::max(1, _readDelay) * 100);
  timeouts.WriteTotalTimeoutMultiplier = 0;
  timeouts.WriteTotalTimeoutConstant = 500;
  if (!SetCommTimeouts(_hCom, &timeouts))
    {
      ReportError(QString("ACRSCom::Setup> Unable to set serial timeouts: %1")
                  .arg(WindowsErrorString(GetLastError())));
      return -1;
    }
  PurgeComm(_hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);
  return 0;
#else
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
      ReportError("ACRSCom::Setup> Bad speed");
      return -1;
    }
  tcgetattr (_hCom, &_commOldSetup);	/* save current port settings */
  _commSetup = _commOldSetup;

  if (cfsetispeed (&_commSetup, speed) == -1 ||
      cfsetospeed (&_commSetup, speed) == -1)
    {
      ReportError("ACRSCom::Setup> Unable to set baud rate");
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
	QLOG_INFO() << "RS232 uses XONXOFF flow control";
	_commSetup.c_iflag |= ~IXOFF;	/* enable */
	_commSetup.c_iflag |= ~IXON;	/* enable */
      }
      else {
	QLOG_INFO() << "RS232 uses defaut (NONE) flow control";
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
      QLOG_INFO() << "RS232 uses 8 data bits";
      _commSetup.c_cflag |= CS8;	/* use 8 bits per byte */
      QLOG_INFO() << "RS232 uses 1 stop bit";
      _commSetup.c_cflag &= ~CSTOPB;	/* use only one stop bit */
      QLOG_INFO() << "RS232 uses no parity";
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
	  ReportError("ACRSCom::Setup> Unable to apply serial settings");
	  Close ();
	  status = -1;
	}
    }
  return status;
#endif
}
