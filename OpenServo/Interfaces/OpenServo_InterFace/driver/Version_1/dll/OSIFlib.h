#ifndef OSIF_LIB_H
#define OSIF_LIB_H     //stop recursion

#include "usb.h"    
#include "OSIFflash.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>


#define USB_CTRL_IN    (USB_TYPE_CLASS | USB_ENDPOINT_IN)
#define USB_CTRL_OUT   (USB_TYPE_CLASS)

/* the vendor and product id was donated by ftdi ... many thanks!*/
#define I2C_TINY_USB_VID  0x0403
#define I2C_TINY_USB_PID  0xc631

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
usb_dev_handle *handle;

struct usb_bus      *bus;
struct usb_device   *dev;

int i2c_tiny_usb_write(int request, int value, int index);
int i2c_tiny_usb_read(unsigned char cmd, void *data, int len);
void i2c_tiny_usb_get_func(void);
void i2c_tiny_usb_set(unsigned char cmd, int value);
int i2c_tiny_usb_get_status(void);
int i2c_read_with_cmd(unsigned char addr, char cmd, int length);
int i2c_write_byte(unsigned char addr, char data);
int i2c_write_cmd_and_byte(unsigned char addr, char cmd, char data);
int i2c_write_cmd_and_word(unsigned char addr, char cmd, int data);
		
#endif
