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


/* write a set of bytes to the OSIF_usb device */
int OSIF_USB_write(usb_dev_handle *handle, int request, int value, int index) {
    if(usb_control_msg(handle, USB_CTRL_OUT, request,
       value, index, NULL, 0, 1000) < 0) {
        fprintf(stderr, "usbwUSB error: %s\n", usb_strerror());
        return -1;
    }
    return 1;
}

/* read a set of bytes from the OSIF_usb device */
int OSIF_USB_read(usb_dev_handle *handle, unsigned char cmd, void *data, int len) {
    int nBytes;

    /* send control request and accept return value */
    nBytes = usb_control_msg(handle,
        USB_CTRL_IN, 
        cmd, 0, 0, (char *)data, len, 1000);

    if(nBytes < 0) {
        fprintf(stderr, "usbrUSB error: %s\n", usb_strerror());
        return nBytes;
    }

    return 0;
}

/* set a value in the I2C_USB interface */
void OSIF_USB_set(usb_dev_handle *handle, unsigned char cmd, int value) {
    if(usb_control_msg(handle, 
        USB_TYPE_VENDOR, cmd, value, 0, 
        NULL, 0, 1000) < 0) {
            fprintf(stderr, "setUSB error: %s\n", usb_strerror());
    }
}

/* get the current transaction status from the OSIF_usb interface */
int OSIF_USB_get_status(usb_dev_handle *handle) {
    int i;
    unsigned char status;

    if((i=OSIF_USB_read(handle, USBI2C_STAT, &status, sizeof(status))) < 0) {
        fprintf(stderr, "Error reading status\n");
        return i;
    }

    return status;
}

/** Initialise the OSIF USB interface **/
EXPORT int OSIF_init(void)
{
#ifndef WIN
    int ret;
#endif
    usb_dev_handle *handle;
    int n;

    usb_init();

    usb_find_busses();
    usb_find_devices();
    adapter_count = -1;

    for(bus = usb_get_busses(); bus; bus = bus->next) 
    {
        for(dev = bus->devices; dev; dev = dev->next) 
        {
            if((dev->descriptor.idVendor == OSIF_USB_VID) && 
               (dev->descriptor.idProduct == OSIF_USB_PID)) 
            {
                /* open device */
                if(!(handle = usb_open(dev))) 
                {
#ifdef DEBUG_OUT
                    printf("Could not open OSIF device\n");
#endif
                    return -1;
                }
                adapter_count++;
                adapters[adapter_count].adapter_number = adapter_count;
                adapters[adapter_count].adapter_handle = handle;
                sprintf( adapters[adapter_count].adapter_name, "OSIF_%d", OSIF_USB_PID );
#ifdef DEBUG_OUT
                printf("adapter init handle %d %d\n", adapters[adapter_count].adapter_handle, handle);
#endif
                // For each interface we are going to send the powerup/powerdown sequence
                // This sets the ports on the AVR to allow very fast I2C comms
                if (usb_control_msg(handle, USB_CTRL_IN, 
                    USBTINY_POWERUP,
                    20, 1, 0, 0, 
                    500) <1) {

                    }
                if (usb_control_msg(handle, USB_CTRL_IN, 
                    USBTINY_POWERDOWN,
                    0, 0, 0, 0, 
                    500) <1) {
                    }

                break;
            }
        }
    }

    if(!handle || adapter_count < 0) {
        adapter_count = -1;
        return -1;
    }

#ifndef WIN
    /* Get exclusive access to interface 0. Does not work under windows. */
    ret = usb_claim_interface(handle, 0);

    if (ret != 0) {
        fprintf(stderr, "claimUSB error: %s\n", usb_strerror());
    }
#endif
    return 0;
}

/** De-Initialise the OSIF USB interface **/
EXPORT int OSIF_deinit(void)
{
    usb_dev_handle *handle;

    //for each interface, disable
    int n;
    for (n=0;n<=adapter_count;n++)
    {
        handle = adapters[n].adapter_handle;

#ifndef WIN
        int ret;
        ret = usb_release_interface(handle, 0);
        if (ret)
            fprintf(stderr, "releaseUSB error: %s\n", usb_strerror());
#endif
        usb_close(handle);
    }
    return 1;
}

/** Write data to the I2C device at addr **/
EXPORT int OSIF_write(int adapter, int servo, unsigned char addr, unsigned char * data, size_t buflen )
{
    usb_dev_handle *handle;
    handle = get_adapter_handle(adapter);
    char msg[65];
    char newbuf[255];
    char tmpbuf[255];
    int n=0;

    if (check_params( servo )<0)
    {
#ifdef DEBUG_OUT
        printf("Data outside bounds. Use 0 -> 127 range\n");
#endif
        return -1;
    }

    if ( buflen > 64 )
        return -1;

    msg[0] = addr;

    //sprintf(newbuf, "data ");
    for (n=0; n<buflen;n++)
    {
        msg[n+1]=data[n];
        sprintf( tmpbuf, "0x%02x ", data[n]);
        strcat(newbuf, tmpbuf);	
    }
#ifdef DEBUG_OUT
    printf("adapter %d, servo %d, addr %d, %s, buflen %d, msg %s\n",adapter,servo,addr,newbuf, buflen,msg);
#endif
    if (write_data( handle, servo, msg, buflen+1 ) <0 ) { return -1; }

    if(OSIF_USB_get_status(handle) != STATUS_ADDRESS_ACK) {
        fprintf(stderr, "write command status failed %d\n",OSIF_USB_get_status(handle));
        return -1;
    }
#ifdef DEBUG_OUT
    printf("write okay\n");
#endif
    return 0;
}

