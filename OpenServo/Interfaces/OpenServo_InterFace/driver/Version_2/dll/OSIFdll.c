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
/*
	OSIF (Open Source InterFace) hardware wrapper.
	The OSIF uses libusb for all communications with the hardware. We 
	use USB control messages to send data back and forth to the OSIF. This
	is because we want to keep the USB protocol communication down to a bare
	minimum for certain transactions. The control message controls directly
	which functions in the OSIF are executed.
	A full data read or write uses a standard USB endpoint.
	

	TODO:
		add support for nix /dev/i2c-x kernel driver
		add gpio support


*/
#include "OSIFdll.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DEBUG_OUT

/* Perform a setup data transfer without a full usb transfer */
int OSIF_USB_write(usb_dev_handle *handle, int request, int value, int index)
{
	/* OSIF communicates over libusb using control transfers for data read/writes */
    if(usb_control_msg(handle, USB_CTRL_OUT, request,
       value, index, NULL, 0, 1000) < 0)
	{
        fprintf(stderr, "OSIF_USB_write: error %s\n", usb_strerror());
        return -1;
    }
    return 1;
}

/* read a set of bytes from the OSIF_usb device */
int OSIF_USB_read(usb_dev_handle *handle, unsigned char cmd, void *data, int len)
{
    int num_bytes;

    /* send control request and accept return value */
    num_bytes = usb_control_msg(handle,
        USB_CTRL_IN, 
        cmd, 0, 0, (char *)data, len, 1000);

    if(num_bytes < 0)
	{
        fprintf(stderr, "OSIF_USB_read: error %s\n", usb_strerror());
        return num_bytes;
    }

    return 1;
}

/* set a value in the I2C_USB interface */
void OSIF_USB_set(usb_dev_handle *handle, unsigned char cmd, int value)
{
    if(usb_control_msg(handle, 
        USB_TYPE_VENDOR, cmd, value, 0, 
        NULL, 0, 1000) < 0)
	{
            fprintf(stderr, "OSIF_USB_set: error %s\n", usb_strerror());
    }
}

/* get the current transaction status from the OSIF_usb interface */
int OSIF_USB_get_status(usb_dev_handle *handle, char *status)
{
    int i;

    if((i=OSIF_USB_read(handle, USBI2C_STAT, status, 10)) < 0)
	{
        fprintf(stderr, "OSIF_USB_get_status: Error reading status\n");
        return i;
    }

    if (status[0] == 0x20) //NACK
        return -1;

#ifdef DEBUG_OUT
    printf("status ");
    for (i=0;i<=10;i++)
    {
        printf("0x%02x ",status[i]);
    }
    printf("\n");
#endif
    return 1;
}

/**
	Initialise the OSIF USB interface. Enumerates all connected OSIF devices.

	returns: <0 error 1 success
**/
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
                    printf("OSIF_init: Could not open OSIF device\n");
#endif
                    return -1;
                }
				/* Assign our global variables. */
                adapter_count++;
                adapters[adapter_count].adapter_number = adapter_count;
                adapters[adapter_count].adapter_handle = handle;
                sprintf( adapters[adapter_count].adapter_name, "OSIF_%d", OSIF_USB_PID );
#ifdef DEBUG_OUT
                printf("OSIF_init: handle %d %d\n", (int)adapters[adapter_count].adapter_handle, (int)handle);
#endif
                /* For each interface we are going to send the powerup/powerdown sequence
                   This sets the ports on the AVR to allow very fast I2C comms
				*/
/* DISABLED FOR NOW AS THIS CAUSES A SPIKE ON THE I2C BUS SOME DEVICES DO NOT LIKE
	INSTEAD USE SET_BIRATE
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
*/
                  usb_set_configuration(handle, dev->config->bConfigurationValue);

#ifndef WIN
                 /* Get exclusive access to interface 0. Does not work under windows. */
                 ret = usb_claim_interface(handle, 0);

                 if (ret != 0)
				 {
                     fprintf(stderr, "claimUSB error: %s\n", usb_strerror());
                 }
