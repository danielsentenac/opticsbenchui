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

#ifndef DRIVERSUPERK_H
#define DRIVERSUPERK_H

#include "ACCom.h"
enum {
    Hunting_SOT,
    Hunting_EOT,
    MessageReady,
    Timeout_Error,
    CRC_Error,
    Garbage_Error,
    Overrun_Error,
    Content_Error,
    Port_Lost
    };

// Set namespace
using namespace std;

/// \ingroup lasers
/// Low-level driver for the SuperK laser over Interbus.
class DriverSuperK 
{
  public:
 
    /// Constructor.
    /// \param commChannel Communication channel instance.
    DriverSuperK(ACCom* commChannel);

    /// Destructor.
    virtual ~DriverSuperK();

    //
    // Methods:
    //
    /// Initialize the SuperK device.
    virtual int Init();
    /// Set the power setting.
    /// \param power Power value.
    virtual int setPower(int power);
    /// Read the current power setting.
    /// \param power Output power value.
    virtual int getPower(int& power);
    /// Set the neutral density (ND) value.
    /// \param nd ND value.
    virtual int setNd(int nd);
    /// Read the current neutral density (ND) value.
    /// \param nd Output ND value.
    virtual int getNd(int& nd);
    /// Set the short-wavelength pass (SWP) value.
    /// \param swp SWP value.
    virtual int setSwp(int swp);
    /// Read the current short-wavelength pass (SWP) value.
    /// \param swp Output SWP value.
    virtual int getSwp(int& swp);
    /// Set the long-wavelength pass (LWP) value.
    /// \param lwp LWP value.
    virtual int setLwp(int lwp);
    /// Read the current long-wavelength pass (LWP) value.
    /// \param lwp Output LWP value.
    virtual int getLwp(int& lwp);

    /// Check whether the last command has completed.
    virtual int operationComplete();

  private :

    static const unsigned char masterId;
    static const int timeout_mS;
    static const int NB_ITEM_DRV_SETTING;

    bool readInterbus_Byte(unsigned char deviceId, unsigned char registerId, unsigned char& value);
    bool writeInterbus_Byte(unsigned char deviceId, unsigned char registerId, unsigned char data);
    unsigned short readInterbus_UInt16(unsigned char deviceId, unsigned char registerId);
    bool writeInterbus_UInt16(unsigned char deviceId, unsigned char registerId, unsigned short data);
    unsigned int readInterbus_UInt32(unsigned char deviceId, unsigned char registerId);
    bool writeInterbus_UInt32(unsigned char deviceId, unsigned char registerId, unsigned int data);
    float readInterbus_Float32(unsigned char deviceId, unsigned char registerId);
    bool writeInterbus_Float32(unsigned char deviceId, unsigned char registerId, float data);
    bool readInterbus_Stream(unsigned char deviceId, unsigned char registerId,
                             unsigned char *data, int dataSize);
    bool writeInterbus_Stream(unsigned char deviceId, unsigned char registerId,
                              unsigned char *data);
    unsigned short calcCRC16(unsigned char data, unsigned short OldCRC);
    void addToTxMsgData(unsigned char *data, int cnt, bool escParse, bool updCRC);
    void addToTxMsgData(unsigned char data, bool escParse, bool updCRC);
    void sendInterbusMessage(unsigned char deviceId, unsigned char registerId, unsigned char msgType, unsigned char *data);
    int ReceiveMessage(unsigned char deviceId, unsigned char registerId, unsigned char msgType, unsigned char *payload);
    QString getStringFromUnsignedChar( unsigned char *str );
    int extractBit(unsigned short data, int pos);
    int rxTimeout;
    bool rxEscape;
    int rxBufferSize;
    unsigned char rxBuffer[255];
    unsigned short rxCRC;
    int txBufferSize;
    unsigned char txBuffer[255];
    unsigned short txCRC;
    int rxState;   
    ACCom *comm;

};
#endif //_DRIVERSuperK_H_
