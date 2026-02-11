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

/**
 * @file ACEthCom.cpp
 * @brief AC communication backend for AC Eth Com.
 */


#include <stdio.h>
#include <string.h>
#include <cstring>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <algorithm>
#ifdef Q_OS_WIN
#include <ws2tcpip.h>
#else
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif
#include "PosixCompat.h"

#include "ACEthCom.h"

namespace {
QString SocketErrorString() {
#ifdef Q_OS_WIN
  const int error_code = WSAGetLastError();
  LPSTR msg_buffer = nullptr;
  const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS;
  const DWORD size = FormatMessageA(
      flags, NULL, static_cast<DWORD>(error_code),
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPSTR>(&msg_buffer), 0, NULL);
  QString message;
  if (size && msg_buffer) {
    message = QString::fromLocal8Bit(msg_buffer).trimmed();
    LocalFree(msg_buffer);
  } else {
    message = QString("WSA error %1").arg(error_code);
  }
  return message;
#else
  return QString(strerror(errno));
#endif
}

int SocketWriteAll(
#ifdef Q_OS_WIN
    SOCKET sock,
#else
    int sock,
#endif
    const char* buffer,
    int length) {
  int total_written = 0;
  while (total_written < length) {
#ifdef Q_OS_WIN
    const int written = send(sock, buffer + total_written,
                             length - total_written, 0);
#else
    const int written = write(sock, buffer + total_written,
                              length - total_written);
#endif
    if (written <= 0) {
      return -1;
    }
    total_written += written;
  }
  return total_written;
}

int SocketReadOnce(
#ifdef Q_OS_WIN
    SOCKET sock,
#else
    int sock,
#endif
    char* buffer,
    int length) {
#ifdef Q_OS_WIN
  return recv(sock, buffer, length, 0);
#else
  return read(sock, buffer, length);
#endif
}

bool SocketIsValid(
#ifdef Q_OS_WIN
    SOCKET sock
#else
    int sock
#endif
) {
#ifdef Q_OS_WIN
  return sock != INVALID_SOCKET;
#else
  return sock >= 0;
#endif
}
}  // namespace

const float
ACEthCom::DEFAULT_READ_DELAY = 1;
const float
ACEthCom::DEFAULT_WRITE_DELAY = 1;


// ----------------------------------------------------------------------------
/// Operation : Open
/// Opens the communication channel by creating a socket and connecting to it
// ----------------------------------------------------------------------------
int
ACEthCom::Open ()
{
  int status = -1;
  int port = 0;
  if (sscanf(_settings.c_str(), "%d,%f,%f", &port, &_readDelay, &_writeDelay) < 1 ||
      port <= 0) {
    ReportError(QString("ACEthCom::Open> Invalid ethernet settings: %1")
                .arg(QString(_settings.c_str())));
    return status;
  }

  if (_state == CLOSED)
    {
#ifdef Q_OS_WIN
      if (!_wsaInitialized) {
        WSADATA wsa_data;
        const int wsa_status = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        if (wsa_status != 0) {
          ReportError(QString("ACEthCom::Open> WSAStartup failed with code %1")
                      .arg(wsa_status));
          return status;
        }
        _wsaInitialized = true;
      }
      _sock = INVALID_SOCKET;
#else
      _sock = -1;
#endif

      size_t pos = _device.find(":");
      const string ip_address = _device.substr(0, pos);
      QLOG_DEBUG() << "IP Address found : " << QString(ip_address.c_str());

      struct addrinfo hints;
      std::memset(&hints, 0, sizeof(hints));
      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_protocol = IPPROTO_TCP;

      struct addrinfo* result = NULL;
      const string port_string = std::to_string(port);
      const int gai_status = getaddrinfo(ip_address.c_str(), port_string.c_str(),
                                         &hints, &result);
      if (gai_status != 0 || !result) {
#ifdef Q_OS_WIN
        ReportError(QString("ACEthCom::Open> getaddrinfo failed: %1")
                    .arg(gai_status));
#else
        ReportError(QString("ACEthCom::Open> %1").arg(QString(gai_strerror(gai_status))));
#endif
        return status;
      }

      bool connected = false;
      for (struct addrinfo* rp = result; rp != NULL && !connected; rp = rp->ai_next) {
        _sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (!SocketIsValid(_sock)) {
          continue;
        }
        int on = 1;
#ifdef Q_OS_WIN
        if (setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR,
                       reinterpret_cast<const char*>(&on),
                       static_cast<int>(sizeof(on))) == SOCKET_ERROR) {
          ReportWarning(QString("ACEthCom::Open> %1").arg(SocketErrorString()));
        }
#else
        if (setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, &on,
                       static_cast<socklen_t>(sizeof(on))) == -1) {
          ReportWarning(QString("ACEthCom::Open> %1").arg(SocketErrorString()));
        }
#endif
#ifdef Q_OS_WIN
        if (connect(_sock, rp->ai_addr, static_cast<int>(rp->ai_addrlen)) == 0) {
#else
        if (connect(_sock, rp->ai_addr, static_cast<socklen_t>(rp->ai_addrlen)) == 0) {
#endif
          connected = true;
          break;
        }
#ifdef Q_OS_WIN
        closesocket(_sock);
        _sock = INVALID_SOCKET;
#else
        close(_sock);
        _sock = -1;
#endif
      }

      freeaddrinfo(result);

      if (!connected) {
        ReportError(QString("ACEthCom::Open> %1").arg(SocketErrorString()));
        ReportError("ACEthCom::Open> Connect failed");
      } else {
        FD_SET(_sock, &_master);
        _state = OPEN;
      }
    }
  if (_state == OPEN) {
    status = 0;
  }
  return status;
}