#endif

                break;
            }
        }

    }

    if(!handle || adapter_count < 0)
	{
        adapter_count = -1;
        return -1;
    }

    return 1;
}

/**
	De-Initialise the OSIF USB interface
**/
EXPORT int OSIF_deinit(void)
{
    usb_dev_handle *handle;

    /* for each interface, disable */
    int n;
    for (n=0;n<=adapter_count;n++)
    {
        handle = adapters[n].adapter_handle;

#ifndef WIN
        int ret;
        ret = usb_release_interface(handle, 0);
        if (ret)
            fprintf(stderr, "OSIF_deinit: error %s\n", usb_strerror());
#endif
        usb_close(handle);
    }
    return 1;
}

/**
	Return the OSIF library version for compatibility checks

	data: a pointer to a string to be filled with the nice name for
		  the library.

	returns: an integer of the version number in  majorminor xxyy
**/
EXPORT int OSIF_get_libversion(unsigned char * data)
{
	sprintf(data, "v%02d%02d\n", LIB_VERSION_MAJOR, LIB_VERSION_MINOR);
	return ((LIB_VERSION_MAJOR<<8)|LIB_VERSION_MINOR);
}

/**
	Write data to the I2C device.
	This will start an I2C transaction (with automatic restart)
	and write buflen bytes to address addr

	This assumes the device needs a register selection before
	doing the write. Some devices don't require this, and
	you should either use OSIF_writeonly or put the first byte
	to write in the addr register and start writing from +1
	offset in data.

	adapter: integer of the adapter scanned. 0 indexed.
	i2c_addr: integer address of the device.
	addr: the register address in the device to read
	data: passed in buffer to be filled
	buflen: number of bytes to read
	issue_stop: issue the stop bit at the end of the transaction?

	returns: <0 error 1 success
**/
EXPORT int OSIF_write_data(int adapter, int i2c_addr, unsigned char addr, unsigned char * data, int buflen, int issue_stop )
{
	char newbuf[1024];
	char tmpbuf[1024];
	char msg[255];
	int n=0;

	msg[0] = addr;

	/* expensive to do it this way, but safer. */
	sprintf( newbuf, "");
	for (n=0; n<buflen;n++)
	{
		msg[n+1]=data[n];
		sprintf( tmpbuf, "0x%02x ", data[n]);
        fprintf( stderr, "0x%02x ", data[n]);
		strcat(newbuf, tmpbuf);
	}
	return OSIF_writeonly(adapter, i2c_addr, msg, buflen+1, issue_stop);
}

/** Shortcut to the above function. This will always send a stop at the end of the write **/
EXPORT int OSIF_write(int adapter, int i2c_addr, unsigned char addr, unsigned char * data, int buflen )
{
	return OSIF_write_data(adapter, i2c_addr, addr, data, buflen, STOP_ON );
}
		
/**
	Write data to the I2C device.
	This will start an I2C transaction (with automatic restart)
	and write buflen bytes to address addr

	This assumes the device does NOT need a register selection before
	doing the write. Some devices do require this, and
	you should either use OSIF_write or put the register selection
	byte at element 0 in your data string

	adapter: integer of the adapter scanned. 0 indexed.
	i2c_addr: integer address of the device.
	addr: the register address in the device to read
	data: passed in buffer to be filled
	buflen: number of bytes to read. Current hardware limit is 64 bytes
	issue_stop: do we want to send the I2C stop signal after the
				transaction? issue_stop will switch off the request

	returns: <0 error 1 success
**/
EXPORT int OSIF_writeonly(int adapter, int i2c_addr, unsigned char * data, int buflen, int issue_stop )
{
    usb_dev_handle *handle;
    handle = get_adapter_handle(adapter);
    int ret;

    if (check_params( i2c_addr )<0)
    {
#ifdef DEBUG_OUT
        printf("Data outside bounds. Use 0 -> 127 range\n");
#endif
        return -1;
    }

    if ( buflen > 64 )
	{
		printf("Data length too long. 64 byte limit\n");
        return -1;
	}

#ifdef DEBUG_OUT
    printf("adapter %d, i2c_addr %d, addr %d, %s, buflen %d\n", adapter, i2c_addr, data[0], data, (int)buflen);
#endif

    if (write_data( handle, i2c_addr, data, buflen, issue_stop ) <0 )
	{
		fprintf(stderr, "OSIF_writeonly: Serious error writing\n");
		return -1;
	}

    char status[255];
    if(OSIF_USB_get_status(handle, status)<0)
	{
		fprintf(stderr, "OSIF_writeonly: write command status failed 0x%02x\n",status[0]);
        return -1;
    }
#ifdef DEBUG_OUT
    printf("write okay\n");
#endif
    return 1;
}

