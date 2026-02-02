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

enum { OPEN, CLOSED };
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

class DriverSuperK 
{
  public:
 
    // Constructor
    DriverSuperK(ACCom* commChannel);

    // Destructor
    virtual ~DriverSuperK();

    //
    // Methods:
    //
    virtual int Init();
    virtual int setPower(int power);
    virtual int getPower(int& power);
    virtual int setNd(int nd);
    virtual int getNd(int& nd);
    virtual int setSwp(int swp);
    virtual int getSwp(int& swp);
    virtual int setLwp(int lwp);
    virtual int getLwp(int& lwp);

    virtual int operationComplete();

  private :

    static const unsigned char masterId;
    static const int timeout_mS;
    static const int NB_ITEM_DRV_SETTING;

    unsigned char* readInterbus_Byte(unsigned char deviceId, unsigned char registerId);
    bool writeInterbus_Byte(unsigned char deviceId, unsigned char registerId, unsigned char data);
    unsigned short readInterbus_UInt16(unsigned char deviceId, unsigned char registerId);
    bool writeInterbus_UInt16(unsigned char deviceId, unsigned char registerId, unsigned short data);
    unsigned int readInterbus_UInt32(unsigned char deviceId, unsigned char registerId);
    bool writeInterbus_UInt32(unsigned char deviceId, unsigned char registerId, unsigned int data);
    float readInterbus_Float32(unsigned char deviceId, unsigned char registerId);
    bool writeInterbus_Float32(unsigned char deviceId, unsigned char registerId, float data);
    unsigned char* readInterbus_Stream(unsigned char deviceId, unsigned char registerId);
    bool writeInterbus_Stream(unsigned char deviceId, unsigned char registerId, unsigned char *data);
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