// ----------------------------------------------------------------------------
/// Operation : WriteEcho
///    sends data through the socket
///    message to send
// ----------------------------------------------------------------------------
int
ACEthCom::WriteEcho (string message)
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
      const int written = SocketWriteAll(_sock, &inMessage[i], 1);
      if (written != 1) {
        break;
      }
      usleep((int)(_writeDelay * 1000000));
      const int bytes_read = SocketReadOnce(_sock, &Reply[i], 1);
      if (bytes_read <= 0) {
        break;
      }
    }
  while(i < STRLENGTH - 1 && inMessage[i++] != '\r');
  
  return i;
}

// ----------------------------------------------------------------------------
/// Operation : Write
///    sends data through the socket
///    message to send
// ----------------------------------------------------------------------------
int
ACEthCom::Write (string & message, ...)
{
  int bytes_sent = 0;
  const char *buf;
  struct timeval timeDelay;
  fd_set writeFds;		// temp file descriptor list for select()
  va_list args;
  /*--------------------------------------------------------------------------*/
 
  va_start( args, (&message)[0] );
  char *typewrite = va_arg(args,char*);
  if (typewrite && (sizeof(typewrite) == sizeof(char*))) {
    if (!strcmp(typewrite,"RAW")) {
      QLOG_DEBUG ( ) << "ACEthCom::Write> WRITE type = " << QString(typewrite);
      va_end(args);
      return(WriteRaw(message));
    }
    else if (!strcmp(typewrite,"ECHO")) {
      QLOG_DEBUG ( ) <<  "ACEthCom::Write> WRITE type = " <<   QString(typewrite);
      va_end(args);
      return(WriteEcho(message));
    }
  }
  va_end(args);
  /*--------------------------------------------------------------------------*/
  writeFds = _master;
  timeDelay.tv_sec = 0;
  timeDelay.tv_usec = (int) (_writeDelay * 1000000);
  
  if (select (static_cast<int>(_sock) + 1, NULL, &writeFds, NULL, &timeDelay) > 0)
    {
      if (FD_ISSET (_sock, &writeFds))
	{
	  buf = message.c_str ();
	  int messageLength = (message.length () + 1);	// Null character included
	  while (bytes_sent < messageLength)
	    {
              const int written = SocketWriteAll(_sock, buf, messageLength - bytes_sent);
              if (written < 0)
                {
                  ReportWarning(QString("ACEthCom::Write> %1").arg(SocketErrorString()));
                  bytes_sent = 0;
                  break;
                }
              bytes_sent += written;
              buf += written;
	    }
	}
    }
  return bytes_sent;
}
// ----------------------------------------------------------------------------
/// Operation : WriteRaw
///    sends data through the socket
///    message to send
// ----------------------------------------------------------------------------
int
ACEthCom::WriteRaw (string message)
{
  int bytes_sent = 0;
  const char *buf;
  struct timeval timeDelay;

  fd_set writeFds;		// temp file descriptor list for select()

  writeFds = _master;
  timeDelay.tv_sec = 0;
  timeDelay.tv_usec = (int) (_writeDelay * 1000000);

  if (select (static_cast<int>(_sock) + 1, NULL, &writeFds, NULL, &timeDelay) > 0)
    {
      if (FD_ISSET (_sock, &writeFds))
	{
	  buf = message.c_str ();
	  int messageLength = (message.length ()) * sizeof (char);	// Null character NOT included !!
	  while (bytes_sent < messageLength)
	    {
              const int written = SocketWriteAll(_sock, buf, messageLength - bytes_sent);
              if (written < 0)
                {
                  ReportWarning(QString("ACEthCom::WriteRaw> %1").arg(SocketErrorString()));
                  bytes_sent = 0;
                  break;
                }
              bytes_sent += written;
              buf += written;
	    }
	}
    }
  return bytes_sent;
}
// ----------------------------------------------------------------------------
/// Operation : Read
///    reads data through the socket
///    message to  read
// ----------------------------------------------------------------------------
int
ACEthCom::Read (string & message, ...)
{

  int bytesRead = 1;
  int totalBytesRead = 0;
  char readBuffer[STRLENGTH + 1];
  fd_set readFds;		// temp file descriptor list for select()
  struct timeval timeDelay;
  va_list args;
  
  /*--------------------------------------------------------------------------*/
  va_start( args, (&message)[0] );
  char *typeread = va_arg(args,char*);
  /*--------------------------------------------------------------------------*/
  if (typeread && (sizeof(typeread) == sizeof(char*))) {
    if (!strcmp(typeread,"DELAY")) {
      QLOG_DEBUG ( ) << "ACEthCom::Read> READ type = " <<  QString(typeread);
      int usec = va_arg(args,int);
      va_end(args);
      return(ReadDelay(message,usec));
    }
  }
  va_end(args);
  message.erase ();
  readFds = _master;
  timeDelay.tv_sec = 0;
  timeDelay.tv_usec = (int) (_readDelay * 1000000);

  // timeout longer on the first loop
  while (bytesRead &&
	 select (static_cast<int>(_sock) + 1, &readFds, NULL, NULL, &timeDelay) > 0)
    {
      if (FD_ISSET (_sock, &readFds))
	{
	  memset (readBuffer, 0, sizeof (readBuffer));
	  bytesRead = SocketReadOnce(_sock, readBuffer, STRLENGTH);
          QLOG_DEBUG() << " readBuffer = "
                       << QString::fromLatin1(readBuffer, bytesRead > 0 ? bytesRead : 0)
                       << " bytesRead = " << bytesRead;
	  if (bytesRead > 0)
	    {
	      totalBytesRead += bytesRead;
	      message.append(readBuffer, bytesRead);
	    }
       }
      readFds = _master;
    }

  return totalBytesRead;
 
}

