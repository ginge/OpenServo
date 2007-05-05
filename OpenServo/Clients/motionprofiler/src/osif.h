/***************************************************************************
 *   Copyright (C) 2007 by Barry Carter   *
 *   barry.carter@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef OSIF_H
#define OSIF_H

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <stdio.h>
/**
OSIF hardware interface class

	@author Barry Carter <barry.carter@gmail.com>
*/
class OSIF{
public:
    OSIF();

    ~OSIF();


	/*Typedef the functions*/
	typedef int  (*OSIF_initfunc     )();
	typedef int  (*OSIF_deinitfunc   )();
	typedef int  (*OSIF_writefunc    )(int adapter, int servo, unsigned char addr, unsigned char *data, size_t len);
	typedef int  (*OSIF_readfunc     )(int adapter, int servo, unsigned char addr, unsigned char *data, size_t len);
	typedef int  (*OSIF_readonlyfunc )(int adapter, int servo, unsigned char *data, size_t len);
	typedef int  (*OSIF_reflashfunc  )(int adapter, int servo, int bootloader_addr, char *filename);
	typedef int  (*OSIF_commandfunc  )(int adapter, int servo, unsigned char command);
	typedef int  (*OSIF_scanfunc     )(int adapter, int devices[], int *dev_count);
	typedef bool (*OSIF_probefunc    )(int adapter, int servo);
	typedef int  (*OSIF_get_adapter_namefunc  )(int adapter, char *name);
	typedef int  (*OSIF_get_adapter_countfunc )(void);

		
	/*A pointer to a function*/
	OSIF_deinitfunc OSIF_deinit;
	OSIF_initfunc OSIF_init;
	OSIF_writefunc OSIF_write;
	OSIF_readfunc OSIF_read;
	OSIF_readonlyfunc OSIF_readonly;
	OSIF_reflashfunc OSIF_reflash;
	OSIF_scanfunc OSIF_scan;
	OSIF_probefunc OSIF_probe;
	OSIF_commandfunc OSIF_command;
	OSIF_get_adapter_namefunc OSIF_get_adapter_name;
	OSIF_get_adapter_countfunc OSIF_get_adapter_count;
	
	int adapters[128];
	int adapterCount;

	int devices[128];
	int devCount;

	int OpenServo[128];
	int OpenServoCount;

	int genericDevices[128];
	int genericDeviceCount;

	int servo;
	int adapter;

	int scanDevices(int adapterScan);
	int scanBus();
	int writeData( int adapter, int servo, int addr, char *val, size_t len );
	int readData(int adapter, int servo, int addr, unsigned char *buf, size_t len);
	int deinit(void);

	char *OSIFgetLastError(void) { return lastError; }
	bool isOSIFinitialised(void) { return OSIFinit; }
private:
#ifdef WIN32
	HINSTANCE hdll; //Windows handle
#else
	void * libhandle; // handle to the shared lib when opened
#endif	
	void logPrint( char *logData);
	char lastError[65535];
	bool OSIFinit;
};

// From AVR_Openservo/register.h 13/03/07

// values to these registers has no effect.

#define REG_DEVICE_TYPE             0x00
#define REG_DEVICE_SUBTYPE          0x01
#define REG_VERSION_MAJOR           0x02
#define REG_VERSION_MINOR           0x03
#define REG_FLAGS_HI                0x04
#define REG_FLAGS_LO                0x05
#define REG_TIMER_HI                0x06
#define REG_TIMER_LO                0x07

#define REG_POSITION_HI             0x08
#define REG_POSITION_LO             0x09
#define REG_VELOCITY_HI             0x0A
#define REG_VELOCITY_LO             0x0B
#define REG_POWER_HI                0x0C
#define REG_POWER_LO                0x0D
#define REG_PWM_DIRA                0x0E
#define REG_PWM_DIRB                0x0F

// TWI read/write registers.  Writing these
// registers controls operation of the servo.

#define REG_SEEK_POSITION_HI        0x10
#define REG_SEEK_POSITION_LO        0x11
#define REG_SEEK_VELOCITY_HI        0x12
#define REG_SEEK_VELOCITY_LO        0x13
#define REG_VOLTAGE_HI              0x14
#define REG_VOLTAGE_LO              0x15
#define REG_CURVE_RESERVED          0x16
#define REG_CURVE_BUFFER            0x17

#define REG_CURVE_DELTA_HI          0x18
#define REG_CURVE_DELTA_LO          0x19
#define REG_CURVE_POSITION_HI       0x1A
#define REG_CURVE_POSITION_LO       0x1B
#define REG_CURVE_IN_VELOCITY_HI    0x1C
#define REG_CURVE_IN_VELOCITY_LO    0x1D
#define REG_CURVE_OUT_VELOCITY_HI   0x1E
#define REG_CURVE_OUT_VELOCITY_LO   0x1F

// TWI safe read/write registers.  These registers
// may only be written to when write enabled.

#define REG_TWI_ADDRESS             0x20
#define REG_PID_OFFSET              0x21
#define REG_PID_PGAIN_HI            0x22
#define REG_PID_PGAIN_LO            0x23
#define REG_PID_DGAIN_HI            0x24
#define REG_PID_DGAIN_LO            0x25
#define REG_PID_IGAIN_HI            0x26
#define REG_PID_IGAIN_LO            0x27

#define REG_MIN_SEEK_HI             0x28
#define REG_MIN_SEEK_LO             0x29
#define REG_MAX_SEEK_HI             0x2A
#define REG_MAX_SEEK_LO             0x2B
#define REG_REVERSE_SEEK            0x2C
#define REG_RESERVED_2D             0x2D
#define REG_RESERVED_2E             0x2E
#define REG_RESERVED_2F             0x2F

//from twi.h
#define TWI_CMD_RESET                   0x80        // Reset command
#define TWI_CMD_CHECKED_TXN             0x81        // Read/Write registers with simple checksum
#define TWI_CMD_PWM_ENABLE              0x82        // Enable PWM to motors
#define TWI_CMD_PWM_DISABLE             0x83        // Disable PWM to servo motors
#define TWI_CMD_WRITE_ENABLE            0x84        // Enable write of safe read/write registers
#define TWI_CMD_WRITE_DISABLE           0x85        // Disable write of safe read/write registers
#define TWI_CMD_REGISTERS_SAVE          0x86        // Save safe read/write registers fo EEPROM
#define TWI_CMD_REGISTERS_RESTORE       0x87        // Restore safe read/write registers from EEPROM
#define TWI_CMD_REGISTERS_DEFAULT       0x88        // Restore safe read/write registers to defaults
#define TWI_CMD_EEPROM_ERASE            0x89        // Erase the EEPROM.
#define TWI_CMD_VOLTAGE_READ            0x90        // Starts a ADC on the supply voltage channel
#define TWI_CMD_CURVE_MOTION_ENABLE     0x91        // Enable curve motion processing.
#define TWI_CMD_CURVE_MOTION_DISABLE    0x92        // Disable curve motion processing.
#define TWI_CMD_CURVE_MOTION_RESET      0x93        // Reset the curve motion buffer.
#define TWI_CMD_CURVE_MOTION_APPEND     0x94        // Append curve motion data.

#endif
