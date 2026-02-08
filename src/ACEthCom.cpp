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
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "ACEthCom.h"

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
  int port;
  struct in_addr *addr = NULL;
  struct hostent *host;
  struct in_addr saddr;
  struct sockaddr_in address;
  string connectionAnswer;
  size_t pos;
  string ip_address;
  int on = 1;
  int error;
  sscanf (_settings.c_str (), "%d,%f,%f", &port, &_readDelay, &_writeDelay);

  if (_state == CLOSED)
    {
      _sock = -1;
      pos = _device.find(":");
      ip_address = _device.substr (0,pos); 
      QLOG_DEBUG ( ) << "IP Address found : " << QString(ip_address.c_str());
      saddr.s_addr = inet_addr (ip_address.c_str ());
      if (saddr.s_addr > 0) {
	addr = &saddr;
      }
      if ( ( host = gethostbyname (ip_address.c_str ()) ) ) {
	addr = (struct in_addr *) *host->h_addr_list;
      }
      if (addr) {
	memset ((char *) &address, 0, sizeof (address));
	address.sin_family = AF_INET;
	address.sin_port = htons (port);
	address.sin_addr.s_addr = addr->s_addr;
	_sock = socket (AF_INET, SOCK_STREAM, 0);
	if (_sock > 0) {
	  /* Setup the socket option to reuse */
	  error = setsockopt( _sock, SOL_SOCKET, SO_REUSEADDR, &on,sizeof(on));
	  if ( error == -1 ) {
           ReportWarning(QString("ACEthCom::Open> %1").arg(QString(strerror(errno))));
          }
          QLOG_DEBUG ( ) << "Connecting to " << QString(inet_ntoa (*addr)) << " on port "
			 <<  port;
	  error = connect (_sock, (struct sockaddr *) &address, sizeof (address));
	  if ( error ) {
	    ReportError(QString("ACEthCom::Open> %1").arg(QString(strerror(errno))));
	    ReportError("ACEthCom::Open> Connect failed");
	    close (_sock);
	  }
	  else {
	    // add the socket to the descriptor set
	    FD_SET (_sock, &_master);
	    _state = OPEN;
	  }
	}
	else  ReportWarning(QString("ACEthCom::Open> %1").arg(QString(strerror(errno))));
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
  strcpy(inMessage,message.c_str());
  do 
    {
      write(_sock, &inMessage[i], 1);
      usleep((int)(_writeDelay * 1000000));
      read(_sock, &Reply[i], 1);
    }
  while(inMessage[i++] != '\r');
  
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
  int nwritten;
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
  
  if (select (_sock + 1, NULL, &writeFds, NULL, &timeDelay) > 0)
    {
      if (FD_ISSET (_sock, &writeFds))
	{
	  buf = message.c_str ();
	  int messageLength = (message.length () + 1);	// Null character included
	  while (bytes_sent < messageLength)
	    {
	      nwritten = write (_sock, buf, messageLength - bytes_sent);
	      if (nwritten < 0)
		{
		  ReportWarning(QString("ACEthCom::Write> %1").arg(QString(strerror(errno))));
		  bytes_sent = 0;
		  break;
		}
	      bytes_sent += nwritten;
	      buf += nwritten;
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
  int nwritten;
  const char *buf;
  struct timeval timeDelay;

  fd_set writeFds;		// temp file descriptor list for select()

  writeFds = _master;
  timeDelay.tv_sec = 0;
  timeDelay.tv_usec = (int) (_writeDelay * 1000000);

  if (select (_sock + 1, NULL, &writeFds, NULL, &timeDelay) > 0)
    {
      if (FD_ISSET (_sock, &writeFds))
	{
	  buf = message.c_str ();
	  int messageLength = (message.length ()) * sizeof (char);	// Null character NOT included !!
	  while (bytes_sent < messageLength)
	    {
	      nwritten = write (_sock, buf, messageLength - bytes_sent);
	      if (nwritten < 0)
		{
		  ReportWarning(QString("ACEthCom::WriteRaw> %1").arg(QString(strerror(errno))));
		  bytes_sent = 0;
		  break;
		}
	      bytes_sent += nwritten;
	      buf += nwritten;
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
	 select (_sock + 1, &readFds, NULL, NULL, &timeDelay) > 0)
    {
      if (FD_ISSET (_sock, &readFds))
	{
	  memset (readBuffer, 0, sizeof (readBuffer));
	  bytesRead = read (_sock, readBuffer, STRLENGTH);
          QLOG_DEBUG() << " readBuffer = " << readBuffer
                       << " bytesRead = " << bytesRead;
	  if (bytesRead > 0)
	    {
	      totalBytesRead += bytesRead;
	      message += readBuffer;
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
  bytesRead = read(_sock, readBuffer, STRLENGTH);
  readBuffer[bytesRead] = '\0';
  message = readBuffer;
  QLOG_DEBUG ( ) << "ReadDelay> " << QString(readBuffer);
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
  return close (_sock);
}
