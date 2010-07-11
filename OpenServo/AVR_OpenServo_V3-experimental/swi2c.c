// swi2c.c
//
// Software I2C for OpenServo
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


#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include "swi2c.h"


// SCL is D5
#define _SCL_PORTx  PORTD
#define _SCL_DDRx  DDRD
#define _SCL_PINx  PIND
#define _SCL_BIT  (1<<5)

// SDA is C2
#define _SDA_PORTx  PORTC
#define _SDA_DDRx  DDRC
#define _SDA_PINx  PINC
#define _SDA_BIT  (1<<2)


static bool _SCL_read(void)
{
    return _SCL_PINx & _SCL_BIT ? true : false;
}


static void _SCL_assert(void)
{
    // drive SCL low
    _SCL_PORTx &= ~_SCL_BIT;
    _SCL_DDRx |= _SCL_BIT;
}


static void _SCL_deassert(void)
{
    // set SCL input
    _SCL_DDRx &= ~_SCL_BIT;
    _SCL_PORTx |= _SCL_BIT;

    // wait for SCL to be released by another device (clock
    // stretching)
    while (!_SCL_read())
        ;  // nothing
}


static bool _SDA_read(void)
{
    return _SDA_PINx & _SDA_BIT ? true : false;
}


static void _SDA_assert(void)
{
    // drive SDA low
    _SDA_PORTx &= ~_SDA_BIT;
    _SDA_DDRx |= _SDA_BIT;
}


static void _SDA_deassert(void)
{
    // set SDA input
    _SDA_DDRx &= ~_SDA_BIT;
    _SDA_PORTx |= _SDA_BIT;
}


static void _start(void)
{
    _SDA_assert();
}


static void _stop(void)
{
    _SDA_assert();
    _SDA_deassert();
}


static void _sendbit(bool data)
{
    _SCL_assert();

    if (data)
    {
        _SDA_deassert();
    }
    else
    {
        _SDA_assert();
    }

    _SCL_deassert();
}


static bool _recvbit(void)
{
    _SCL_assert();
    _SDA_deassert();
    _SCL_deassert();

    return _SDA_read();
}


static bool _sendbyte(uint8_t data)
{
    int i;

    for (i = 0; i < 8; i++)
    {
        _sendbit(data & 0x80 ? true : false);
        data <<= 1;
    }

    return !_recvbit();
}


static uint8_t _recvbyte(bool ack)
{
    unsigned char data = 0;
    int i;

    for (i = 0; i < 8; i++)
    {
        data <<= 1;
        data |= _recvbit() ? 1 : 0;
    }

    _sendbit(!ack);

    return data;
}


void swi2c_init(void)
{
    // set SCL input
    _SCL_DDRx &= ~_SCL_BIT;
    _SCL_PORTx |= _SCL_BIT;

    // set SDA input
    _SDA_DDRx &= ~_SDA_BIT;
    _SDA_PORTx |= _SDA_BIT;
}


uint8_t swi2c_read(uint8_t addr, void * data, int length)
{
    //assert(addr & 0x1);

    unsigned char * ptr = data;
    int count = 0;

    _start();

    if (_sendbyte(addr))
    {
        while (count < length)
        {
            *ptr++ = _recvbyte(true);
            count++;
        }
    }

    _stop();

    return count;
}


uint8_t swi2c_write(uint8_t addr, const void * data, int length)
{
    //assert(addr & 0x1 == 0);

    const unsigned char * ptr = data;
    int count = 0;

    _start();

    if (_sendbyte(addr))
    {
        while (count < length)
        {
            if (_sendbyte(*ptr))
            {
                ptr++;
                count++;
            }
            else
            {
                break;
            }
        }
    }

    _stop();

    return count;
}
