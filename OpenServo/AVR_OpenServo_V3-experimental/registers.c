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
#include "pid.h"
#include "pwm.h"
#include "step.h"
#include "registers.h"
#include "banks.h"

// Register values.
uint8_t registers[REGISTER_COUNT];

#if ROLLING_SUBTYPE
uint8_t subtype_id[] = {'O', 'p', 'e', 'n', 'S', 'e', 'r', 'v', 'o'};
uint8_t subtype_index;
#endif



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

#if ROLLING_SUBTYPE
    subtype_index = 0;
#endif

    banks_init();

}


void registers_defaults(void)
// Reset safe read/write registers to defaults.
{
    // Initialize read/write protected registers to defaults.

#if PWM_ENABLED
    // Call the PWM module to initialize the PWM related default values.
    pwm_registers_defaults();
#endif
#if STEP_ENABLED
    step_registers_defaults();
#endif


    // Call the PID module to initialize the PID related default values.
    pid_registers_defaults();

    banks_defaults();
}


uint16_t registers_read_word(uint8_t address_hi)
// Read a 16-bit word from the registers.
// Interrupts are disabled during the read.
{
    uint8_t sreg;
    uint16_t value;


    // Clear interrupts.
    asm volatile ("in %0,__SREG__\n\tcli\n\t" : "=&r" (sreg));

    // Read the registers.
    value = (registers[address_hi] << 8) | registers[address_hi+1];

    // Restore status.
    asm volatile ("out __SREG__,%0\n\t" : : "r" (sreg));

    return value;
}


void registers_write_word(uint8_t address_hi, uint16_t value)
// Write a 16-bit word to the registers.
// Interrupts are disabled during the write.
{
    uint8_t sreg;

    // Clear interrupts.
    asm volatile ("in %0,__SREG__\n\tcli\n\t" : "=&r" (sreg));

    // Write the registers.
    registers[address_hi] = value >> 8;
    registers[address_hi+1] = value;

    // Restore status.
    asm volatile ("out __SREG__,%0\n\t" : : "r" (sreg));
}

#if ROLLING_SUBTYPE
uint8_t registers_subtype_cycle(void)
// Cycle the output of the device subtype register with the values in the subtype_id array
{
    if (subtype_index > sizeof(subtype_id)-1) { subtype_index=0; }
    return subtype_id[subtype_index++];
}
#endif



