/*
   Copyright (c) 2006, Mike Thompson <mpthompson@gmail.com>
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
#include "motion.h"
#include "openservo.h"
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

    // Set device and software identification information.
    registers_write_byte(REG_DEVICE_TYPE, OPENSERVO_DEVICE_TYPE);
    registers_write_byte(REG_DEVICE_SUBTYPE, OPENSERVO_DEVICE_SUBTYPE);
    registers_write_byte(REG_VERSION_MAJOR, SOFTWARE_VERSION_MAJOR);
    registers_write_byte(REG_VERSION_MINOR, SOFTWARE_VERSION_MINOR);

    // Initialize read/write protected registers to defaults.
    registers_defaults();

    // Does the EEPROM appear to be erased?
    if (!eeprom_is_erased())
    {
        // No. Restore the register values.
        eeprom_restore_registers();
    }
}


void registers_defaults(void)
// Reset safe read/write registers to defaults.
{
    // Initialize read/write protected registers to defaults.

    // Default TWI address.
    registers_write_byte(REG_TWI_ADDRESS, 0x10);

    // Call the motion module to initialize the motion related default values.
    // This is done so the motion related parameters can be kept in a single file.
    motion_registers_defaults();
}

