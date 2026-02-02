/// @file ACUsbCom.cpp
/// Class ACUsbCom
/// This class provides a set of operations to perform
/// Usb based communication

// who      when     what
// ----------------------------------------------------------------------------


#include "ACUsbCom.h"
#include <vector>

// ----------------------------------------------------------------------------
/// Operation : Open
/// Opens the communication channel by creating a device_handle and connecting to it
// ----------------------------------------------------------------------------
int
ACUsbCom::Open ()
{
  int r,status,cnt;
 
  struct libusb_config_descriptor *config;
  int configuration_value;
  status = -1;
  _state = CLOSED;

  sscanf(_settings.c_str(), "VENDOR=0x%x,PRODUCT=0x%x", &_device_vendor_id, &_device_product_id);
  QLOG_DEBUG () <<"ACUsbCom::Open> VENDOR =" << _device_vendor_id << " PRODUCT = " << _device_product_id;
  
  _device_handle = NULL; 

  // Temporary case for STANDA controller
if ( _device_vendor_id == 0x10c4 && _device_product_id == 0x0230 )
  {
   status = 0;
   _state = OPEN;
   return status;
  }

  r = libusb_init(NULL);
  
  if (r < 0) ReportError(QString("ACUsbCom::Open> Usb Error Init %1").arg(r));
  
  libusb_set_debug(NULL,3);

  cnt = libusb_get_device_list(NULL, &_device_list);
  
  if (cnt < 0) ReportError(QString("ACUsbCom::Open> No Usb device list found %1").arg(cnt));
  
  r = Check_Device();
 
  if (r < 0)  ReportError(QString("ACUsbCom::Open> Check Device Error %1").arg(r));
  
  r = libusb_open(_device_list[_device_number],&_device_handle);
  
  if (r < 0) ReportError(QString("ACUsbCom::Open> Usb Open Error %1").arg(r));
 
  r = libusb_get_configuration (_device_handle,&configuration_value);
 
  if (r < 0) ReportError(QString("ACUsbCom::Open> Get configuration Error %1").arg(r));

  r = libusb_set_configuration( _device_handle, configuration_value);
 
  if (r < 0) ReportError(QString("ACUsbCom::Open> Set configuration Error %1").arg(r));

  r = libusb_get_active_config_descriptor(_device_list[_device_number],&config);

  if (r < 0 )  {
     ReportError(QString("ACUsbCom::Open> Configuration descriptor error %1").arg(r));
     _interface_number = 0;
  }
  else {
  _interface_number = config->interface->altsetting->bInterfaceNumber;
   libusb_free_config_descriptor(config);
  }

  r = libusb_kernel_driver_active(_device_handle,_interface_number);
  
  if (r < 0) ReportError(QString("ACUsbCom::Open> Usb kernel Error %1").arg(r));
  
  if (r == 1) {
    r = libusb_detach_kernel_driver(_device_handle, _interface_number); 
    if (r < 0) ReportError(QString("ACUsbCom::Open> Usb detach kernel Error %1").arg(r));
  }
  r = libusb_claim_interface(_device_handle, _interface_number);
  
  if (r < 0) 
    ReportError(QString("ACUsbCom::Open> Claim interface Error %1").arg(r));
  else {
    QLOG_DEBUG () <<"ACUsbCom::Open> USB device open ";
    status = 0;
    _state = OPEN;
  }
  return status;
}

