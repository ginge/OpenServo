/*
    Copyright (c) 2007 Michael P. Thompson <mpthompson@gmail.com>

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
#include <avr/interrupt.h>

#include "config.h"
#include "bootloader.h"
#include "prog.h"
#include "timer.h"
#include "twi.h"

uint8_t bootloader_exit;
uint8_t bootloader_active;

#if !BOOTSTRAPPER

#define TIMEOUT     128

int main(void)
// This is the main bootloader function.
{
    uint16_t timer_count = 0;

    // Set up function pointer to RESET vector.
    void (*reset_vector)( void ) = 0x0000;

    // Make sure interrupts are cleared.
    cli();

    // Diable the watchdog timer in case we got here through applicaition reset

    // Clear WDRF in MCUSR.
    MCUSR &= ~(1<<WDRF);

    // Write logical one to WDCE and WDE.
    WDTCSR |= (1<<WDCE) | (1<<WDE);

    // Turn off WDT.
    WDTCSR = 0x00;

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
        }
    }

    // Restore timer to powerup defaults.
    timer_deinit();

    // Restore TWI interface to powerup defaults.
    twi_deinit();

    // Call application RESET vector.
    reset_vector();

    return 0;
}

#else

int main(void)
// This is the main bootloader function.
{
    // Make sure interrupts are cleared.
    cli();

    // Initialize the bootloader active flags.
    bootloader_active = 0;

    // Initialize programming module.
    prog_init();

    // Initialize TWI module.
    twi_init();

    // Loop forever.
    for (;;)
    {
        // Check for TWI conditions that require handling.
        twi_check_conditions();
    }

    // Restore timer to powerup defaults.
    timer_deinit();

    // Restore TWI interface to powerup defaults.
    twi_deinit();

    return 0;
}

#endif
