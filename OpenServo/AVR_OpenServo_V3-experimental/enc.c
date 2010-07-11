// enc.c
//
// OpenEncoder driver for OpenServo
//
// Copyright (C) 2009-2010  Darius Rad <alpha@area49.net>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include <stdint.h>
#include "enc.h"
#include "swi2c.h"
#include "registers.h"

#if ENCODER_ENABLED


/// Serial Interface Unit status bits
enum
{
    STATUS_OCF = (1<<5),  ///< Offset Comp Finish
    STATUS_COF = (1<<4),  ///< CORDIC Overflow
    STATUS_LIN = (1<<3),  ///< Lin Alarm
    STATUS_M_I = (1<<2),  ///< Mag Incr
    STATUS_M_D = (1<<1),  ///< Mag Decr
    STATUS_P = (1<<0),  ///< Even Parity

    /// Mask of bits to check for valid reading
    STATUS_VALID_MASK = (STATUS_OCF | STATUS_COF),

    /// Value of bits to check for valid reading
    STATUS_VALID_VALUE = STATUS_OCF,
};


struct serial_interface_unit_s
{
    uint16_t angle;
    uint8_t magnitude;
    uint8_t status;
};


/// Serial Interface Unit register read
///
/// \retval true Data is valid
/// \retval false Data is invalid
static bool _reg_read_serial_interface_unit(struct serial_interface_unit_s * siu)
{
    unsigned char data[4];

    swi2c_read(0x51, data, sizeof(data));

    siu->angle = (data[0] << 4) | ((data[1] & 0xc0) >> 4) | ((data[3] & 0xc0) >> 6);
    siu->magnitude = data[2];
    siu->status = data[1] & 0x3f;

    return (siu->status & STATUS_VALID_MASK) == STATUS_VALID_VALUE;
}


uint16_t enc_get_position_value(void)
{
    struct serial_interface_unit_s siu;
    unsigned char *raw = (void*)&siu;
    uint16_t ret;

    ret = _reg_read_serial_interface_unit(&siu) ? siu.angle : 0xFFFF;

    registers_write_byte(REG_ENCODER_RAW_0, *raw++);
    registers_write_byte(REG_ENCODER_RAW_1, *raw++);
    registers_write_byte(REG_ENCODER_RAW_2, *raw++);
    registers_write_byte(REG_ENCODER_RAW_3, *raw);

    return ret;
}

#endif