/**
	Read from the I2C device at address addr
	will fill data into the read buffer.

	Note:
	This function will do a write before a read
	with a restart.

	adapter: integer of the adapter scanned. 0 indexed.
	i2c_addr: integer address of the device.
	addr: the register address in the device to read
	data: passed in buffer to be filled
	buflen: number of bytes to read. read in small chunks. 64 bytes is a realistic figure
	issue_stop: issue the stop bit at the end of the transaction?

	returns: <0 error 1 success
**/
EXPORT int OSIF_read_data(int adapter, int i2c_addr, unsigned char addr, unsigned char * data, int buflen, int issue_stop )
{
    usb_dev_handle *handle;
    handle = get_adapter_handle(adapter);

    if (check_params( i2c_addr )<0)
    {
        fprintf(stderr, "Data outside bounds. Use 0 -> 127 range\n");
        return -1;
    }

    if (write_data( handle, i2c_addr, &addr, 1,  STOP_OFF) <0 )
	{
      printf( "write error: OSIF_read\n");
      return -1; 
    }

    char status[255];
	if(OSIF_USB_get_status(handle, status) <0)
	{
		fprintf(stderr, "write command status failed\n");
		return -1;
	}

    if (OSIF_readonly(adapter, i2c_addr, data, buflen, issue_stop) <0)
	{
		fprintf(stderr, "OSIF_read: error reading\n");
	}

#ifdef DEBUG_OUT
    int n;
    char tmpbuf[1024];
    char newbuf[1024];
    sprintf( newbuf, "");
    for (n=0; n<buflen;n++)
    {
        sprintf( tmpbuf, "0x%02x ", data[n]);
        strcat(newbuf, tmpbuf);	
    }

    printf("adapter %d, i2c_addr %d, addr %d, %s, buflen %d\n",adapter,i2c_addr,addr,newbuf, buflen);

    printf("OSIF_read: read okay\n");
#endif
    return 1;
}

/** Shortcut to the above function with an I2C stop bit **/
EXPORT int OSIF_read(int adapter, int i2c_addr, unsigned char addr, unsigned char * data, int buflen)
{
	return OSIF_read_data(adapter, i2c_addr, addr, data, buflen, STOP_ON);
}

/**
	Read from the I2C device at address addr
	will fill data into the read buffer.

	Note:
	This function will NOT do a write before a read
	it will only perform a read. Make sure the I2C
	device is setup for this read only transfer
	by using OSIF_write, or alternatively be
	sure your device supports this method of communication

	adapter: integer of the adapter scanned. 0 indexed.
	i2c_addr: integer address of the device.
	data: passed in buffer to be filled
	issue_stop: issue the stop bit at the end of the transaction?

	returns: <0 error 1 success
 **/
