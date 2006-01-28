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
#include <avr/eeprom.h>

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


