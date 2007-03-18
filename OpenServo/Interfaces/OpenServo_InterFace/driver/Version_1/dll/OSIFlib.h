#ifndef OSIF_LIB_H
#define OSIF_LIB_H     //stop recursion
 
#include "OSIFflash.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>


#define USB_CTRL_IN    (USB_TYPE_CLASS | USB_ENDPOINT_IN)
#define USB_CTRL_OUT   (USB_TYPE_CLASS)

/* the vendor and product id was donated by ftdi ... many thanks!*/
#define OSIF_USB_VID  0x0403
#define OSIF_USB_PID  0xc631

#define I2C_M_RD		0x01

/* commands via USB, must e.g. match command ids firmware */
#define CMD_ECHO       0
#define CMD_GET_FUNC   1
#define CMD_SET_DELAY  2
#define CMD_GET_STATUS 3
#define CMD_I2C_IO     4
#define CMD_I2C_BEGIN  1  // flag to I2C_IO
#define CMD_I2C_END    2  // flag to I2C_IO

#define STATUS_IDLE          0
#define STATUS_ADDRESS_ACK   1
#define STATUS_ADDRESS_NAK   2

struct usb_bus      *bus;
struct usb_device   *dev;

typedef struct adap_t
{
	usb_dev_handle *adapter_handle;
	int adapter_number;
	char adapter_name[255];
} adapter_t;

adapter_t adapters[127];
int adapter_count;

int OSIF_USB_write(usb_dev_handle *handle, int request, int value, int index);
int OSIF_USB_read(usb_dev_handle *handle, unsigned char cmd, void *data, int len);
void OSIF_USB_get_func(usb_dev_handle *handle );
void OSIF_USB_set(usb_dev_handle *handle, unsigned char cmd, int value);
int OSIF_USB_get_status(usb_dev_handle *handle);
		
int write_data( usb_dev_handle *thandle, int servo, unsigned char * data, size_t len);
int read_data( usb_dev_handle *thandle, int servo, unsigned char * data, size_t buflen);
usb_dev_handle *get_adapter_handle(int adapter_no);
#endif
