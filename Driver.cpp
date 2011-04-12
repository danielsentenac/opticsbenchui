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

#include "Driver.h"
#include "DriverPi_C862.h"
#include "DriverNewFocus_8750_Ol.h"
#include "DriverNewFocus_8750_Cl.h"
#include "DriverPi_C509.h"
#include "DriverMicos_Pollux.h"
#include "DriverNewPort_NSC200.h"

const string Driver::PI_C862              = "Pi_C862";
const string Driver::NEWFOCUS_8750_OL     = "NewFocus_8750_Ol";
const string Driver::NEWFOCUS_8750_CL     = "NewFocus_8750_Cl";
const string Driver::PI_C509              = "Pi_C509";
const string Driver::MICOS_POLLUX         = "Micos_Pollux";
const string Driver::NEWPORT_NSC200       = "NewPort_NSC200";

using namespace DriverDefinition;

// -----------------------------------------------------------------------------
/// Operation : Create
///    Instantiates a subclass and returns a pointer on it
//
/// @param driverType
///    Driver type to instantiate
/// @param setting
///    Driver setting
/// @channel
///    Channel communication
//
// -----------------------------------------------------------------------------
Driver* Driver::Create(string driverType,
		       string setting,
		       ACCom* channel)
{
  QLOG_DEBUG ( ) << "Driver::Create(driverType, setting, channel)";

  Driver *pactuatorDrv = NULL;

  if (driverType == PI_C509)
  {
    pactuatorDrv = new DriverPi_C509(channel);
  }
  else if (driverType == NEWFOCUS_8750_OL)
  {
    pactuatorDrv = new DriverNewFocus_8750_Ol(channel);
  }
  else if (driverType == NEWFOCUS_8750_CL)
  {
    pactuatorDrv = new DriverNewFocus_8750_Cl(channel);
  }
  else if (driverType == PI_C862)
  {
    pactuatorDrv = new DriverPi_C862(channel);
  }
  else if (driverType == MICOS_POLLUX)
  {
    pactuatorDrv = new DriverMicos_Pollux(channel);
  }
  else if (driverType == NEWPORT_NSC200)
  {
    pactuatorDrv = new DriverNewPort_NSC200(channel);
  }
  if (pactuatorDrv)
  { 
    pactuatorDrv->_setting    = setting;
    pactuatorDrv->_lastError  = NO_ERROR;
  }
  return pactuatorDrv;
}

// -----------------------------------------------------------------------------
/// Operation : Create
///    Instantiates a subclass and returns a pointer on it
//
/// @param prefActuatorDrv
///    pointer on the reference driver
/// @param prefChannel
///    pointer on the reference channel
//
// -----------------------------------------------------------------------------
Driver* Driver::Create(const Driver* prefActuatorDrv,
                                     ACCom* prefChannel)
{
  QLOG_DEBUG ( ) << "Driver::Create(prefActuatorDrv, prefChannel)";

  Driver *pactuatorDrv = NULL;

  if (const DriverPi_C509* prefPi_C509Drv = 
      dynamic_cast<const DriverPi_C509*> (prefActuatorDrv))
  {
    pactuatorDrv = new DriverPi_C509(*prefPi_C509Drv,prefChannel);
  }
  else if (const DriverNewFocus_8750_Ol* prefNewFocus_8750_OlDrv = 
	   dynamic_cast<const DriverNewFocus_8750_Ol*> (prefActuatorDrv))
  {  
    pactuatorDrv = new DriverNewFocus_8750_Ol(*prefNewFocus_8750_OlDrv, prefChannel);
  }
  else if (const DriverNewFocus_8750_Cl* prefNewFocus_8750_Cl =
	   dynamic_cast<const DriverNewFocus_8750_Cl*> (prefActuatorDrv))
  {
    pactuatorDrv = new DriverNewFocus_8750_Cl(*prefNewFocus_8750_Cl,prefChannel);
  }
  else if (const DriverPi_C862* prefPi_C862Drv = 
	   dynamic_cast<const DriverPi_C862*> (prefActuatorDrv))
  {
    pactuatorDrv = new DriverPi_C862(*prefPi_C862Drv,prefChannel);
  }
  else if (const DriverMicos_Pollux* prefMicos_PolluxDrv = 
	   dynamic_cast<const DriverMicos_Pollux*> (prefActuatorDrv))
  {
    pactuatorDrv = new DriverMicos_Pollux(*prefMicos_PolluxDrv,prefChannel);
  }
  else if (const DriverNewPort_NSC200* prefNewPort_NSC200Drv = 
	   dynamic_cast<const DriverNewPort_NSC200*> (prefActuatorDrv))
    {
      pactuatorDrv = new DriverNewPort_NSC200(*prefNewPort_NSC200Drv,prefChannel);
    }
  return pactuatorDrv;
}


// -----------------------------------------------------------------------------
/// Operation : GetLastError
///    Gives the code of the last problem
//
/// @param lastError
///    code of the last error
//
// -----------------------------------------------------------------------------
int Driver::GetLastError(ADErrorCode &lastError)
{
  QLOG_DEBUG ( ) <<"Driver::GetLastError";

  lastError = _lastError;
  return 0;   
}
//------------------------------------------------------------------------------
/// Operation : SendCommand
///    Sends a command to the motor driver interface
///
/// @param state
///    command line text
/// @param actuatorName
/// actuator name
//------------------------------------------------------------------------------
int 
Driver::SendGeneralCommand(char *buffer,string& rply) const
{  
  int retStatus = 1;
  string answer = "";
  string command = "";
  char mybuffer[32];
  vector<string>  mBufferVect;
  string strbuffer =  buffer;

  if (!mBufferVect.empty()) mBufferVect.erase(mBufferVect.begin(),mBufferVect.end());  
  Tokenize(strbuffer,mBufferVect,",");
  for (unsigned int i = 0 ; i < mBufferVect.size(); i++ ) {
    sprintf(mybuffer,"%s\r",mBufferVect[i].c_str());
    command = mybuffer;
    QLOG_DEBUG ( ) << "Driver> Send Command " << QString(command.c_str());
    //////////////////////////////////////////////////////////////////////////
    // Apply Command
    //////////////////////////////////////////////////////////////////////////
    if (_pcommChannel->Write(command) < (int)command.length()) { 
      QLOG_DEBUG ( ) <<"Driver> Unable to write to port";
      return (-1);
    }
    usleep(100000);
  }
  ////////////////////////////////////////////////////////////////////////////////////
  // Read reply
  ////////////////////////////////////////////////////////////////////////////////////
  if (!_pcommChannel->Read(answer)) {
    QLOG_DEBUG ( ) << "Driver> Unable to read from port";
    return(-2);
  }
  QLOG_DEBUG ( ) << "Driver> Reply to Command is :" << QString(answer.c_str());
  rply = answer;
  QLOG_DEBUG ( ) << "Driver> returning retStatus " << retStatus;
  return retStatus;
}

void  
Driver::Tokenize(const string& str,
			vector<string>& tokens,
			const string& delimiters) const
{
  // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);
    
    while (string::npos != pos || string::npos != lastPos)
      {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
      }
}