EXPORT int OSIF_readonly(int adapter, int i2c_addr, unsigned char * data, int buflen, int issue_stop )
{
    usb_dev_handle *handle;
    handle = get_adapter_handle(adapter);

    if (check_params( i2c_addr )<0)
    {
        fprintf(stderr, "Device address outside bounds. Use 0 -> 127 range\n");
        return -1;
    }

    if (read_data( handle, i2c_addr, data, buflen, issue_stop ) <0 ) { return -1; }

    char status[255];
    if(OSIF_USB_get_status(handle, status) <0) {
        fprintf(stderr, "OSIF_readonly: Device 0x%02x did not ACK\n", i2c_addr);
        return -1;
    }

    return 1;
}

/**
	Scan the I2C bus for devices.

	Scan the I2C bus by addressing all devices (0x01 to 0x7F) in turn and waiting
	to see if we get an ACK
	Note not all devices work like this, and can send some devices into an unknown
	state. BE CAREFUL.

	adapter: integer of the adapter scanned. 0 indexed.
	devices: *devices with 1d array of devices on bus and *dev_count with number found
	dev_count: returns the number of devices found on the bus

	returns: <0 error 1 success
**/
EXPORT int OSIF_scan(int adapter, int devices[], int *dev_count )
{
    usb_dev_handle *handle;
    int i;
    int res;
    char addr;
    handle = get_adapter_handle(adapter);

    *dev_count = 0;
	
    /* search all addresses for a response */
    for (i=0x01; i<MAX_I2C_DEVICES; i++)
    {
        addr=0x00;
        if ((write_data( handle, i, &addr, 1, STOP_ON )) <0 ) { return -1; }

        char status[255];
        if(OSIF_USB_get_status(handle, status) <0) {
#ifdef DEBUG_OUT
            printf("No device at 0x%02x\n", i );
#endif
        }
        else
        {
            printf("Found device at address 0x%02x, dc %d\n", i, *dev_count);
            devices[lc] = i;
            (*dev_count)++;
#ifdef DEBUG_OUT
            printf ("OSIF_scan: dev count %d\n", *dev_count);
#endif
        }
#ifndef WIN
/*          usleep(1000); */
#else
        Sleep(1);
#endif
    }
    return 1;
}

/**
	Probe a device at a given address to see if it will ACK

	adapter: integer of the adapter scanned. 0 indexed.
	i2c_addr: integer address of the device.

	returns: true if a device is found at address
**/
EXPORT bool OSIF_probe(int adapter, int i2c_addr )
{
    usb_dev_handle *handle;
    char addr;

    addr=0x00;

    handle = get_adapter_handle(adapter);

    if (check_params( i2c_addr )<0)
    {
        fprintf(stderr, "Data outside bounds. Use 0 -> 127 range\n");
        return -1;
    }

    if (write_data( handle, i2c_addr, &addr, 1, STOP_ON ) <0 ) { return false; }

    char status[255];
    if(OSIF_USB_get_status(handle, status) >=0)
    {
#ifdef DEBUG_OUT
        printf("OSIF_probe: Found device at address 0x%02x\n", i2c_addr);
#endif
        return true;
    }

    return false;
}

/**
	Write 1 to a register in the device in one transaction.
	generally used for "command" functions in I2C slave
	devices that will trigger a function from a write to
	a register.

	adapter: integer of the adapter scanned. 0 indexed.
	i2c_addr: integer address of the device.
	command: the register to write to.

	returns: <0 error 1 success
**/
EXPORT int OSIF_command(int adapter, int i2c_addr, unsigned char command)
{
    usb_dev_handle *handle;
    handle = get_adapter_handle(adapter);

    if (check_params( i2c_addr )<0)
    {
        fprintf(stderr, "Data outside bounds. Use 0 -> 127 range\n");
        return -1;
    }

    if (write_data( handle, i2c_addr, &command, 1, STOP_ON ) <0 ) { return -1; }

    char status[255];
    if(OSIF_USB_get_status(handle, status) <0) {
        fprintf(stderr, "OSIF_command: Got no ACK\n");
        return -1;
    }
#ifdef DEBUG_OUT
    printf( "Command Sent\n" );
#endif
    return 1;
}

