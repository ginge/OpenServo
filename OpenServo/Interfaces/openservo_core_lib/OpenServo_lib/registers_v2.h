/***************************************************************************
 *   Copyright (C) 2008 by Barry Carter,,,   *
 *   barry.carter@robotfuzz.com   *
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


#define REG_PWM_FREQ_DIVIDER_HI     0x28
#define REG_PWM_FREQ_DIVIDER_LO     0x29

#define REG_MIN_SEEK_HI             0x2A
#define REG_MIN_SEEK_LO             0x2B
#define REG_MAX_SEEK_HI             0x2C
#define REG_MAX_SEEK_LO             0x2D
#define REG_REVERSE_SEEK            0x2E
#define REG_RESERVED_2D             0x2F

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


#define FLAGS_LO_GENERALCALL_START   0x05
#define FLAGS_LO_GENERALCALL_WAIT    0x04
#define FLAGS_LO_GENERALCALL_ENABLED 0x03
#define FLAGS_LO_MOTION_ENABLED      0x02
#define FLAGS_LO_WRITE_ENABLED       0x01
#define FLAGS_LO_PWM_ENABLED         0x00