/** read from the I2C device at address addr **/
EXPORT int OSIF_read(int adapter, int servo, unsigned char addr, unsigned char * data, size_t buflen )
{
    usb_dev_handle *handle;
    handle = get_adapter_handle(adapter);

    if (check_params( servo )<0)
    {
        fprintf(stderr, "Data outside bounds. Use 0 -> 127 range\n");
        return -1;
    }

    if (write_data( handle, servo, &addr, 1 ) <0 ) { return -1; }

    if(OSIF_USB_get_status(handle) != STATUS_ADDRESS_ACK) {
        fprintf(stderr, "write command status failed\n");
        return -1;
    }

    if (read_data( handle, servo, data, buflen ) <0 ) { return -1; }

    if(OSIF_USB_get_status(handle) != STATUS_ADDRESS_ACK) {
        fprintf(stderr, "read data status failed\n");
        return -1;
    }

    return 1;
}

/** perform a I2C read without the register select write **/
EXPORT int OSIF_readonly(int adapter, int servo, unsigned char * data, size_t buflen )
{
    usb_dev_handle *handle;
    handle = get_adapter_handle(adapter);

    if (check_params( servo )<0)
    {
        fprintf(stderr, "Data outside bounds. Use 0 -> 127 range\n");
        return -1;
    }

    if (read_data( handle, servo, data, buflen ) <0 ) { return -1; }

    if(OSIF_USB_get_status(handle) != STATUS_ADDRESS_ACK) {
        fprintf(stderr, "read data status failed\n");
        return -1;
    }

    return 1;
}

/** Scan the I2C bus for devices. Fills pointer *devices with 1d array of devices on bus and *dev_count with number found **/
EXPORT int OSIF_scan(int adapter, int devices[], int *dev_count )
{
    usb_dev_handle *handle;
    int i;
    int res;
    char addr;

    handle = get_adapter_handle(adapter);

    *dev_count = 0;
    //search all addresses for a response
    for (i=0x02; i<=MAX_I2C_DEVICES; i++) 
    {
        addr=0x00;
        if (write_data( handle, i, &addr, 1 ) <0 ) { return -1; }

        if(OSIF_USB_get_status(handle) != STATUS_ADDRESS_ACK) {
#ifdef DEBUG_OUT
            printf("no device at 0x%02x\n", i );//
#endif
        }
        else
        {
            printf("found device 0x%02x\n", i);
            devices[*dev_count] = i;
            *dev_count+=1;
#ifdef DEBUG_OUT
            printf ("%x\n", *dev_count);
#endif
        }
#ifndef WIN
//          usleep(1000);
#else
        Sleep(1);
#endif
    }
    return 1;
}

/** returns true if a device is found at address servo **/
EXPORT bool OSIF_probe(int adapter, int servo )
{
    usb_dev_handle *handle;
    char addr;

    addr=0x00;

    handle = get_adapter_handle(adapter);

    if (check_params( servo )<0)
    {
        fprintf(stderr, "Data outside bounds. Use 0 -> 127 range\n");
        return -1;
    }

    if (write_data( handle, servo, &addr, 1 ) <0 ) { return false; }

    if(OSIF_USB_get_status(handle) == STATUS_ADDRESS_ACK)
    {
#ifdef DEBUG_OUT
        printf("found device 0x%02x\n", servo);
#endif
        return true;
    }

    return false;
}

/** Send a command to the OpenServo. See website for commands **/
EXPORT int OSIF_command(int adapter, int servo, unsigned char command)
{
    usb_dev_handle *handle;
    handle = get_adapter_handle(adapter);

    if (check_params( servo )<0)
    {
        fprintf(stderr, "Data outside bounds. Use 0 -> 127 range\n");
        return -1;
    }

    if (write_data( handle, servo, &command, 1 ) <0 ) { return -1; }

    if(OSIF_USB_get_status(handle) != STATUS_ADDRESS_ACK) {
        fprintf(stderr, "write command status failed\n");
        return -1;
    }
#ifdef DEBUG_OUT
    printf( "Command Sent\n" );
#endif
    return 0;
}

/** Returns number of connected OSIF adapters **/
EXPORT int OSIF_get_adapter_count(void)
{
#ifdef DEBUG_OUT	
    printf( "adapter count %d\n", adapter_count);
#endif
    return adapter_count;
}

/** Returns the friendly name of the connected adapter **/
EXPORT int OSIF_get_adapter_name(int adapter, char* name)
{
    strcpy( name, adapters[adapter].adapter_name );
}

/** Check the validity of the options **/
int check_params( int val )
{
    if (val > 127 || val <0 )
    {
        return -1;
    }
    return 1;
}

/** Get the handle of the selected adapter **/
usb_dev_handle * get_adapter_handle(int adapter_no )
{
    if (adapter_no > adapter_count )
    {
        return NULL;
    }

    return adapters[adapter_no].adapter_handle;
}
