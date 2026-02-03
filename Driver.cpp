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
#include "DriverPI_C862.h"
#include "DriverNewFocus_8750_Ol.h"
#include "DriverNewFocus_8750_Cl.h"
#include "DriverPI_C509.h"
#include "DriverMicos_Pollux.h"
#include "DriverNewPort_NSC200.h"
#ifdef STANDA_uSMC
#include "DriverStanda_uSMC.h"
#elif !defined(NO_STANDA)
#include "DriverStanda_uSMC2.h"
#endif
#include "DriverNewPort_AGUC2.h"
#include "DriverPI_E725.h"
#include "Utils.h"


const string Driver::PI_C862              = "PI_C862";
const string Driver::NEWFOCUS_8750_OL     = "NewFocus_8750_Ol";
const string Driver::NEWFOCUS_8750_CL     = "NewFocus_8750_Cl";
const string Driver::PI_C509              = "PI_C509";
const string Driver::MICOS_POLLUX         = "Micos_Pollux";
const string Driver::NEWPORT_NSC200       = "NewPort_NSC200";
const string Driver::STANDA_USMC          = "Standa_uSMC";
const string Driver::STANDA_USMC2         = "Standa_uSMC2";
const string Driver::PI_E725              = "PI_E725";
const string Driver::NEWPORT_AGUC2        = "NewPort_AGUC2";


using namespace DriverDefinition;

// -----------------------------------------------------------------------------
/// Operation : Create
///    Instantiates a subclass and returns a pointer on it
//
///    Driver type to instantiate and settings.
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
    pactuatorDrv = new DriverPI_C509(channel);
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
    pactuatorDrv = new DriverPI_C862(channel);
  }
  else if (driverType == MICOS_POLLUX)
  {
    pactuatorDrv = new DriverMicos_Pollux(channel);
  }
  else if (driverType == NEWPORT_NSC200)
  {
    pactuatorDrv = new DriverNewPort_NSC200(channel);
  }
#ifdef STANDA_uSMC
  else if (driverType == STANDA_USMC)
  {
    pactuatorDrv = new DriverStanda_uSMC(channel);
  }
#elif !defined(NO_STANDA)
  else if (driverType == STANDA_USMC2)
  {
    pactuatorDrv = new DriverStanda_uSMC2(channel);
  }
#endif
  else if (driverType == NEWPORT_AGUC2)
  {
    pactuatorDrv = new DriverNewPort_AGUC2(channel);
  }
  else if (driverType == PI_E725)
  {
    pactuatorDrv = new DriverPI_E725(channel);
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
///    pointer on the reference driver and channel
//
// -----------------------------------------------------------------------------
Driver* Driver::Create(const Driver* prefActuatorDrv,
                                     ACCom* prefChannel)
{
  QLOG_DEBUG ( ) << "Driver::Create(prefActuatorDrv, prefChannel)";

  Driver *pactuatorDrv = NULL;

  if (const DriverPI_C509* prefPI_C509Drv = 
      dynamic_cast<const DriverPI_C509*> (prefActuatorDrv))
  {
    pactuatorDrv = new DriverPI_C509(*prefPI_C509Drv,prefChannel);
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
  else if (const DriverPI_C862* prefPI_C862Drv = 
	   dynamic_cast<const DriverPI_C862*> (prefActuatorDrv))
  {
    pactuatorDrv = new DriverPI_C862(*prefPI_C862Drv,prefChannel);
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
#ifdef STANDA_uSMC
  else if (const DriverStanda_uSMC* prefStanda_uSMCDrv =
           dynamic_cast<const DriverStanda_uSMC*> (prefActuatorDrv))
    {
      pactuatorDrv = new DriverStanda_uSMC(*prefStanda_uSMCDrv,prefChannel);
    }
#elif !defined(NO_STANDA)
  else if (const DriverStanda_uSMC2* prefStanda_uSMCDrv =
           dynamic_cast<const DriverStanda_uSMC2*> (prefActuatorDrv))
    {
      pactuatorDrv = new DriverStanda_uSMC2(*prefStanda_uSMCDrv,prefChannel);
    }
#endif
  else if (const DriverNewPort_AGUC2* prefNewPort_AGUC2Drv =
           dynamic_cast<const DriverNewPort_AGUC2*> (prefActuatorDrv))
    {
      pactuatorDrv = new DriverNewPort_AGUC2(*prefNewPort_AGUC2Drv,prefChannel);
    }
  else if (const DriverPI_E725* prefPI_E725Drv =
           dynamic_cast<const DriverPI_E725*> (prefActuatorDrv))
    {
      pactuatorDrv = new DriverPI_E725(*prefPI_E725Drv,prefChannel);
    }

  return pactuatorDrv;
}


// -----------------------------------------------------------------------------
/// Operation : GetLastError
///    Gives the code of the last problem
//
///    code of the last error
//
// -----------------------------------------------------------------------------
int Driver::GetLastError(ADErrorCode &lastError)
{
  QLOG_DEBUG ( ) <<"Driver::GetLastError";

  lastError = _lastError;
  return 0;   
}

void Driver::ReportWarning(ADErrorCode code, const QString& message) const
{
  if (code != NO_ERROR) {
    _lastError = code;
  }
  Utils::ReportWarning(NULL, message);
}

void Driver::ReportError(ADErrorCode code, const QString& message) const
{
  if (code != NO_ERROR) {
    _lastError = code;
  }
  Utils::ReportError(NULL, message);
}

void Driver::ReportSettingError(const QString& message) const
{
  ReportWarning(OTHER_ERROR, message);
}

void Driver::ReportCommError(const QString& message) const
{
  ReportError(COMMUNICATION_ERROR, message);
}

void Driver::ReportRangeError(const QString& message) const
{
  ReportWarning(OUT_OF_RANGE_ERROR, message);
}
//------------------------------------------------------------------------------
/// Operation : SendCommand
///    Sends a command to the motor driver interface
///
///    command line text buffer and reply string
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
      ReportCommError("Driver> Unable to write to port");
      return -1;
    }
    usleep(100000);
  }
  ////////////////////////////////////////////////////////////////////////////////////
  // Read reply
  ////////////////////////////////////////////////////////////////////////////////////
  if (!_pcommChannel->Read(answer)) {
    ReportCommError("Driver> Unable to read from port");
    return -2;
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
