/*
    Copyright (c) 2007 Barry Carter <Barry.Carter@gmail.com>

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

/*
    This code adds bank selection functionality. This allows the 
    OpenServo to add a near infinite amount of r/w registers.

    USE: the bank selection register is REG_BANK_SELECT as defined
    in registers.h setting this value changes the current bank of
    BANK_REGISTER_COUNT registers at address MIN_BANK_REGISTER.
    The maximum number of banks is defined in registers.h as
    MAX_BANKS and trying to select a bank over this value will
    cap to this value.

    in order to read and write data from the registers, you must 
    supply a bank value eg. banks_read_word(uint8_t bank, ...)

    Available banks are defined in banks.h

    NOTE: all of these registers are r/w with no protection
    care must be taken not to destroy your own data.

*/

#include <inttypes.h>
#include <string.h>

#include "openservo.h"
#include "config.h"
#include "eeprom.h"
#include "pid.h"
#include "pwm.h"
#include "registers.h"
#include "banks.h"
#include "alert.h"
#include <avr/eeprom.h>

// Register values.
uint8_t banks[MAX_BANKS][BANK_REGISTER_COUNT];

void banks_init(void)
// Function to initialize all banks.
{
    uint8_t n;

    // Initialize all banks to zero.
    for( n=0;n<MAX_BANKS;n++)
        memset(&banks[n][0], 0, BANK_REGISTER_COUNT);

    // Reset the bank selection to 0
    registers_write_byte(REG_BANK_SELECT,0);

    // Restore the read/write protected registers from EEPROM.  If the
    // EEPROM fails checksum this function will return zero and the
    // read/write protected registers should be initialized to defaults.
    if (!eeprom_restore_registers())
    {
        // Initialize read/write protected registers to defaults.
        registers_defaults();
    }

    //initialise the alert registers
    alert_init();
}


void banks_defaults(void)
// Reset safe read/write registers to defaults.
{
  // put your default bank data here

    // Default TWI address.
    banks_write_byte(POS_PID_BANK, REG_TWI_ADDRESS, REG_DEFAULT_TWI_ADDR);
    alert_defaults();
}

uint16_t banks_read_word(uint8_t bank, uint8_t address_hi, uint8_t address_lo)
// Read a 16-bit word from the registers.
// Interrupts are disabled during the read.
{
    uint8_t sreg;
    uint16_t value;


    // Clear interrupts.
    asm volatile ("in %0,__SREG__\n\tcli\n\t" : "=&r" (sreg));

    // Read the registers.
    value = (banks[bank][address_hi] << 8) | banks[bank][address_lo];

    // Restore status.
    asm volatile ("out __SREG__,%0\n\t" : : "r" (sreg));

    return value;
}


void banks_write_word(uint8_t bank, uint8_t address_hi, uint8_t address_lo, uint16_t value)
// Write a 16-bit word to the registers.
// Interrupts are disabled during the write.
{
    uint8_t sreg;

    // Clear interrupts.
    asm volatile ("in %0,__SREG__\n\tcli\n\t" : "=&r" (sreg));

    // Write the registers.
    banks[bank][address_hi] = value >> 8;
    banks[bank][address_lo] = value;

    // Restore status.
    asm volatile ("out __SREG__,%0\n\t" : : "r" (sreg));
}

uint8_t banks_save_registers(void)
// Save the registers that need saving in the bank. Add your save functions here
// There is no checksum done on these as yet. Needs discussion
{

    // Save the redirected registers at REGISTERS_COUNT + 3 NOTE: no checksum!
    eeprom_write_block(&banks[REDIRECTED_BANK][MIN_REDIRECT_REGISTER], 
                        (void *)(WRITE_PROTECT_REGISTER_COUNT + 2),
                         REDIRECT_REGISTER_COUNT);

    // Save the alert registers
    eeprom_write_block(&banks[CONFIG_BANK][CONFIG_SAVE_MIN], 
                        (void *)(WRITE_PROTECT_REGISTER_COUNT + REDIRECT_REGISTER_COUNT + 2), 
                         CONFIG_SAVE_COUNT);

    return 1;
}

uint8_t banks_restore_registers(void)
// Load any addition bank related registers into the banks
// There is no checksum done on these as yet. Needs discussion

{

    // Load the redirected registers out of the eeprom
    eeprom_read_block(&banks[REDIRECTED_BANK][MIN_REDIRECT_REGISTER], 
                       (void *)(WRITE_PROTECT_REGISTER_COUNT + 2), 
                        REDIRECT_REGISTER_COUNT);

    // Load the configuration bank
    eeprom_read_block(&banks[CONFIG_BANK][CONFIG_SAVE_MIN], 
                       (void *)(WRITE_PROTECT_REGISTER_COUNT + REDIRECT_REGISTER_COUNT + 2), 
                        CONFIG_SAVE_COUNT);

    return 1;
}

void banks_update_registers(void)
// Update any functions that need running in the bank context
// This runs in the main loop so DO NOT tie it up too long.
{

}
