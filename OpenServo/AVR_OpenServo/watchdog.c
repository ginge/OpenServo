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

// The following is needed until WINAVR supports the ATtinyX5 MCUs.
#undef __AVR_ATtiny2313__
#define __AVR_ATtiny45__

// Minimal module to support watchdog timer.  This module may be expanded in
// the future to support more robust use of the watchdog timer to monitor
// error conditions in the servo.

#include <inttypes.h>
#include <avr/io.h>

#include "pwm.h"

void watchdog_init(void)
// Initialize the watchdog module.
{
    // Clear WDRF in MCUSR.
    MCUSR = 0x00;

    // Write logical one to WDCE and WDE.
    WDTCR |= (1<<WDCE) | (1<<WDE);

    // Turn off WDT.
    WDTCR = 0x00;
}


void watchdog_hard_reset(void)
// Reset the device using the watchdog timer.
{
    // Disable PWM to the servo motor.
    pwm_disable();

    // Enable the watchdog.
    WDTCR = (1<<WDIF) |                                     // Reset any interrupt.
            (0<<WDIE) |                                     // Disable interrupt.
            (1<<WDE) |                                      // Watchdog enable.
            (0<<WDP3) | (0<<WDP2) | (0<<WDP1) | (0<<WDP0);  // Minimum prescaling - 16mS.

    // Wait for reset to occur.
    for (;;);
}

