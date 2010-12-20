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

/* stop recursion */
#ifndef OSIF_DLL_H
#define OSIF_DLL_H


/*
If you intend to rebuild this dll, you must define one of these. They are in the Makefile
for windows define WIN and BUILD_DLL to build dll
for windows to link against .a define WIN
for linux and Mac BUILD_STATIC_LINKER to generate shared .so library run ldconfig after copying to /lib/
  #define WIN
  #define BUILD_DLL
  #define BUILD_STATIC_LINKER
*/

#ifdef BUILD_DLL
/* DLL export */
#define EXPORT __declspec(dllexport)
#endif

/* EXE import */
#ifdef BUILD_STATIC_LINKER
#define EXPORT __declspec(dllimport)
#endif

#ifdef WIN
#define usleep(t) Sleep((t) / 1000)
#include "win_usb.h"
#include <windows.h>
#else
#define EXPORT
#include <usb.h>
#endif

#include "OSIFlib.h"

/* The current firmware version */
#define LIB_VERSION_MAJOR 0
#define LIB_VERSION_MINOR 9

/**
  All function return <0 for error and >0 for success
    Function arguments:
    adapter:  adapter number starting from 0
    i2c_addr: address of device on the I2C bus
    addr:     start address of data read/write
    data:     pointer to unsigned char containing read bytes/bytes to write
    buflen:   length of data to read/write
	stop_off: STOP_ON will send a stop bit after the transaction, STOP_OFF will now.

**/


/** Initialise the OSIF USB interface **/
EXPORT int OSIF_init(void);

/** De-Initialise the OSIF USB interface **/
EXPORT int OSIF_deinit(void);

/** Return the OSIF library version as integer for compatibility checks **/
EXPORT int OSIF_get_libversion(unsigned char * data);

/** read from the I2C device at address addr does an initial register selection **/
EXPORT int OSIF_read(int adapter_no, int i2c_addr, unsigned char addr, unsigned char * data, int buflen );

/** read from the I2C device at address addr optional stop bit**/
EXPORT int OSIF_read_data(int adapter_no, int i2c_addr, unsigned char addr, unsigned char * data, int buflen, int issue_stop );

/** perform a I2C read without the register select write **/
EXPORT int OSIF_readonly(int adapter_no, int i2c_addr, unsigned char * data, int buflen, int issue_stop);

/** Write data to the I2C device at addr **/
EXPORT int OSIF_write(int adapter_no, int i2c_addr, unsigned char addr, unsigned char * data, int buflen);

/** Write data to the I2C device at addr with an option to specify the stop bit status STOP_ON and STOP_OFF **/
EXPORT int OSIF_write_data(int adapter_no, int i2c_addr, unsigned char addr, unsigned char * data, int buflen, int issue_stop );

/** Write data to the I2C device at addr **/
EXPORT int OSIF_writeonly(int adapter_no, int i2c_addr, unsigned char * data, int buflen, int issue_stop );

/** Scan the I2C bus for devices. Fills pointer *devices with 1d array of devices on bus and *dev_count with number found **/
EXPORT int OSIF_scan(int adapter_no, int *devices, int *dev_count );

/** returns true if a device is found at address i2c_addr **/
EXPORT bool OSIF_probe(int adapter_no, int i2c_addr );

/** Send a command to the device. **/
EXPORT int OSIF_command(int adapter_no, int i2c_addr, unsigned char command);

/** Flash the application portion with new firmware. bootloader_addr is normally 0x7f **/
EXPORT int OSIF_reflash(int adapter_no, int i2c_addr, int bootloader_addr, char *filename);

/** Returns the friendly name of the connected adapter **/
EXPORT int OSIF_get_adapter_name(int adapter_no, char* name);

/** Returns number of connected OSIF adapters **/
EXPORT int OSIF_get_adapter_count(void);

/** set the bitrate (in HZ) **/
EXPORT int OSIF_set_bitrate(int adapter_no, int bitrate);

/** Disable the I2C module on the selected OSIF adapter **/
EXPORT int OSIF_disable_i2c(int adapter_no);

/** Enable the I2C module on the selected OSIF adapter **/
EXPORT int OSIF_enable_i2c(int adapter_no);

/** Set a Data Direction (DDR) on a GPIO pin **/
EXPORT int OSIF_io_set_ddr(int adapter_no, unsigned char ddr, unsigned char enabled);

/** GPIO Control function to set pin high or low **/
EXPORT int OSIF_io_set_out(int adapter_no, int io);

/** GPIO Control function to read the status of all gpio pins **/
EXPORT int OSIF_io_get_in(int adapter_no);

/** Shortcut functions to update one pin only **/
EXPORT int OSIF_io_set_out1(int adapter_no, int gpio, int state);

/** get the current pin states. THIS IS NOT suitable for asking the osif what pins
    are set when this lib loaded, OSIF doesn't know that. **/
EXPORT int OSIF_io_get_current(int adapter_no);

EXPORT int OSIF_set_pwm(int adapter_no, int pwm);

#endif