// ----------------------------------------------------------------------------
/// Operation : ReadDelay
///    reads data through the socket without timeout applied. 
///    The data is limited to the buffer size STRLENGTH.
///    It should be used when one expect a small message with size < STRLENGTH.
///    message to  read
// ----------------------------------------------------------------------------
int
ACEthCom::ReadDelay (string & message, int usec)
{
  int bytesRead;
  char readBuffer[STRLENGTH];
  memset (readBuffer, 0, sizeof (readBuffer)); 
  message.erase ();
  usleep(usec);
  bytesRead = SocketReadOnce(_sock, readBuffer, STRLENGTH - 1);
  if (bytesRead > 0) {
    readBuffer[bytesRead] = '\0';
    message = readBuffer;
    QLOG_DEBUG ( ) << "ReadDelay> " << QString(readBuffer);
  } else {
    message.clear();
  }
  return bytesRead;
}

// ----------------------------------------------------------------------------
/// Operation : Close
///    Closes the communcation
// ----------------------------------------------------------------------------
int
ACEthCom::Close ()
{
  _state = CLOSED;
  FD_ZERO (&_master);
  int status = 0;
  if (SocketIsValid(_sock)) {
#ifdef Q_OS_WIN
    status = closesocket(_sock);
    _sock = INVALID_SOCKET;
#else
    status = close(_sock);
    _sock = -1;
#endif
  }
#ifdef Q_OS_WIN
  if (_wsaInitialized) {
    WSACleanup();
    _wsaInitialized = false;
  }
#endif
  return status;
}