/**
	Get a count of the connected OSIF adapters:

	returns: number of connected OSIF adapters
**/
EXPORT int OSIF_get_adapter_count(void)
{
#ifdef DEBUG_OUT	
    printf( "OSIF_get_apapter_count: adapter count (physical) %d\n", adapter_count+1);
#endif
    return adapter_count;
}

/**
	Query the connected OSIF for its name.
	May also be used for firmware version.

	adapter: integer of the adapter scanned. 0 indexed.
	name: character string filled with the name

	returns: always 1
**/
EXPORT int OSIF_get_adapter_name(int adapter, char* name)
{
    strcpy( name, adapters[adapter].adapter_name );
    return 1;
}

/**
    GPIO Control function to set the direction of the pins, and is enabled
        If you set the I2C pins (SDA SCL) as outputs, you will disable the
        i2c module. Be warned!
    adapter: integer of the adapter scanned. 0 indexed.
    ddr: a bitwise OR of the pins to set as input(0) or output(1)
        eg gpio1(TX) and gpio2(RX)  00000011 or 0x03
    enabled: a bitwise OR to enable the pin (1) for future writes. Can be used as a mask
**/
/**
 // TX line
 // RX line
 // MISO
 // MOSI
 // SDA
 // SCL
**/
EXPORT int OSIF_io_set_ddr(int adapter_no, int ddr, int enabled)
{
    usb_dev_handle *handle;
    handle = get_adapter_handle(adapter_no);

    if ((ddr & (1<<4)) || (ddr & (1<<5)))
    {
        printf("will disable I2C\n");
        OSIF_disable_i2c(adapter_no);
    }

    if (usb_control_msg(handle, USB_CTRL_IN, 
        USBIO_SET_DDR,
        0, ddr, 0, enabled, 1000) >0)
    {
        fprintf(stderr, "OSIF_io_set_ddr: USB write error: %s\n", usb_strerror());
        return -1;
    }

    // store the values in case we use it later
    gpio_state[adapter_no].ddr = ddr;
    gpio_state[adapter_no].enabled = enabled;

#ifdef DEBUG_OUT
    printf( "OSIF_io_set_ddr: Set Data Direction Command Sent\n" );
#endif

	return 1;
}

/**
    GPIO Control function to set pin high or low
        (only works if ddr set to output)

    adapter: integer of the adapter scanned. 0 indexed.
    io: a bitwise OR of the pins to set high (1) or low (0)
        eg gpio1(TX) and gpio2(RX)  00000011 or 0x03 to set those high
**/
EXPORT int OSIF_io_set_out(int adapter_no, int io)
{
    usb_dev_handle *handle;
    handle = get_adapter_handle(adapter_no);

    // invert the io bits suitsble for OSIF inverted format
    int io_inv; io_inv = io^ 0xff;

    if (usb_control_msg(handle, USB_CTRL_IN, 
        USBIO_SET_OUT,
        0, io_inv
        , 0, 0,
        1000) >0)
    {
        fprintf(stderr, "w_io_USB error: %s\n", usb_strerror());
        return -1;
    }

    // store the gpio values
    gpio_state[adapter_no].output_state = io;
#ifdef DEBUG_OUT
    int j;
    for (j=0; j < 6; j++)
    {
        if (io & (1<<j))
            printf("gpio %d high\n", j);
    }
#endif

    return 1;
}

/**
    GPIO Control function to read the status of all gpio pins

    adapter: integer of the adapter scanned. 0 indexed.
    io: a single bitwise OR int representing all bits of the gpio pins
        eg gpio1(TX) and gpio2(RX) are high, returns 00000011 or 0x03
    enabled: a bitwise OR to enable the pin (1) for future writes. Can be used as a mask
**/
EXPORT int OSIF_io_get_in(int adapter_no)
{
    usb_dev_handle *handle;
    handle = get_adapter_handle(adapter_no);
    char data[10];
    int retval = usb_control_msg(handle, USB_CTRL_IN,
        USBIO_GET_IN,
        0, 0, data, 0,
        1000);

    if (retval > 0)
    {
        fprintf(stderr, "r_io_USB error: %s\n", usb_strerror());
        return -1;
    }

    return (int)data[0];
}

