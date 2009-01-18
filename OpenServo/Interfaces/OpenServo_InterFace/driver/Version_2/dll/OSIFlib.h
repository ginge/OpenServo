/* Open Source InterFace (OSIF) v0.3
  USB to I2C converter. Shared library head information.
    Copyright (C) 2007  Barry Carter <barry.carter@robotfuzz.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

http://www.gnu.org/licenses/gpl.txt

*/

#ifndef OSIF_LIB_H
#define OSIF_LIB_H     //stop recursion

#include "OSIFflash.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>


#define USB_CTRL_IN    (USB_TYPE_CLASS | USB_ENDPOINT_IN)
#define USB_CTRL_OUT   (USB_TYPE_CLASS)

/* the vendor and product id was donated by ftdi ... many thanks!*/
#define OSIF_USB_VID  0x1964
#define OSIF_USB_PID  0x0001

#define I2C_M_RD      0x01

/* commands via USB, must e.g. match command ids firmware */
#define STATUS_ADDRESS_ACK   0
#define STATUS_ADDRESS_NAK   2


// Generic requests
#define USBTINY_ECHO                  0	 // echo test
#define USBTINY_READ                  1	 // read byte (wIndex:address)
#define USBTINY_WRITE                 2  // write byte (wIndex:address, wValue:value)
#define USBTINY_CLR                   3	 // clear bit (wIndex:address, wValue:bitno)
#define USBTINY_SET                   4	 // set bit (wIndex:address, wValue:bitno)
// Programming requests
#define USBTINY_POWERUP               5	 // apply power (wValue:SCK-period, wIndex:RESET)
#define USBTINY_POWERDOWN             6	 // remove power from chip
#define USBTINY_SPI                   7	 // issue SPI command (wValue:c1c0, wIndex:c3c2)
#define USBTINY_POLL_BYTES            8	 // set poll bytes for write (wValue:p1p2)
#define USBTINY_FLASH_READ            9	 // read flash (wIndex:address)
#define USBTINY_FLASH_WRITE           10 // write flash (wIndex:address, wValue:timeout)
#define USBTINY_EEPROM_READ           11 // read eeprom (wIndex:address)
#define USBTINY_EEPROM_WRITE          12 // write eeprom (wIndex:address, wValue:timeout)
// I2C requests
#define USBI2C_READ                   20 // read from i2c bus
#define USBI2C_WRITE                  21 // write to i2c bus
#define USBI2C_STOP                   22 // send stop condition
#define USBI2C_STAT                   23 // get stats from i2c action
#define USBI2C_SET_BITRATE            24
// GPIO requests
#define USBIO_SET_DDR                 30
#define USBIO_SET_OUT                 31
#define USBIO_GET_IN                  32
// USB Serial requests
//

#define STOP_ON 1
#define STOP_OFF 0

#define MAX_ADAPTERS 127

struct usb_bus      *bus;
struct usb_device   *dev;

typedef struct adap_t
{
    usb_dev_handle *adapter_handle;
    int adapter_number;
    char adapter_name[255];
} adapter_t;

adapter_t adapters[MAX_ADAPTERS];
int adapter_count;

typedef struct gpio_state_t
{
    int output_state;
    int ddr;
    int enabled;
} gpios_t;

gpios_t gpio_state[MAX_ADAPTERS];

int  OSIF_USB_write(usb_dev_handle *handle, int request, int value, int index);
int  OSIF_USB_read(usb_dev_handle *handle, unsigned char cmd, void *data, int len);
void OSIF_USB_get_func(usb_dev_handle *handle );
void OSIF_USB_set(usb_dev_handle *handle, unsigned char cmd, int value);
int  OSIF_USB_get_status(usb_dev_handle *handle, char *status);

int  write_data( usb_dev_handle *thandle, int servo, unsigned char * data, int len, int stop);
int  read_data( usb_dev_handle *thandle, int servo, unsigned char * data, int buflen, int stop);
usb_dev_handle *get_adapter_handle(int adapter_no);

int check_params( int val );
#endif
