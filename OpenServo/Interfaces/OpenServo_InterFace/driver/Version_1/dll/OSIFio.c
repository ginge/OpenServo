// OSIF Libusb driver
#include "OSIFdll.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int write_data( usb_dev_handle *thandle, int servo, unsigned char * data, size_t len)
{

  /* write one byte register address to chip */
  if(usb_control_msg(thandle, USB_CTRL_OUT, 
		     CMD_I2C_IO + CMD_I2C_BEGIN + CMD_I2C_END,
		     0, servo, data, len, 
		     1000) < 1) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    printf("USB error: %s\n", usb_strerror());
    return -1;
  } 

  return 1;
}

int read_data(usb_dev_handle *thandle, int servo, unsigned char * data, size_t buflen)
{
  if(usb_control_msg(thandle, 
		     USB_CTRL_IN, 
		     CMD_I2C_IO + CMD_I2C_END,
		     I2C_M_RD, servo, data, buflen, 
		     1000) < 1) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    return -1;
  } 
}