/**
    Shortcut functions to update one pin only
**/
EXPORT int OSIF_io_set_out1(int adapter_no, int gpio, int state)
{
    int newstate;
    if (state) //high
        newstate = gpio_state[adapter_no].output_state | (1<<gpio);
    else
        newstate = gpio_state[adapter_no].output_state & ~(1<<gpio);
    printf("newstate %d %d %d\n", newstate, gpio_state[adapter_no].output_state, 1<<gpio);
    
    return OSIF_io_set_out(adapter_no, newstate);
}

/**
    get the current pin states. THIS IS NOT suitable for asking the osif what pins
    are set when this lib loaded, OSIF doesn't know that. It is this lib that keeps
    tabs on what pins are being set/uset
**/
EXPORT int OSIF_io_get_current(int adapter_no)
{
    return gpio_state[adapter_no].output_state;
}
/**
    Disable the I2C port in case we want to use the pins as gpios,
    or if we want to reset the I2C module
**/
EXPORT int OSIF_disable_i2c(int adapter_no)
{
    usb_dev_handle *handle;
    handle = get_adapter_handle(adapter_no);

    // Powerup the SPI interface, disabling the I2C
    if (usb_control_msg(handle, USB_CTRL_IN,
        USBTINY_POWERUP,
        20, 1, 0, 0,
        500) >0) {
            return -1;
        }
    return 1;
}

/**
    Enable a previously disabled I2C module
**/
EXPORT int OSIF_enable_i2c(int adapter_no)
{
    usb_dev_handle *handle;
    handle = get_adapter_handle(adapter_no);

    // Power down the SPI Interface, reinitialising I2C
    if (usb_control_msg(handle, USB_CTRL_IN,
        USBTINY_POWERDOWN,
        0, 0, 0, 0,
        500) >0) {
            return -1;
        }
    return 1;
}

EXPORT int OSIF_set_bitrate(int adapter_no, int bitrate_hz)
{
    int twbr;
    int twps = 1;
    usb_dev_handle *handle;
    handle = get_adapter_handle(adapter_no);

    // Calculate the new bitrate based on the formula
    // br scl = cpu / (16 + 2(TWBR)) . 4 ^TWPS
    if (bitrate_hz < 26000)
    {
        twps = 4;
    }
    //int poww = pow(4, twps);
    twbr = (((12000000/bitrate_hz)/twps)-16)/2 ;

    printf("twbr: %d\n", twbr);
    if (usb_control_msg(handle, USB_CTRL_IN,
        USBI2C_SET_BITRATE, twbr
        , twps, 0, 0,
        1000) >0)
    {
        fprintf(stderr, "w_bitrate_USB error: %s\n", usb_strerror());
        return -1;
    }
    return 1;
}
EXPORT int OSIF_set_twbr(int adapter_no, int twbr, int twps)
{
    usb_dev_handle *handle;
    handle = get_adapter_handle(adapter_no);

    printf("twbr: %d\n", twbr);
    if (usb_control_msg(handle, USB_CTRL_IN,
        USBI2C_SET_BITRATE, twbr
                , twps, 0, 0,
                1000) >0)
    {
        fprintf(stderr, "w_bitrate_USB error: %s\n", usb_strerror());
        return -1;
    }
    return 1;
}

/**** Internal Utility functions ****/

/**
	Check the validity of the options
 **/
int check_params( int val )
{
	if (val > 127 || val <0 )
	{
		return -1;
	}
	return 1;
}

/**
	Get the handle of the selected adapter
**/
usb_dev_handle * get_adapter_handle(int adapter_no)
{
	if (adapter_no > adapter_count )
	{
		return NULL;
	}

	return adapters[adapter_no].adapter_handle;
}