// ----------------------------------------------------------------------------
/// Operation : Write
///    sends data through the USB port
/// @message
///    message to send
// ----------------------------------------------------------------------------
int
ACUsbCom::Write (string & message, ...)
{
  va_list args;
  /*--------------------------------------------------------------------------*/
  va_start( args, (&message)[0] );
  char *typewrite = va_arg(args,char*);
  if (typewrite && (sizeof(typewrite) == sizeof(char*))) {
    if (!strcmp(typewrite,"CONTROL")) {
      QLOG_DEBUG () <<"ACUsbCom::Write> WRITE type = " << typewrite;
      uint8_t bmRequestType = (uint8_t)va_arg(args,int);
      uint8_t  bRequest = (uint8_t)va_arg(args,int);
      uint16_t wValue = (uint16_t)va_arg(args,int);
      uint16_t wIndex = (uint16_t)va_arg(args,int);
      unsigned char *data =(unsigned char*)va_arg(args,int);
      uint16_t wLength = (uint16_t)va_arg(args,int);
      unsigned int timeout = va_arg(args,unsigned int);
      QLOG_INFO () << "ACUsbCom::Write> bmRequestType " << bmRequestType << "bRequest " << bRequest
		   << " wValue " << wValue << " wIndex " << wIndex 
                   << " data " << data << " wLength " << wLength << " timeout " << timeout;
      va_end(args);
      return(WriteControlTransfer (message,bmRequestType,bRequest,wValue,wIndex,data,wLength,timeout));
    }
    else if (!strcmp(typewrite,"BULK")) {
      QLOG_DEBUG () <<"ACUsbCom::Write> WRITE type " << typewrite;
      unsigned char endpoint = (unsigned char)va_arg(args,int);
      int *transferred = va_arg(args,int*);
      unsigned int timeout = va_arg(args,unsigned int);
     
      QLOG_DEBUG () <<"ACUsbCom::Write> endpoint " << endpoint << " timeout " << timeout;
      va_end(args);
      return(WriteBulkTransfer (message,endpoint,transferred,timeout));
    }
    else if (!strcmp(typewrite,"INTERRUPT")) {
     QLOG_DEBUG () <<"ACUsbCom::Write> WRITE type " << typewrite;

      unsigned char endpoint = (unsigned char)va_arg(args,int);
      int *transferred = va_arg(args,int*);
      unsigned int timeout = va_arg(args,unsigned int);
      QLOG_DEBUG () <<"ACUsbCom::Write> endpoint " << endpoint << " timeout " << timeout;
      va_end(args);
      return(WriteInterruptTransfer (message,endpoint,transferred,timeout));
    }
  }
  va_end(args);
  return 0;
}
// ----------------------------------------------------------------------------
/// Operation : WriteControlTransfer
///    sends data through the USB port
/// @message
///    message to send
// ----------------------------------------------------------------------------
int
ACUsbCom::WriteControlTransfer (string & message,uint8_t bmRequestType,uint8_t  bRequest,
				uint16_t wValue,uint16_t wIndex, unsigned char *data,
                                uint16_t wLength, unsigned int timeout)
{
  QLOG_DEBUG () << "ACUsbCom::Write> bmRequestType " << bmRequestType << "bRequest " << bRequest
                   << " wValue " << wValue << " wIndex " << wIndex 
                   << " data " << data << " wLength " << wLength << " timeout " << timeout;

  if (libusb_control_transfer(_device_handle, bmRequestType, bRequest, wValue, wIndex, 
			      data, wLength,timeout) < 0 ) {
    ReportWarning("ACUsbCom::WriteControlTransfer> libusb_control_transfer failed");
    return 0;
  }
  return wLength;
}
// ----------------------------------------------------------------------------
/// Operation : WriteBulkTransfer
///    sends data through the USB port
/// @message
///    message to send
// ----------------------------------------------------------------------------
int
ACUsbCom::WriteBulkTransfer (string & message,unsigned char endpoint,int *transferred,
			      unsigned int timeout)
{
  std::vector<unsigned char> command(message.begin(), message.end());
  QLOG_DEBUG () << "ACUsbCom::WriteBulkTransfer> message = " << message.c_str() << " endpoint " << endpoint
	       << " timeout " << timeout;
  if (libusb_bulk_transfer(_device_handle, endpoint, command.data(),
                           static_cast<int>(command.size()),
                           transferred, timeout) < 0 ) {
    ReportWarning("ACUsbCom::WriteBulkTransfer> libusb_bulk_transfer failed");
    return 0;
  }
  if (transferred && *transferred > 0) {
    size_t count = static_cast<size_t>(*transferred);
    if (count > command.size()) {
      count = command.size();
    }
    message.assign(reinterpret_cast<char*>(command.data()), count);
  } else {
    message.clear();
  }
  return (transferred ? *transferred : 0);
}
// ----------------------------------------------------------------------------
/// Operation : WriteInterruptTransfer
///    sends data through the USB port
/// @message
///    message to send
// ----------------------------------------------------------------------------
int
ACUsbCom::WriteInterruptTransfer (string & message,unsigned char endpoint,int *transferred,
			      unsigned int timeout)
{
  std::vector<unsigned char> command(message.begin(), message.end());
  if (libusb_interrupt_transfer(_device_handle, endpoint, command.data(),
				static_cast<int>(command.size()), 
				transferred, timeout) < 0 ) {
    ReportWarning("ACUsbCom::WriteInterruptTransfer> libusb_interrupt_transfer failed");
    return 0;
  }
  if (transferred && *transferred > 0) {
    size_t count = static_cast<size_t>(*transferred);
    if (count > command.size()) {
      count = command.size();
    }
    message.assign(reinterpret_cast<char*>(command.data()), count);
  } else {
    message.clear();
  }
  return (transferred ? *transferred : 0);
}
// ----------------------------------------------------------------------------
/// Operation : Read
///    reads data through the socket
/// @message
///    message to  read
// ----------------------------------------------------------------------------
int
ACUsbCom::Read (string & message, ...)
{
  
  //int mb_addr;
  va_list args;
  /*--------------------------------------------------------------------------*/
  va_start( args, (&message)[0] );
  char *typeread = va_arg(args,char*);
  if (typeread && (sizeof(typeread) == sizeof(char*))) {
    if (!strcmp(typeread,"BULK")) {
      QLOG_DEBUG () <<"ACUsbCom::Read> WRITE type " << typeread;
      unsigned char endpoint = (unsigned char)va_arg(args,int);
      int *transferred = va_arg(args,int*);
      unsigned int timeout = va_arg(args,unsigned int);
      
      QLOG_DEBUG () <<"ACUsbCom::Read> endpoint " << endpoint << " timeout " << timeout;
      va_end(args);
      return(ReadBulkTransfer (message,endpoint,transferred,timeout));
   
    }
    else if (!strcmp(typeread,"INTERRUPT")) {
      QLOG_DEBUG () <<"ACUsbCom::Read> WRITE type " << typeread;
      unsigned char endpoint = (unsigned char)va_arg(args,int);
      int *transferred = va_arg(args,int*);
      unsigned int timeout = va_arg(args,unsigned int);
      QLOG_DEBUG () <<"ACUsbCom::Read> endpoint " << endpoint << " timeout " << timeout;
      va_end(args);
      return(ReadInterruptTransfer (message,endpoint,transferred,timeout));
    
    }
  }
  va_end(args);
  return 0;
}
// ----------------------------------------------------------------------------
/// Operation : ReadBulkTransfer
///    reads data through the USB port
/// @message
///    message to send
// ----------------------------------------------------------------------------
int
ACUsbCom::ReadBulkTransfer (string & message,unsigned char endpoint,int *transferred,
			     unsigned int timeout)
{
  std::vector<unsigned char> answer(64);
  QLOG_DEBUG () <<"ACUsbCom::ReadBulkTransfer> endpoint " << endpoint << " timeout " << timeout;
  if (libusb_bulk_transfer(_device_handle, endpoint, answer.data(),
                           static_cast<int>(answer.size()),
                           transferred, timeout) < 0 ) {
    ReportWarning("ACUsbCom::ReadBulkTransfer> libusb_bulk_transfer failed");
    return 0;
  }
  if (*transferred > 0) {
    message.assign(reinterpret_cast<char*>(answer.data()), *transferred);
  } else {
    message.clear();
  }
  return (*transferred);
}
// ----------------------------------------------------------------------------
/// Operation : ReadInterruptTransfer
///    reads data through the USB port
/// @message
///    message to send
// ----------------------------------------------------------------------------
int
ACUsbCom::ReadInterruptTransfer (string & message,unsigned char endpoint,int *transferred,
				  unsigned int timeout)
{
  std::vector<unsigned char> answer(64);
  if (libusb_interrupt_transfer(_device_handle, endpoint, answer.data(),
				static_cast<int>(answer.size()),
				transferred, timeout) < 0 ) {
    ReportWarning("ACUsbCom::ReadInterruptTransfer> libusb_interrupt_transfer failed");
    return 0;
  }
  if (*transferred > 0) {
    message.assign(reinterpret_cast<char*>(answer.data()), *transferred);
  } else {
    message.clear();
  }
  return (*transferred);
}
// ----------------------------------------------------------------------------
/// Operation : Close
///    Closes the communication
// ----------------------------------------------------------------------------
int
ACUsbCom::Close ()
{
  int r;
  if (_device_handle) {
    r = libusb_release_interface(_device_handle, _interface_number);
    if (r < 0)
      ReportError(QString("ACUsbCom::Close> Usb release interface Error %1").arg(r));
    
    libusb_close(_device_handle);
    
    libusb_free_device_list(_device_list, 1);
    
    libusb_exit(NULL);
  }
  _state = CLOSED;
  return 1;
}
int 
ACUsbCom::Check_Device()
{
  libusb_device *dev;
  int i = 0;
  int status;

  while ((dev = _device_list[i++]) != NULL) {
    struct libusb_device_descriptor desc;
    status = libusb_get_device_descriptor(dev, &desc);
    if (status < 0) {
      ReportError("ACUsbCom::Check_Device> Failed to get device descriptor");
      return status ;
    }
    if (desc.idVendor == _device_vendor_id && desc.idProduct == _device_product_id) {
      QLOG_INFO () <<"ACUsbCom::Check_Device> Found requested device " << i-1 << " at address "  
	     << libusb_get_device_address(dev)
	     << " on bus " << libusb_get_bus_number(dev);
      _device_number = i-1;
      _device_address = libusb_get_device_address(dev);
      status = 1;
      break;
    }
		
  }
  if (_device_number == -1) {
    ReportError("ACUsbCom::Check_Device> Requested device not found");
    status = -1;
  }
  return status;
}
