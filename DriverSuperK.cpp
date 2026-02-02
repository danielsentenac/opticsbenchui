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

#include "DriverSuperK.h"

#include <cstring>

const int DriverSuperK::NB_ITEM_DRV_SETTING  =  1;  // number of items at the driver ()
const unsigned char DriverSuperK::masterId = 66; // Device master Id
const int DriverSuperK::timeout_mS = 50;

DriverSuperK::DriverSuperK(ACCom* _commChannel)
{
  comm = _commChannel;
  QLOG_DEBUG() << "DriverSuperK::DriverSuperK";

}
DriverSuperK::~DriverSuperK()
{
  QLOG_DEBUG() << "DriverSuperK::~DriverSuperK";

}
int 
DriverSuperK::Init()
{
  QLOG_DEBUG() << "DriverSuperK::Init";
  int retStatus = 0;
  unsigned char version[255] = {};
  if (readInterbus_Stream(0x0F, 0x64, version, static_cast<int>(sizeof(version)))) {
    QLOG_INFO() << " Version : " << QString(reinterpret_cast<char*>(version));
  } else {
    retStatus = -1;
  }
  unsigned char emission = 0;
  if (readInterbus_Byte(0x0F, 0x30, emission)) {
    QLOG_INFO() << " Read On/Off emission " << QString("%1").arg(emission,0,16);
  } else {
    retStatus = -1;
  }
  return retStatus;
}
int 
DriverSuperK::getPower(int& power) 
{
  QLOG_DEBUG() << "DriverSuperK::getPower";
  int retStatus = 0;
  unsigned short _power;
  _power = readInterbus_UInt16(0x0F, 0x37);
  power = _power;
  QLOG_INFO() << "Power Level= " << QString::number(power);
  return retStatus;
}
int 
DriverSuperK::setPower(int power)
{
  QLOG_DEBUG() << "DriverSuperK::setPower";
  int retStatus = 0;
  unsigned short _power;
  _power = power;
  retStatus = writeInterbus_UInt16(0x0F, 0x37,_power);
  return retStatus;
}
int
DriverSuperK::getNd(int& nd)
{
  QLOG_DEBUG() << "DriverSuperK::getNd";
  int retStatus = 0;
  unsigned short _nd;
  _nd = readInterbus_UInt16(0x10, 0x32);
  nd = _nd;
  QLOG_INFO() << "Nd= " << QString::number(nd);
  return retStatus;
}
int
DriverSuperK::setNd(int nd)
{
  QLOG_DEBUG() << "DriverSuperK::setNd";
  int retStatus = 0;
  unsigned short _nd;
  _nd = nd;
  retStatus = writeInterbus_UInt16(0x10, 0x32,_nd);
  return retStatus;
}
int
DriverSuperK::getSwp(int& swp)
{
  QLOG_DEBUG() << "DriverSuperK::getSwp";
  int retStatus = 0;
  unsigned short _swp;
  _swp = readInterbus_UInt16(0x10, 0x33);
  swp = _swp;
  QLOG_INFO() << "Swp= " << QString::number(swp);
  return retStatus;
}
int
DriverSuperK::setSwp(int swp)
{
  QLOG_DEBUG() << "DriverSuperK::setSwp";
  int retStatus = 0;
  unsigned short _swp;
  _swp = swp;
  retStatus = writeInterbus_UInt16(0x10, 0x33,_swp);
  return retStatus;
}
int
DriverSuperK::getLwp(int& lwp)
{
  QLOG_DEBUG() << "DriverSuperK::getLwp";
  int retStatus = 0;
  unsigned short _lwp;
  _lwp = readInterbus_UInt16(0x10, 0x34);
  lwp = _lwp;
  QLOG_INFO() << "Lwp= " << QString::number(lwp);
  return retStatus;
}
int
DriverSuperK::setLwp(int lwp)
{
  QLOG_DEBUG() << "DriverSuperK::setLwp";
  int retStatus = 0;
  unsigned short _lwp;
  _lwp = lwp;
  retStatus = writeInterbus_UInt16(0x10, 0x34,_lwp);
  return retStatus;
}
int 
DriverSuperK::operationComplete()
{
  QLOG_DEBUG() << "DriverSuperK::operationComplete";
  int retStatus = 0;
  unsigned short status;
  status = readInterbus_UInt16(0x10, 0x66);
  for (int i = 0 ; i < 16 ; i++)
    QLOG_DEBUG() << "Status Bit " << i << " =  " <<  extractBit(status,i);
  if ( extractBit(status,12) == 0 && 
       extractBit(status,13) == 0 &&
       extractBit(status,14) == 0 )
     retStatus = 1;

  return retStatus;
}
bool
DriverSuperK::readInterbus_Byte(unsigned char deviceId, unsigned char registerId, unsigned char& value)
{
  unsigned char tempData[1] = {0};
  if (comm->GetStatus() == ACCom::OPEN) {
    sendInterbusMessage(deviceId, registerId, 0x04, NULL);
    if (ReceiveMessage(deviceId, registerId, 0x08, tempData) == MessageReady) {
      value = tempData[0];
      return true;
    }
  }
  return false;
}
bool 
DriverSuperK::writeInterbus_Byte(unsigned char deviceId, unsigned char registerId, unsigned char data)
{
  unsigned char ackbyte[1] = {0};

  if (comm->GetStatus() == ACCom::OPEN) {
    sendInterbusMessage(deviceId, registerId, 0x05, &data);
    if (ReceiveMessage(deviceId, registerId, 0x03, ackbyte) == MessageReady) {
      return true;  // Acknowledge received (0x03)
    }
  }
    return false;
}
unsigned short 
DriverSuperK::readInterbus_UInt16(unsigned char deviceId, unsigned char registerId)
{
  typedef union
  {
    unsigned short ushortdata;
    unsigned char bytedata[2];
  }  ByteUInt16Conv;

  ByteUInt16Conv tempData;

  if (comm->GetStatus() == ACCom::OPEN) { 
    sendInterbusMessage(deviceId, registerId, 0x04, NULL);
    if (ReceiveMessage(deviceId, registerId, 0x08, tempData.bytedata) == MessageReady){
       return tempData.ushortdata;
    }
  }
  return 0;
}
bool 
DriverSuperK::writeInterbus_UInt16(unsigned char deviceId, unsigned char registerId, unsigned short data)
{
  typedef union
  {
    unsigned short ushortdata;
    unsigned char bytedata[2];
  }  ByteUInt16Conv;

  ByteUInt16Conv tempData;
  unsigned char ackbyte[1];

  tempData.ushortdata = data;

  if (comm->GetStatus() == ACCom::OPEN) {
    sendInterbusMessage(deviceId, registerId, 0x05, tempData.bytedata);
    if (ReceiveMessage(deviceId, registerId, 0x03, ackbyte) == MessageReady)
      return true;  // Acknowledge received (0x03)
  } 
  return false;
}
unsigned int 
DriverSuperK::readInterbus_UInt32(unsigned char deviceId, unsigned char registerId)
{
  typedef union
  {
    unsigned int uintdata;
    unsigned char bytedata[4];
  }  ByteUInt32Conv;

  ByteUInt32Conv tempData;

  if (comm->GetStatus() == ACCom::OPEN) {
    sendInterbusMessage(deviceId, registerId, 0x04, NULL);
    if (ReceiveMessage(deviceId, registerId, 0x08, tempData.bytedata) == MessageReady){
       return tempData.uintdata;
    }
  }
  return 0;
}
bool 
DriverSuperK::writeInterbus_UInt32(unsigned char deviceId, unsigned char registerId, unsigned int data)
{
  typedef union
  {
    unsigned int uintdata;
    unsigned char bytedata[4];
  }  ByteUInt32Conv;

  ByteUInt32Conv tempData;
  unsigned char ackbyte[1];

  tempData.uintdata = data;

  if (comm->GetStatus() == ACCom::OPEN) {
    sendInterbusMessage(deviceId, registerId, 0x05, tempData.bytedata);
    if (ReceiveMessage( deviceId, registerId, 0x03, ackbyte) == MessageReady)
      return true;  // Acknowledge received (0x03)
    }
  return false;
}
float 
DriverSuperK::readInterbus_Float32(unsigned char deviceId, unsigned char registerId)
{
  typedef union
  {
    float floatdata;
    unsigned char bytedata[4];
  }  ByteFloatConv;

  ByteFloatConv tempData;

  if (comm->GetStatus() == ACCom::OPEN) {
    sendInterbusMessage(deviceId, registerId, 0x04, NULL);
    if (ReceiveMessage(deviceId, registerId, 0x08, tempData.bytedata) == MessageReady)
       return tempData.floatdata;
  }
  return 0;
}
bool 
DriverSuperK::writeInterbus_Float32(unsigned char deviceId, unsigned char registerId, float data)
{
  typedef union
  {
    float floatdata;
    unsigned char bytedata[4];
  }  ByteFloatConv;

  ByteFloatConv tempData;
  unsigned char ackbyte[1];

  tempData.floatdata = data;

  if (comm->GetStatus() == ACCom::OPEN) {
    sendInterbusMessage(deviceId, registerId, 0x05, tempData.bytedata);
    if (ReceiveMessage( deviceId, registerId, 0x03, ackbyte) == MessageReady)
      return true;  // Acknowledge received (0x03)
  }
  return false;
}
bool
DriverSuperK::readInterbus_Stream(unsigned char deviceId, unsigned char registerId,
                                  unsigned char *data, int dataSize)
{
   if (!data || dataSize <= 0) {
     return false;
   }
   unsigned char tempData[255] = {};
   if (comm->GetStatus() == ACCom::OPEN) {  
     sendInterbusMessage(deviceId, registerId, 0x04, NULL);
     if (ReceiveMessage(deviceId, registerId, 0x08, tempData) == MessageReady) {
       int copySize = dataSize;
       if (copySize > static_cast<int>(sizeof(tempData))) {
         copySize = static_cast<int>(sizeof(tempData));
       }
       memcpy(data, tempData, copySize);
       if (copySize < dataSize) {
         memset(data + copySize, 0, dataSize - copySize);
       }
       return true;
     }
   }
    return false;
}
bool 
DriverSuperK::writeInterbus_Stream(unsigned char deviceId, unsigned char registerId, unsigned char data[255])
{
   unsigned char ackbyte[1];
   if (comm->GetStatus() == ACCom::OPEN) {
     sendInterbusMessage(deviceId, registerId, 0x05, data);
     if (ReceiveMessage(deviceId, registerId, 0x03, ackbyte) == MessageReady)
	return true;  // Acknowledge received (0x03)
   }
   return false;
}
unsigned short 
DriverSuperK::calcCRC16(unsigned char data, unsigned short OldCRC)
{
   OldCRC = (unsigned short)((OldCRC >> 8) | (OldCRC << 8));
   OldCRC ^= data;
   OldCRC ^= (unsigned char)((OldCRC & 0xff) >> 4);
   OldCRC ^= (unsigned short)((OldCRC << 8) << 4);
   OldCRC ^= (unsigned short)(((OldCRC & 0xff) << 4) << 1);
   return OldCRC;
}
void 
DriverSuperK::addToTxMsgData(unsigned char *data, int cnt, bool escParse, bool updCRC)
{
   for (int i = 0; i < cnt; i++)
     addToTxMsgData(data[i], escParse, updCRC);
}
void 
DriverSuperK::addToTxMsgData(unsigned char data, bool escParse, bool updCRC)
{
   if (updCRC)
     txCRC = calcCRC16(data, txCRC);
   if (escParse) {
     if (data == 0x0D || data == 0x0A || data == 0x5E) {
        txBuffer[txBufferSize] = 0x5E;
	if (txBufferSize < sizeof(txBuffer))
	   txBufferSize++;
	data += 0x40;
     }
   }
   txBuffer[txBufferSize] = data;
   //printf("txBuffer[%d]=0x%x\n",txBufferSize,txBuffer[txBufferSize]);
   if (txBufferSize < sizeof(txBuffer)) // Assure we are not overloading the txBuffer
     txBufferSize++;
}
void 
DriverSuperK::sendInterbusMessage(unsigned char deviceId, unsigned char registerId, unsigned char msgType, unsigned char *data)
{
    txCRC = 0;
    txBufferSize = 0;
    addToTxMsgData(0x0D, false, false);                     // Start of Telegram
    addToTxMsgData(deviceId, true, true);
    addToTxMsgData(masterId, true, true);
    addToTxMsgData(msgType, true, true);
    addToTxMsgData(registerId, true, true);
    if (msgType == 0x05 && data != NULL)                 // 0x05 = Write messsage - 0x04 = Read message
     addToTxMsgData(data, sizeof(data), true, true);  // Add write data to message
    addToTxMsgData((unsigned char)(txCRC >> 8), true, false);
    addToTxMsgData((unsigned char)txCRC, true, false);
    addToTxMsgData(0x0A, false, false);                     // End of Telegram
		
    string message;
    for (int i = 0 ; i < txBufferSize; i++)
      message[i] = txBuffer[i];
    //printf("0x%x,0x%x,0x%x,0x%x,0x%x\n",message[0],message[1],message[2],message[3],message[4]);
    QString buffersize = "SIZE="+QString::number(txBufferSize);
    comm->Write(message, buffersize.toStdString().c_str());
}



