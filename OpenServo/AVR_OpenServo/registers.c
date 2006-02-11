/*
    Copyright (c) 2006 Michael P. Thompson <mpthompson@gmail.com>

    Permission is hereby granted, free of charge, to any person 
    obtaining a copy of this software and associated documentation 
    files (the "Software"), to deal in the Software without 
    restriction, including without limitation the rights to use, copy, 
    modify, merge, publish, distribute, sublicense, and/or sell copies 
    of the Software, and to permit persons to whom the Software is 
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be 
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.

    $Id$
*/

// The following is needed until WINAVR supports the ATtinyX5 MCUs.
#undef __AVR_ATtiny2313__
#define __AVR_ATtiny45__

#include <inttypes.h>

#include "config.h"
#include "eeprom.h"
#include "estimator.h"
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

#if ESTIMATOR_ENABLED
    // Call the motion module to initialize the velocity estimator related 
    // default values. This is done so the estimator related parameters can
    // be kept in a single file.
    estimator_registers_defaults();
#endif

    // Call the motion module to initialize the motion related default values.
    // This is done so the motion related parameters can be kept in a single file.
    motion_registers_defaults();
}

