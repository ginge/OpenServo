/*
   Copyright (c) 2005, Mike Thompson <mpthompson@gmail.com>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

// The following is needed until WINAVR supports the ATtinyX5 MCUs.
#undef __AVR_ATtiny2313__
#define __AVR_ATtiny45__

#include <inttypes.h>

#include "eeprom.h"
#include "registers.h"

// Register values.
uint8_t registers[MAX_REGISTER + 1];

void registers_init(void)
// Function to initialize all registers.
{
	uint8_t i;

	// Initialize all registers to zero.
	for (i = 0; i < (MAX_REGISTER + 1); ++i)
	{
		// Initialize the word.
		registers_write_byte(i, 0x00);
	}

	// Initialize select read/only registers to disable 
	// PWM and disable register write on power up.
	registers_write_byte(PWM_ENABLE, 0x00);
	registers_write_byte(WRITE_ENABLE, 0x00);

	// Reset other read/only registers.  These values will
	// be populated under normal operation.
	registers_write_byte(POWER_HI, 0x00);
	registers_write_byte(POWER_LO, 0x00);
	registers_write_byte(POSITION_HI, 0x00);
	registers_write_byte(POSITION_LO, 0x00);
	registers_write_byte(PWM_CW, 0x00);
	registers_write_byte(PWM_CCW, 0x00);

	// Initialize safe read/write registers to defaults.
	registers_defaults();

	// Does the EEPROM appear to be erased?
	if (!eeprom_is_erased())
	{
		// No. Restore the register values.
		eeprom_restore_registers();
	}

	// Initialize read/write registers.  
	registers_write_byte(SEEK_HI, registers_read_byte(DEFAULT_SEEK_HI));
	registers_write_byte(SEEK_LO, registers_read_byte(DEFAULT_SEEK_LO));
}


void registers_defaults(void)
// Reset safe read/write registers to defaults.
{
	// Initialize safe read/write registers to defaults.
	registers_write_byte(TWI_ADDRESS, 0x10);
	registers_write_byte(PID_OFFSET, 0x00);
	registers_write_byte(PID_PGAIN_HI, 0x08);
	registers_write_byte(PID_PGAIN_LO, 0x00);
	registers_write_byte(PID_DGAIN_HI, 0x28);
	registers_write_byte(PID_DGAIN_LO, 0x00);
	registers_write_byte(PID_IGAIN_HI, 0x0E);
	registers_write_byte(PID_IGAIN_LO, 0x00);
	registers_write_byte(MIN_SEEK_HI, 0x00);
	registers_write_byte(MIN_SEEK_LO, 0x60);
	registers_write_byte(MAX_SEEK_HI, 0x03);
	registers_write_byte(MAX_SEEK_LO, 0xa0);
	registers_write_byte(DEFAULT_SEEK_HI, 0x02);
	registers_write_byte(DEFAULT_SEEK_LO, 0x00);
	registers_write_byte(REVERSE_SEEK, 0x00);
}

