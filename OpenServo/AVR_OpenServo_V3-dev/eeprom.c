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
#include <avr/io.h>
#include <avr/eeprom.h>

#include "openservo.h"
#include "config.h"
#include "eeprom.h"
#include "registers.h"
#include "banks.h"

static uint8_t eeprom_checksum(const uint8_t *buffer, size_t size, uint8_t sum)
// Adds the buffer to the checksum passed in returning the updated sum.
{
    // Keep going while bytes in the buffer.
    while (size)
    {
        // Add the buffer to the sum.
        sum += *buffer;

        // One less byte left.
        ++buffer;
        --size;
    }

    // Return the new sum.
    return sum;
}


uint8_t eeprom_erase(void)
// Erase the entire EEPROM.
{
    uint16_t i;
    uint8_t buffer[16];

    // XXX Disable PWM to servo motor while reading registers.

    // Clear the buffer contents to 0xFF.
    memset(buffer, 0xFF, sizeof(buffer));

    // Loop over the EEPROM in buffer increments.
    for (i = 0; i < E2END; i += sizeof(buffer))
    {
        // Write the buffer to the block of EEPROM.
        eeprom_write_block(buffer, (void *) i, sizeof(buffer));
    }

    // XXX Restore PWM to servo motor.

    // Return success.
    return 1;
}


uint8_t eeprom_restore_registers(void)
// Restore registers from EEPROM.  Returns 1 if success or 0 if the registers failed
// checksum.  Upon failure the caller should initialize the registers to defaults.
{
    uint8_t header[2];

    // XXX Disable PWM to servo motor while reading registers.

    // Read EEPROM header which is the first two bytes of EEPROM.
    eeprom_read_block(&header[0], (void *) 0, 2);

    // Does the version match?
    if (header[0] != EEPROM_VERSION) return 0;

    // Read the write protected and redirect registers from EEPROM.
    eeprom_read_block(&registers[MIN_WRITE_PROTECT_REGISTER], (void *) 2, WRITE_PROTECT_REGISTER_COUNT);

    // Load any configuration registers saves in banks

    // Does the checksum match?
    if (header[1] != eeprom_checksum(&registers[MIN_WRITE_PROTECT_REGISTER], WRITE_PROTECT_REGISTER_COUNT, EEPROM_VERSION)) return 0;

    // Read EEPROM header which is the first two bytes of EEPROM.
    eeprom_read_block(&header[0], (void *) WRITE_PROTECT_REGISTER_COUNT + 2, 1);

    // Load the redirected registers out of the eeprom
    eeprom_read_block(&banks[CONFIG_BANK][CONFIG_SAVE_MIN], 
                       (void *)(WRITE_PROTECT_REGISTER_COUNT + 3), 
                        CONFIG_SAVE_COUNT);

    // Does the checksum match?
    if (header[0] != eeprom_checksum(&banks[CONFIG_BANK][CONFIG_SAVE_MIN], CONFIG_SAVE_COUNT, EEPROM_VERSION)) return 0;

    // Read redirect page 2
    // Read EEPROM header
    eeprom_read_block(&header[0], (void *) CONFIG_SAVE_COUNT + WRITE_PROTECT_REGISTER_COUNT + 4, 1);

    // Load the configuration bank
    eeprom_read_block(&banks[REDIRECTED_BANK][MIN_REDIRECT_REGISTER], 
                       (void *)(WRITE_PROTECT_REGISTER_COUNT + CONFIG_SAVE_COUNT + 5), 
                        REDIRECT_REGISTER_COUNT);

    // Does the checksum match?
    if (header[0] != eeprom_checksum(&banks[REDIRECTED_BANK][MIN_REDIRECT_REGISTER], REDIRECT_REGISTER_COUNT, EEPROM_VERSION)) return 0;

    // XXX Restore PWM to servo motor.

    // Return success.
    return 1;
}


uint8_t eeprom_save_registers(void)
// Save registers to EEPROM.
{
    uint8_t header[2];

    // XXX Disable PWM to servo motor while reading registers.

    // Fill in the EEPROM header.
    header[0] = EEPROM_VERSION;
    header[1] = eeprom_checksum(&registers[MIN_WRITE_PROTECT_REGISTER], WRITE_PROTECT_REGISTER_COUNT, EEPROM_VERSION);

    // Write the EEPROM header which is the first two bytes of EEPROM.
    eeprom_write_block(&header[0], (void *) 0, 2);

    // Write the write protected and redirect registers from EEPROM.
    eeprom_write_block(&registers[MIN_WRITE_PROTECT_REGISTER], (void *) 2, WRITE_PROTECT_REGISTER_COUNT);

    // Save any registers defined in banks.
    
    // Save the config bank (1)
    // Calculate a new checksum for bank 1
    header[0] = eeprom_checksum(&banks[CONFIG_BANK][CONFIG_SAVE_MIN], CONFIG_SAVE_COUNT, EEPROM_VERSION);

    // Write the EEPROM header which is 1 byte long
    eeprom_write_block(&header[0], (void *) WRITE_PROTECT_REGISTER_COUNT + 2, 1);

    // Save the redirected registers at REGISTERS_COUNT + 3
    eeprom_write_block(&banks[CONFIG_BANK][CONFIG_SAVE_MIN], 
                        (void *)(WRITE_PROTECT_REGISTER_COUNT + 3),
                         CONFIG_SAVE_COUNT);

    // Calculate the checksum for bank 2 redirects
    header[0] = eeprom_checksum(&banks[REDIRECTED_BANK][MIN_REDIRECT_REGISTER], REDIRECT_REGISTER_COUNT, EEPROM_VERSION);

    // Write the EEPROM header which is 1 byte long, after bank 0
    eeprom_write_block(&header[0], (void *) CONFIG_SAVE_COUNT + WRITE_PROTECT_REGISTER_COUNT + 4, 1);

    // Save the alert registers
    eeprom_write_block(&banks[REDIRECTED_BANK][MIN_REDIRECT_REGISTER], 
                        (void *)(WRITE_PROTECT_REGISTER_COUNT + CONFIG_SAVE_COUNT + 5), 
                         REDIRECT_REGISTER_COUNT);

    // XXX Restore PWM to servo motor.

    // Return success.
    return 1;
}


