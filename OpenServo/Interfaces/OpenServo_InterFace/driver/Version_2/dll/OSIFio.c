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

#include "OSIFdll.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int write_data( usb_dev_handle *thandle, int i2c_addr, unsigned char * data, int len, int stop)
{
    /* write one byte register address to chip */
    if (usb_control_msg(thandle, USB_CTRL_OUT, 
        USBI2C_WRITE,
        0, i2c_addr, data, len,
        1000) <1) 
    {
        fprintf(stderr, "wUSB error: %s\n", usb_strerror());
        return -1;
    }

    if (stop == 1)
    {
        usb_control_msg(thandle, USB_CTRL_IN, USBI2C_STOP, 0, 0, 0, 0, 1000);
    }
    return 1;
}

int read_data(usb_dev_handle *thandle, int i2c_addr, unsigned char * data, int buflen, int stop)
{
    int ret = -1;
    ret = usb_control_msg(thandle, 
       USB_CTRL_IN, 
       USBI2C_READ,
       0, i2c_addr, data, buflen,
       1000);
    
    if(ret < 1) 
    {
        char status[100];
        OSIF_USB_get_status(thandle, status);
        fprintf(stderr, "rUSB error: %s i2caddr 0x%02x buflen %d %d\n", usb_strerror(), i2c_addr, buflen, ret);
        return -1;
    }

    if (stop == 1)
    {
        usb_control_msg(thandle, USB_CTRL_IN, USBI2C_STOP, 0, 0, 0, 0, 1000);
    }

    return 1;
}
