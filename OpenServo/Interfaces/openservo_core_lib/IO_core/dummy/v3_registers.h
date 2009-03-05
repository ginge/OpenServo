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

//
// Defines servo control registers.
//

// TWI read/only status registers.  Writing
// values to these registers has no effect.


#define V3_REG_DEVICE_TYPE             0x00
#define V3_REG_DEVICE_SUBTYPE          0x01
#define V3_REG_VERSION_MAJOR           0x02
#define V3_REG_VERSION_MINOR           0x03
#define V3_REG_FLAGS_HI                0x04
#define V3_REG_FLAGS_LO                0x05
#define V3_REG_TIMER_HI                0x06
#define V3_REG_TIMER_LO                0x07

#define V3_REG_POSITION_HI             0x08
#define V3_REG_POSITION_LO             0x09
#define V3_REG_VELOCITY_HI             0x0A
#define V3_REG_VELOCITY_LO             0x0B
#define V3_REG_POWER_HI                0x0C
#define V3_REG_POWER_LO                0x0D
#define V3_REG_PWM_DIRA                0x0E
#define V3_REG_PWM_DIRB                0x0F

#define V3_REG_VOLTAGE_HI              0x10
#define V3_REG_VOLTAGE_LO              0x11
#define V3_REG_CURVE_BUFFER            0x12
#define V3_REG_TEMPERATURE_HI          0x13
#define V3_REG_TEMPERATURE_LO          0x14


// TWI read/write registers.  Writing these
// registers controls operation of the servo.

#define V3_REG_SEEK_POSITION_HI        0x20
#define V3_REG_SEEK_POSITION_LO        0x21
#define V3_REG_SEEK_VELOCITY_HI        0x22
#define V3_REG_SEEK_VELOCITY_LO        0x23

#define V3_REG_CURVE_DELTA_HI          0x24
#define V3_REG_CURVE_DELTA_LO          0x25
#define V3_REG_CURVE_POSITION_HI       0x26
#define V3_REG_CURVE_POSITION_LO       0x27
#define V3_REG_CURVE_IN_VELOCITY_HI    0x28
#define V3_REG_CURVE_IN_VELOCITY_LO    0x29
#define V3_REG_CURVE_OUT_VELOCITY_HI   0x2A
#define V3_REG_CURVE_OUT_VELOCITY_LO   0x2B
#define V3_REG_BANK_SELECT             0x2F

#define V3_ALERT_OVERCURR  0
#define V3_ALERT_OVERVOLT  1
#define V3_ALERT_UNDERVOLT 2
#define V3_ALERT_OVERTEMP  3

/*

 Banks

*/
#define MAX_BANKS 3

#define V3_BANK_0 0
#define V3_BANK_1 1
#define V3_BANK_2 2
#define V3_BANK_3 3  //not used
#define V3_BANK_4 4  //not used

//bank0
#define V3_ALERT_BANK                  V3_BANK_0
#define V3_INFORMATION_BANK            V3_BANK_0

#define V3_ALERT_STATUS                0x00
#define V3_REG_BACKEMF_HI              0x01
#define V3_REG_BACKEMF_LO              0x02

// Bank1
// TWI safe read/write registers.  These registers
// may only be written to when write enabled.
#define V3_POS_PID_BANK                V3_BANK_1
#define V3_ALERT_CONFIG_BANK           V3_BANK_1
#define V3_CONFIG_BANK                 V3_BANK_1

#define V3_REG_TWI_ADDRESS             0x00
#define V3_REG_PID_DEADBAND            0x01
#define V3_REG_PID_PGAIN_HI            0x02
#define V3_REG_PID_PGAIN_LO            0x03
#define V3_REG_PID_DGAIN_HI            0x04
#define V3_REG_PID_DGAIN_LO            0x05
#define V3_REG_PID_IGAIN_HI            0x06
#define V3_REG_PID_IGAIN_LO            0x07
#define V3_REG_MIN_SEEK_HI             0x08
#define V3_REG_MIN_SEEK_LO             0x09
#define V3_REG_MAX_SEEK_HI             0x0A
#define V3_REG_MAX_SEEK_LO             0x0B
#define V3_REG_REVERSE_SEEK            0x0C
#define V3_REG_PWM_FREQ_DIVIDER_HI     0x0D
#define V3_REG_PWM_FREQ_DIVIDER_LO     0x0E
#define V3_REG_PWM_MAX                 0x0F

#define V3_ALERT_CURR_MAX_LIMIT_HI     0x10
#define V3_ALERT_CURR_MAX_LIMIT_LO     0x11
#define V3_ALERT_VOLT_MAX_LIMIT_HI     0x12
#define V3_ALERT_VOLT_MAX_LIMIT_LO     0x13
#define V3_ALERT_VOLT_MIN_LIMIT_HI     0x14
#define V3_ALERT_VOLT_MIN_LIMIT_LO     0x15
#define V3_ALERT_TEMP_MAX_LIMIT_HI     0x16
#define V3_ALERT_TEMP_MAX_LIMIT_LO     0x17
// Delay in ms for the back emf spike collapse
#define V3_REG_EMF_COLLAPSE_DELAY      0x2A
// This is 41us * n us delay for back emf charging cap
#define V3_REG_EMF_CHARGE_TIME         0x2B


#define V3_CONFIG_SAVE_MIN             0x00
#define V3_CONFIG_SAVE_MAX             0x2B
#define V3_CONFIG_SAVE_COUNT (V3_CONFIG_SAVE_MAX - V3_CONFIG_SAVE_MIN + 1)

// Bank 2
// Put the redirects into bank 2
#define V3_REDIRECTED_BANK V3_BANK_2

#define V3_MIN_REDIRECT_REGISTER       0x00
#define V3_MAX_REDIRECT_REGISTER       0x0F
#define V3_MIN_REDIRECTED_REGISTER     0x10
#define V3_MAX_REDIRECTED_REGISTER     0x1F

// Define the number of redirect registers.
#define V3_REDIRECT_REGISTER_COUNT         (V3_MAX_REDIRECT_REGISTER - V3_MIN_REDIRECT_REGISTER + 1)

//
// Define the register ranges.
//
#define V3_MIN_READ_ONLY_REGISTER      0x00
#define V3_MAX_READ_ONLY_REGISTER      0x1F
#define V3_MIN_READ_WRITE_REGISTER     0x20
#define V3_MAX_READ_WRITE_REGISTER     0x37
#define V3_MIN_WRITE_PROTECT_REGISTER  0x38
#define V3_MAX_WRITE_PROTECT_REGISTER  0x3E
#define V3_MIN_UNUSED_REGISTER         0x3F
#define V3_MAX_UNUSED_REGISTER         0x3F
#define V3_MIN_BANK_REGISTER           0x40
#define V3_MAX_BANK_REGISTER           0x7F

// Define the total number of registers define.  This includes
// all registers except unused and redirected registers.
#define V3_REGISTER_COUNT              (V3_MIN_UNUSED_REGISTER + 16)

// Define the number of write protect registers.
#define V3_WRITE_PROTECT_REGISTER_COUNT    (V3_MAX_WRITE_PROTECT_REGISTER - V3_MIN_WRITE_PROTECT_REGISTER + 1)

// Define the number of banked registers.
#define V3_BANK_REGISTER_COUNT             (V3_MAX_BANK_REGISTER - V3_MIN_BANK_REGISTER + 1)