int 
DriverSuperK::ReceiveMessage(unsigned char deviceId, unsigned char registerId, unsigned char msgType, unsigned char *payload)
{
   int rxIntData;
   unsigned char rxCh;
   rxState = Hunting_SOT;
   rxTimeout = timeout_mS;
   string message;
   while (rxState == Hunting_SOT || rxState == Hunting_EOT) {
     rxIntData = comm->Read(message,"BYTE");
     if (rxIntData == -1) { // No RX data ready - Go and wait for 1mS
	if (rxTimeout > 0) {
          rxTimeout--;
	  usleep(1000);
        }
        else 
          rxState = Timeout_Error;
     }
     else {
       // We have RX data ready
       rxCh = (unsigned char) message[0]; 
       if (rxState == Hunting_SOT) {        // Hunting Start Of Telegram
         if (rxCh == 0x0D) { // Got start of telegram
           rxBufferSize = 0;
           for (int i = 0 ; i < sizeof(rxBuffer); i++)
             rxBuffer[i] = 0;
           rxEscape = false;
           rxCRC = 0;
           rxState = Hunting_EOT;
         }
       }
       else if (rxState == Hunting_EOT) {   // Hunting End Of Telegram while collecting telegram
        if (rxCh == 0x0A) { // Got end of telegram
	 if (rxBufferSize >= 5) {   // Dest + Src + Type + msbCRC + lsbCRC - Minimum telegram length
           if (rxCRC == 0) { // We have collected a message with valid CRC - Check the contents
             if (rxBuffer[0] == masterId && rxBuffer[1] == deviceId && rxBuffer[2] == msgType && rxBuffer[3] == registerId){
             // We have a complete telegram - Copy the payload to destination array
		rxBufferSize--;                     // Remove CRC
        	rxBuffer[rxBufferSize] = 0;
		rxBufferSize--;
		rxBuffer[rxBufferSize] = 0;
                QLOG_DEBUG() << "rxBufferSize = " << rxBufferSize;
                for (int i = 4 ; i < rxBufferSize; i++) {
                 if ( rxBuffer[i] == '\0' && i < rxBufferSize - 1)
		   payload[i-4] = ' ';
                 else
                   payload[i-4] = rxBuffer[i];
                }
		rxState = MessageReady;
             }
             else
		rxState = Content_Error;
           }
           else
              rxState = CRC_Error;
	 }
         else
            rxState = Garbage_Error;
        }
        else { // Collecting telegram
         if (rxCh == 0x5E)
           rxEscape = true;    // Got escape sequence
         else { // Got normal telegram contents
           if (rxEscape) {
             rxBuffer[rxBufferSize] = (unsigned char)(rxCh - 0x40);
             rxEscape = false;
           }
           else
             rxBuffer[rxBufferSize] = rxCh;
           rxCRC = calcCRC16(rxBuffer[rxBufferSize], rxCRC);   // Update crc
           if (rxBufferSize < sizeof(rxBuffer))
              rxBufferSize++;
           else
              rxState = Overrun_Error;
	 }
        }
       }
      }
     }
     return rxState;
}
int 
DriverSuperK::extractBit(unsigned short data, int pos) {
   return (data >> pos) & 0x01;
}
