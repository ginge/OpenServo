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

#define DEVICE_NAME           1
#define DEVICE_SUBTYPE        2
#define VERSION_MAJOR         3
#define VERSION_MINOR         4
#define FLAGS_HI              5
#define FLAGS_LO              6
#define TIMER_HI              7
#define TIMER_LO              8
#define POSITION_HI           9
#define POSITION_LO           10
#define VELOCITY_HI           11
#define VELOCITY_LO           12
#define POWER_HI              13
#define POWER_LO              14
#define PWM_DIRA              15
#define PWM_DIRB              16
#define VOLTAGE_HI            17
#define VOLTAGE_LO            18
#define CURVE_BUFFER          19
#define TEMPERATURE_HI        20
#define TEMPERATURE_LO        21
#define SEEK_POSITION_HI      22
#define SEEK_POSITION_LO      23
#define SEEK_VELOCITY_HI      24
#define SEEK_VELOCITY_LO      25
#define CURVE_DELTA_HI        26
#define CURVE_DELTA_LO        27
#define CURVE_POSITION_HI     28
#define CURVE_POSITION_LO     29
#define CURVE_IN_VELOCITY_HI  30
#define CURVE_IN_VELOCITY_LO  31
#define CURVE_OUT_VELOCITY_HI 32
#define CURVE_OUT_VELOCITY_LO 33
#define BANK_SELECT           34
#define ALERT_STATUS          35
#define BACKEMF_HI            36
#define BACKEMF_LO            37
#define TWI_ADDRESS           38
#define PID_DEADBAND          39
#define PID_PGAIN_HI          40
#define PID_PGAIN_LO          41
#define PID_DGAIN_HI          42
#define PID_DGAIN_LO          43
#define PID_IGAIN_HI          44
#define PID_IGAIN_LO          45
#define PWM_FREQ_DIVIDER_HI   46
#define PWM_FREQ_DIVIDER_LO   47
#define MIN_SEEK_HI           48
#define MIN_SEEK_LO           49
#define MAX_SEEK_HI           50
#define MAX_SEEK_LO           51
#define REVERSE_SEEK          52
#define PWM_MAX               53
#define ALERT_CURR_MAX_LIMIT_HI 54
#define ALERT_CURR_MAX_LIMIT_LO 55
#define ALERT_VOLT_MAX_LIMIT_HI 56
#define ALERT_VOLT_MAX_LIMIT_LO 57
#define ALERT_VOLT_MIN_LIMIT_HI 57
#define ALERT_VOLT_MIN_LIMIT_LO 59
#define ALERT_TEMP_MAX_LIMIT_HI 60
#define ALERT_TEMP_MAX_LIMIT_LO 61
#define EMF_COLLAPSE_DELAY      62
#define EMF_CHARGE_TIME         63
#define CHECKED_TXN             64

#define END_TABLE               -1
#define REG                     -1
#define UNSUPPORTED_FEATURE     -1
