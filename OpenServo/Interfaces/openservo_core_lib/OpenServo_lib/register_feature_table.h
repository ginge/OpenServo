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

#ifndef REGISTER_FEATURES_H
#define REGISTER_FEATURES_H     //stop recursion

#include "registers_features.h"
#include "registers_v2.h"
#include "registers_v3.h"

#define R 0
#define RW 1
#define WIDTH_8 1
#define WIDTH_16 2

typedef struct register_types_t
{
    long long   feature_name;
    int    register_location;
    double feature_version_min;
    double feature_version_max;
    int    bank_location;      // -1 register 0 bank 0 etc
    int    direction;          // R (read only) RW (read/write)
    int    width;              // 8 or 16 bit
} register_types;

static register_types register_features[] =
{
    // *1 = minimum version the feature was introduced (double notation Major.Minor)
    // *2 = last version this feature was seen in
    // *3 = Register or bank? REG=register or number = bank
    // Nice Feature name       OpenServo feature name          *1   *2   *3       r/w   8 or 16bit
    { DEVICE_NAME,             REG_DEVICE_TYPE,                0.0, 2.9, REG,       R,  WIDTH_8  },
    { DEVICE_NAME,             V3_REG_DEVICE_TYPE,             3.0, 3.9, REG,       R,  WIDTH_8  },
    { DEVICE_SUBTYPE,          REG_DEVICE_SUBTYPE,             0.0, 2.9, REG,       R,  WIDTH_8  },
    { DEVICE_SUBTYPE,          V3_REG_DEVICE_SUBTYPE,          3.0, 3.9, REG,       R,  WIDTH_8  },
    { VERSION_MAJOR,           REG_VERSION_MAJOR,              0.0, 2.9, REG,       R,  WIDTH_8  },
    { VERSION_MAJOR,           V3_REG_VERSION_MAJOR,           3.0, 3.9, REG,       R,  WIDTH_8  },
    { VERSION_MINOR,           REG_VERSION_MINOR,              0.0, 2.9, REG,       R,  WIDTH_8  },
    { VERSION_MINOR,           REG_VERSION_MINOR,              3.0, 3.9, REG,       R,  WIDTH_8  },
    { FLAGS_HI,                REG_FLAGS_HI,                   0.0, 2.9, REG,       R,  WIDTH_16 },
    { FLAGS_HI,                V3_REG_FLAGS_HI,                3.0, 3.9, REG,       R,  WIDTH_16 },
    { TIMER_HI,                REG_TIMER_HI,                   0.0, 2.9, REG,       R,  WIDTH_16 },
    { TIMER_HI,                V3_REG_TIMER_HI,                3.0, 3.9, REG,       R,  WIDTH_16 },
    { POSITION_HI,             REG_POSITION_HI,                0.0, 2.9, REG,       R,  WIDTH_16 },
    { POSITION_HI,             V3_REG_POSITION_HI,             3.0, 3.9, REG,       R,  WIDTH_16 },
    { VELOCITY_HI,             REG_VELOCITY_HI,                0.0, 2.9, REG,       R,  WIDTH_16 },
    { VELOCITY_HI,             V3_REG_VELOCITY_HI,             3.0, 3.9, REG,       R,  WIDTH_16 },
    { POWER_HI,                REG_POWER_HI,                   0.0, 2.9, REG,       R,  WIDTH_16 },
    { POWER_HI,                V3_REG_POWER_HI,                3.0, 3.9, REG,       R,  WIDTH_16 },
    { PWM_DIRA,                REG_PWM_DIRA,                   0.0, 2.9, REG,       R,  WIDTH_8  },
    { PWM_DIRA,                V3_REG_PWM_DIRA,                3.0, 3.9, REG,       R,  WIDTH_8  },
    { PWM_DIRB,                REG_PWM_DIRB,                   0.0, 2.9, REG,       R,  WIDTH_8  },
    { PWM_DIRB,                V3_REG_PWM_DIRB,                3.0, 3.9, REG,       R,  WIDTH_8  },
    { VOLTAGE_HI,              REG_VOLTAGE_HI,                 0.0, 2.9, REG,       R,  WIDTH_16 },
    { VOLTAGE_HI,              V3_REG_VOLTAGE_HI,              3.0, 3.9, REG,       R,  WIDTH_16 },
    { CURVE_BUFFER,            REG_CURVE_BUFFER,               0.0, 2.9, REG,       RW, WIDTH_8  },
    { CURVE_BUFFER,            V3_REG_CURVE_BUFFER,            3.0, 3.9, REG,       RW, WIDTH_8  },
    { TEMPERATURE_HI,          V3_REG_TEMPERATURE_HI,          3.0, 3.9, REG,       R,  WIDTH_16 },
    { SEEK_POSITION_HI,        REG_SEEK_POSITION_HI,           0.0, 2.9, REG,       RW, WIDTH_16 },
    { SEEK_POSITION_HI,        V3_REG_SEEK_POSITION_HI,        3.0, 3.9, REG,       RW, WIDTH_16 },
    { SEEK_VELOCITY_HI,        REG_SEEK_VELOCITY_HI,           0.0, 2.9, REG,       RW, WIDTH_16 },
    { SEEK_VELOCITY_HI,        V3_REG_SEEK_VELOCITY_HI,        3.0, 3.9, REG,       RW, WIDTH_16 },
    { CURVE_DELTA_HI,          REG_CURVE_DELTA_HI,             0.0, 2.9, REG,       RW, WIDTH_16 },
    { CURVE_DELTA_HI,          V3_REG_CURVE_DELTA_HI,          3.0, 3.9, REG,       RW, WIDTH_16 },
    { CURVE_POSITION_HI,       REG_CURVE_POSITION_HI,          0.0, 2.9, REG,       RW, WIDTH_16 },
    { CURVE_POSITION_HI,       V3_REG_CURVE_POSITION_HI,       3.0, 3.9, REG,       RW, WIDTH_16 },
    { CURVE_IN_VELOCITY_HI,    REG_CURVE_IN_VELOCITY_HI,       0.0, 2.9, REG,       RW, WIDTH_16 },
    { CURVE_IN_VELOCITY_HI,    V3_REG_CURVE_IN_VELOCITY_HI,    3.0, 3.9, REG,       RW, WIDTH_16 },
    { CURVE_OUT_VELOCITY_HI,   REG_CURVE_OUT_VELOCITY_HI,      0.0, 2.9, REG,       RW, WIDTH_16 },
    { CURVE_OUT_VELOCITY_HI,   V3_REG_CURVE_OUT_VELOCITY_HI,   3.0, 3.9, REG,       RW, WIDTH_16 },
    { BANK_SELECT,             V3_REG_BANK_SELECT,             3.0, 3.9, REG,       RW, WIDTH_8  },
    { ALERT_STATUS,            V3_ALERT_STATUS,                3.0, 3.9, V3_BANK_0, R,  WIDTH_8  },
    { BACKEMF_HI,              V3_REG_BACKEMF_HI,              3.0, 3.9, V3_BANK_0, R,  WIDTH_16 },
    { TWI_ADDRESS,             REG_TWI_ADDRESS,                0.0, 2.9, V3_BANK_1, RW, WIDTH_8  },
    { TWI_ADDRESS,             V3_REG_TWI_ADDRESS,             3.0, 3.9, V3_BANK_1, RW, WIDTH_8  },
    { PID_DEADBAND,            REG_PID_OFFSET,                 0.0, 2.9, V3_BANK_1, RW, WIDTH_8  },
    { PID_DEADBAND,            V3_REG_PID_DEADBAND,            3.0, 3.9, V3_BANK_1, RW, WIDTH_8  },
    { PID_PGAIN_HI,            REG_PID_PGAIN_HI,               0.0, 2.9, REG,       RW, WIDTH_16 },
    { PID_PGAIN_HI,            V3_REG_PID_PGAIN_HI,            3.0, 3.9, V3_BANK_1, RW, WIDTH_16 },
    { PID_DGAIN_HI,            REG_PID_DGAIN_HI,               0.0, 2.9, REG,       RW, WIDTH_16 },
    { PID_DGAIN_HI,            V3_REG_PID_DGAIN_HI,            3.0, 3.9, V3_BANK_1, RW, WIDTH_16 },
    { PID_IGAIN_HI,            REG_PID_IGAIN_HI,               0.0, 2.9, REG,       RW, WIDTH_16 },
    { PID_IGAIN_HI,            V3_REG_PID_IGAIN_HI,            3.0, 3.9, V3_BANK_1, RW, WIDTH_16 },
    { PWM_FREQ_DIVIDER_HI,     REG_PWM_FREQ_DIVIDER_HI,        0.0, 2.9, REG,       RW, WIDTH_16 },
    { PWM_FREQ_DIVIDER_HI,     V3_REG_PWM_FREQ_DIVIDER_HI,     3.0, 3.9, V3_BANK_1, RW, WIDTH_16 },
    { MIN_SEEK_HI,             REG_MIN_SEEK_HI,                0.0, 2.9, REG,       RW, WIDTH_16 },
    { MIN_SEEK_HI,             V3_REG_MIN_SEEK_HI,             3.0, 3.9, V3_BANK_1, RW, WIDTH_16 },
    { MAX_SEEK_HI,             REG_MAX_SEEK_HI,                0.0, 2.9, REG,       RW, WIDTH_16 },
    { MAX_SEEK_HI,             V3_REG_MAX_SEEK_HI,             3.0, 3.9, V3_BANK_1, RW, WIDTH_16 },
    { REVERSE_SEEK,            REG_REVERSE_SEEK,               0.0, 2.9, REG,       RW, WIDTH_8  },
    { REVERSE_SEEK,            V3_REG_REVERSE_SEEK,            3.0, 3.9, V3_BANK_1, RW, WIDTH_8  },
    { PWM_MAX,                 V3_REG_PWM_MAX,                 3.0, 3.9, V3_BANK_1, RW, WIDTH_8  },
    { ALERT_CURR_MAX_LIMIT_HI, V3_ALERT_CURR_MAX_LIMIT_HI,     3.0, 3.9, V3_BANK_1, RW, WIDTH_16 },
    { ALERT_CURR_MAX_LIMIT_HI, V3_ALERT_CURR_MAX_LIMIT_HI,     3.0, 3.9, V3_BANK_1, RW, WIDTH_16 },
    { ALERT_VOLT_MAX_LIMIT_HI, V3_ALERT_VOLT_MAX_LIMIT_HI,     3.0, 3.9, V3_BANK_1, RW, WIDTH_16 },
    { ALERT_VOLT_MIN_LIMIT_HI, V3_ALERT_VOLT_MIN_LIMIT_HI,     3.0, 3.9, V3_BANK_1, RW, WIDTH_16 },
    { ALERT_TEMP_MAX_LIMIT_HI, V3_ALERT_TEMP_MAX_LIMIT_HI,     3.0, 3.9, V3_BANK_1, RW, WIDTH_16 },
    { EMF_COLLAPSE_DELAY,      V3_REG_EMF_COLLAPSE_DELAY,      3.0, 3.9, V3_BANK_1, RW, WIDTH_8  },
    { EMF_CHARGE_TIME,         V3_REG_EMF_CHARGE_TIME,         3.0, 3.9, V3_BANK_1, RW, WIDTH_8  },
    { END_TABLE,               END_TABLE,                      0.0, 0.0, 0, 0, 0                 }
};


#endif //REGISTER_FEATURES
