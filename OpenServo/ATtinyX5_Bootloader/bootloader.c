/*
   Copyright (c) 2005, Mike Thompson <mpthompson@gmail.com>
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

#undef __AVR_ATtiny2313__
#define __AVR_ATtiny45__

#include <inttypes.h>
#include <avr/io.h>

#include "bootloader.h"
#include "prog.h"
#include "timer.h"
#include "twi.h"

uint8_t bootloader_exit;
uint8_t bootloader_active;

// Don't build this function if the BOOTSTRAPPER is being built.
#ifndef BOOTSTRAPPER

BOOTLOADER_SECTION void bootloader(void)
// This is the main bootloader function.  When this function returns
// the application code loaded into lower memory will be executed.
// The application code must reside immediately after the vector
// table at address 0x001E.
{
    uint8_t timer_count = 0;

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
            if (timer_count > 128)
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

    return;
}

#endif // !BOOTSTRAPPER
