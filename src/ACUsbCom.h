/// @file ACUsbCom.h
/// Class ACUsbCom
///
/// This class provides a set of operations to perform communication
/// via USB port.
// who      when     what
// ----------------------------------------------------------------------------


#if !defined(_ACUSBCOMM_H_)
#define _ACUSBCOMM_H_

#include <libusb-1.0/libusb.h>
#include "ACCom.h"

using namespace std;

/// USB communication channel backed by libusb.
class ACUsbCom:public ACCom
{
public:
  
  /** Constructor
   */
  ACUsbCom (string device, string settings):ACCom (device, settings)
  {
  }
  /** Constructor
   */
  ACUsbCom (const ACUsbCom & channel):ACCom (channel),
					_device_handle (channel._device_handle),
					_device_list (channel._device_list),
					_device_product_id (channel._device_product_id),
					_device_vendor_id (channel._device_vendor_id),
					_device_number (channel._device_number),
					_device_address (channel._device_address),
					_interface_number (channel._interface_number)
  {
  }
  /**
     The Open method allows to open the channel
  */
  virtual int Open ();
  
  /**
     The Read method allows to read in the channel
     @param message
     The message to be read
     @param ...
     The parametrization must be either of the following:\n
     - NULL : no default mode (will return a failure).
     - "BULK", endpoint (unsigned char), data (unsigned char *), length (int), 
     transferred (int*), timeout (unsigned int)
     
  */  
  virtual int Read (string & message, ...);

  /**
     The Write method allows to write in the channel
     @param message
     The message to be read
     @param ...
     The parametrization must be either of the following:\n
     - NULL : no default mode (will return a failure).\n
     - "CONTROL",bmRequestType (uint8_t), bRequest (uint8_t), wValue (uint16_t), wIndex (uint16_t) , 
     data (unsigned char *), wLength (uint16_t), timeout (unsigned int)
     - "BULK", endpoint (unsigned char), data (unsigned char *), length (int), 
     transferred (int*), timeout (unsigned int)
     - "INTERRUPT", endpoint (unsigned char), data (unsigned char *), 
     length (int), transferred (int*), timeout (unsigned int)
     The message string contains the result.\n
   
  */  
  virtual int Write (string & message, ...);
 
  /**
     The Close method allows to close the channel
  */
  virtual int Close ();

private:
  
  /** The WriteControlTransfer method uses libusb_control_transfer function
      to write data.
      @param  message
      A string message to be written (should be 1 byte message hexadecimal value)
      @param bmRequestType 	
      The request type field for the setup packet
      @param 	bRequest 	
      The request field for the setup packet
      @param 	wValue 	
      The value field for the setup packet
      @param 	wIndex
      The index field for the setup packet
      @param    data
      The data pointer
      @param    wLength 
      The (expected) data length    
      @param 	timeout 	
      Timeout (in millseconds) that this function should wait 
      before giving up due to no response being received. 
      For an unlimited timeout, use value 0. 
  */
  virtual int WriteControlTransfer (string & message, 
				    uint8_t bmRequestType,
				    uint8_t  bRequest,
				    uint16_t wValue,
				    uint16_t wIndex,
				    unsigned char *data,
			            uint16_t wLength, 
				    unsigned int timeout);
  /** The WriteBulkTransfer method uses libusb_bulk_transfer function
      to write the value.
      @param  message
      A string message to be written 
      @param endpoint 	
      The address of a valid endpoint to communicate with
      @param  transferred 	
      Output location for the number of bytes actually transferred.
      @param	timeout 	
      Timeout (in millseconds) that this function should 
      wait before giving up due to no response being received. 
      For an unlimited timeout, use value 0.       
  */
  virtual int WriteBulkTransfer (string & message,
				 unsigned char endpoint,
				 int *transferred,
				 unsigned int timeout);
  /** The WriteInterruptTransfer method uses libusb_interrupt_transfer function
      to write the value.
      @param  message
      A string message to be written
      @param endpoint 	
      The address of a valid endpoint to communicate with
      @param  transferred 	
      Output location for the number of bytes actually transferred.
      @param	timeout 	
      Timeout (in millseconds) that this function should 
      wait before giving up due to no response being received. 
      For an unlimited timeout, use value 0.       
  */
  virtual int WriteInterruptTransfer (string & message,
				      unsigned char endpoint,
				      int *transferred,
				      unsigned int timeout);
  /** The ReadBulkTransfer method uses libusb_bulk_transfer function
      to read the value.
      @param  message
      A string message to be read
      @param endpoint 	
      The address of a valid endpoint to communicate with
      @param  transferred 	
      Output location for the number of bytes actually transferred.
      @param	timeout 	
      Timeout (in millseconds) that this function should 
      wait before giving up due to no response being received. 
      For an unlimited timeout, use value 0.       
  */
  virtual int ReadBulkTransfer (string & message,
				unsigned char endpoint,
				int *transferred,
				unsigned int timeout);
  /** The ReadInterruptTransfer method uses libusb_interrupt_transfer function
      to read data.
      @param  message
      A string message to be read
      @param endpoint 	
      The address of a valid endpoint to communicate with
      @param  transferred 	
      Output location for the number of bytes actually transferred.
      @param	timeout 	
      Timeout (in millseconds) that this function should 
      wait before giving up due to no response being received. 
      For an unlimited timeout, use value 0.       
  */
  virtual int ReadInterruptTransfer (string & message,
				     unsigned char endpoint,
				     int *transferred,
				     unsigned int timeout);
  
  /** The Check_Device method checks the existence of the requested device (used in Open method)
  */
  virtual int Check_Device();

  /** The USB device handle object
   */
  struct libusb_device_handle *  _device_handle;
  struct libusb_device **_device_list;
  int _device_product_id;
  int _device_vendor_id;
  int _device_number;
  int _device_address;
  int _interface_number;
};

#endif // !defined(_ACUSBCOMM_H_)
