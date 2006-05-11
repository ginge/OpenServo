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
#include <avr/io.h>

#include "config.h"
#include "bootloader.h"
#include "prog.h"
#include "timer.h"
#include "twi.h"

uint8_t bootloader_exit;
uint8_t bootloader_active;

// Don't build this function if the BOOTSTRAPPER is being built.
#if !BOOTSTRAPPER

#define TIMEOUT     128

BOOTLOADER_SECTION void bootloader(void)
// This is the main bootloader function.  When this function returns
// the application code loaded into lower memory will be executed.
// The application code must reside immediately after the vector
// table at address 0x001E.
{
    uint16_t timer_count = 0;

#if 0
    // XXX Enable PB1/PB3 as outputs and clear the LED.
    DDRB |= (1<<DDB1) | (1<<DDB3);
    PORTB |= (1<<PB1) | (1<<PB3);
#endif

    // Initialize the bootloader exit and active flags.
    bootloader_exit = 0;
    bootloader_active = 0;

    // Initialize programming module.
    prog_init();

    // Initialize TWI module.
    twi_init();

    // Initialize timer.
    timer_init();

    // Loop until the bootloader exit flag is active.
    while (!bootloader_exit)
    {
        // Check for TWI conditions that require handling.
        twi_check_conditions();

        // Check for timer elapsed.
        if (timer_check_elapsed())
        {
            // Increment the time count.
            ++timer_count ;

            // Have we exceeded the bootloader timeout (about three seconds)?
            if (timer_count > TIMEOUT)
            {
                // Set the bootloader exit flag if the bootloader is not active.
                if (!bootloader_active) bootloader_exit = 1;
            }

#if 0
            // XXX Turn off or on the LED.
            if (timer_count & 0x40)
            {
                // XXX Turn on the PB1 LED.
                PORTB &= ~(1<<PB1);
            }
            else
            {
                // XXX Turn off the PB1 LED.
                PORTB |= (1<<PB1);
            }
#endif
        }
    }

    // Restore timer to powerup defaults.
    timer_deinit();

    // Restore TWI interface to powerup defaults.
    twi_deinit();
}

#endif // !BOOTSTRAPPER
