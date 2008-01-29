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

#include <inttypes.h>
#include <string.h>

#include "openservo.h"
#include "config.h"
#include "eeprom.h"
#include "estimator.h"
#include "ipd.h"
#include "pid.h"
#include "step.h"
#include "regulator.h"
#include "registers.h"

// Register values.
uint8_t registers[REGISTER_COUNT];

void registers_init(void)
// Function to initialize all registers.
{
    // Initialize all registers to zero.
    memset(&registers[0], 0, REGISTER_COUNT);

    // Set device and software identification information.
    registers_write_byte(REG_DEVICE_TYPE, OPENSERVO_DEVICE_TYPE);
    registers_write_byte(REG_DEVICE_SUBTYPE, OPENSERVO_DEVICE_SUBTYPE);
    registers_write_byte(REG_VERSION_MAJOR, SOFTWARE_VERSION_MAJOR);
    registers_write_byte(REG_VERSION_MINOR, SOFTWARE_VERSION_MINOR);

    // Restore the read/write protected registers from EEPROM.  If the
    // EEPROM fails checksum this function will return zero and the
    // read/write protected registers should be initialized to defaults.
    if (!eeprom_restore_registers())
    {
        // Reset read/write protected registers to zero.
        memset(&registers[MIN_WRITE_PROTECT_REGISTER], WRITE_PROTECT_REGISTER_COUNT + REDIRECT_REGISTER_COUNT, REGISTER_COUNT);

        // Initialize read/write protected registers to defaults.
        registers_defaults();
    }
}


void registers_defaults(void)
// Reset safe read/write registers to defaults.
{
    // Initialize read/write protected registers to defaults.

    // Default TWI address.
    registers_write_byte(REG_TWI_ADDRESS, REG_DEFAULT_TWI_ADDR);

    // Call the Step module to initialize the Stepping related default values.
    step_registers_defaults();

#if ESTIMATOR_ENABLED
    // Call the motion module to initialize the velocity estimator related 
    // default values. This is done so the estimator related parameters can
    // be kept in a single file.
    estimator_registers_defaults();
#endif

#if REGULATOR_MOTION_ENABLED
    // Call the regulator module to initialize the regulator related default values.
    regulator_registers_defaults();
#endif

#if PID_MOTION_ENABLED
    // Call the PID module to initialize the PID related default values.
    pid_registers_defaults();
#endif

#if IPD_MOTION_ENABLED
    // Call the IPD module to initialize the IPD related default values.
    ipd_registers_defaults();
#endif
}


uint16_t registers_read_word(uint8_t address_hi, uint8_t address_lo)
// Read a 16-bit word from the registers.
// Interrupts are disabled during the read.
{
    uint8_t sreg;
    uint16_t value;


    // Clear interrupts.
    asm volatile ("in %0,__SREG__\n\tcli\n\t" : "=&r" (sreg));

    // Read the registers.
    value = (registers[address_hi] << 8) | registers[address_lo];

    // Restore status.
    asm volatile ("out __SREG__,%0\n\t" : : "r" (sreg));

    return value;
}


void registers_write_word(uint8_t address_hi, uint8_t address_lo, uint16_t value)
// Write a 16-bit word to the registers.
// Interrupts are disabled during the write.
{
    uint8_t sreg;

    // Clear interrupts.
    asm volatile ("in %0,__SREG__\n\tcli\n\t" : "=&r" (sreg));

    // Write the registers.
    registers[address_hi] = value >> 8;
    registers[address_lo] = value;

    // Restore status.
    asm volatile ("out __SREG__,%0\n\t" : : "r" (sreg));
}


