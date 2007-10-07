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

#ifndef OSIF_DLL_H
#define OSIF_DLL_H     //stop recursion

/*
you must define one of these. They are in the Makefile
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
/**
  All function return <0 for error and >0 for success
    Function arguments:
    adapter: adapter number starting from 0
    servo:   address of device on the I2C bus
    addr:    start address of data read/write
    data:    pointer to unsigned char containing read bytes/bytes to write
    buflen:  length of data to read/write
**/


/** Initialise the OSIF USB interface **/
EXPORT int OSIF_init(void);

/** De-Initialise the OSIF USB interface **/
EXPORT int OSIF_deinit(void);

/** read from the I2C device at address addr **/
EXPORT int OSIF_read(int adapter, int servo, unsigned char addr, unsigned char * data, size_t buflen );

/** perform a I2C read without the register select write **/
EXPORT int OSIF_readonly(int adapter, int servo, unsigned char * data, size_t buflen );

/** Write data to the I2C device at addr **/
EXPORT int OSIF_write(int adapter, int servo, unsigned char addr, unsigned char * data, size_t buflen );

/** Scan the I2C bus for devices. Fills pointer *devices with 1d array of devices on bus and *dev_count with number found **/
EXPORT int OSIF_scan(int adapter, int *devices, int *dev_count );

/** returns true if a device is found at address servo **/
EXPORT bool OSIF_probe(int adapter, int servo );

/** Send a command to the OpenServo. See website for commands **/
EXPORT int OSIF_command(int adapter, int servo, unsigned char command);

/** Flash the application portion with new firmware. bootloader_addr is normally 0x7f **/
EXPORT int OSIF_reflash(int adapter, int servo, int bootloader_addr, char *filename);

/** Returns the friendly name of the connected adapter **/
EXPORT int OSIF_get_adapter_name(int adapter, char* name);

/** Returns number of connected OSIF adapters **/
EXPORT int OSIF_get_adapter_count(void);

#endif
