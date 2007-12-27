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
    eeprom_read_block(&registers[MIN_WRITE_PROTECT_REGISTER], (void *) 2, WRITE_PROTECT_REGISTER_COUNT + REDIRECT_REGISTER_COUNT);

    // Does the checksum match?
    if (header[1] != eeprom_checksum(&registers[MIN_WRITE_PROTECT_REGISTER], WRITE_PROTECT_REGISTER_COUNT + REDIRECT_REGISTER_COUNT, EEPROM_VERSION)) return 0;

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
    header[1] = eeprom_checksum(&registers[MIN_WRITE_PROTECT_REGISTER], WRITE_PROTECT_REGISTER_COUNT + REDIRECT_REGISTER_COUNT, EEPROM_VERSION);

    // Write the EEPROM header which is the first two bytes of EEPROM.
    eeprom_write_block(&header[0], (void *) 0, 2);

    // Write the write protected and redirect registers from EEPROM.
    eeprom_write_block(&registers[MIN_WRITE_PROTECT_REGISTER], (void *) 2, WRITE_PROTECT_REGISTER_COUNT + REDIRECT_REGISTER_COUNT);

    // XXX Restore PWM to servo motor.

    // Return success.
    return 1;
}


