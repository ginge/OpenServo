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
#include <avr/eeprom.h>

#include "openservo.h"
#include "config.h"
#include "eeprom.h"
#include "registers.h"

uint8_t eeprom_is_erased(void)
// Perform simple test to determine if the EEPROM may have been erased
// and register values should not be restored from the EEPROM.
{
    // Validate certain register values that should not be 0xFF.
    if (eeprom_read_byte ((void *) (REG_TWI_ADDRESS - MIN_WP_REGISTER)) == 0xFF) return 1;
    if (eeprom_read_byte ((void *) (REG_PID_PGAIN_HI - MIN_WP_REGISTER)) == 0xFF) return 1;
    if (eeprom_read_byte ((void *) (REG_PID_DGAIN_HI - MIN_WP_REGISTER)) == 0xFF) return 1;
    if (eeprom_read_byte ((void *) (REG_PID_IGAIN_HI - MIN_WP_REGISTER)) == 0xFF) return 1;
    if (eeprom_read_byte ((void *) (REG_MIN_SEEK_HI - MIN_WP_REGISTER)) == 0xFF) return 1;
    if (eeprom_read_byte ((void *) (REG_MAX_SEEK_HI - MIN_WP_REGISTER)) == 0xFF) return 1;

    // Doesn't appear to be erased.
    return 0;
}


void eeprom_restore_registers(void)
// Restore registers from EEPROM.
{
    // XXX Disable PWM to servo motor while reading registers.

    // Read the safe read/write registers from EEPROM.
    eeprom_read_block(&registers[MAX_RW_REGISTER + 1], (void *) 0, 16);

    // XXX Restore PWM to servo motor.

    return;
}


void eeprom_save_registers(void)
// Save registers to EEPROM.
{
    // XXX Disable PWM to servo motor while reading registers.

    // Write the safe read/write registers from EEPROM.
    eeprom_write_block(&registers[MAX_RW_REGISTER + 1], (void *) 0, 16);

    // XXX Restore PWM to servo motor.

    return;
}


