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

int write_data( usb_dev_handle *thandle, int servo, unsigned char * data, size_t len, int stop)
{
    /* write one byte register address to chip */
    if (usb_control_msg(thandle, USB_CTRL_OUT, 
        USBI2C_WRITE,
        0, servo, data, len, 
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

int read_data(usb_dev_handle *thandle, int servo, unsigned char * data, size_t buflen, int stop)
{
    if(usb_control_msg(thandle, 
       USB_CTRL_IN, 
       USBI2C_READ,
       0, servo, data, buflen, 
       1000) <1) 
    {
        fprintf(stderr, "rUSB error: %s\n", usb_strerror());
        return -1;
    }

    if (stop == 1)
    {
        usb_control_msg(thandle, USB_CTRL_IN, USBI2C_STOP, 0, 0, 0, 0, 1000);
    }

    return 1;
}
