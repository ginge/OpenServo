// swi2c.h
//
// Software I2C for OpenServo, header file
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


#ifndef __SWI2C_H__
#define __SWI2C_H__


#include <stdbool.h>
#include <stdint.h>


void swi2c_init(void);

uint8_t swi2c_read(uint8_t addr, void * data, int length);

uint8_t swi2c_write(uint8_t addr, const void * data, int length);


#endif
