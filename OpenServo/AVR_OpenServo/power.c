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

// Minimal module to support power reporting.  In the future this module
// may be expanded for the servo to monitor it's own power usage and take
// action when action when too much power is being consumed.

#include <inttypes.h>

#include "power.h"
#include "registers.h"

// Static structure for averaging power values.
static uint8_t power_index;
static uint16_t power_array[8];

void power_init(void)
// Initialize the power module.
{
    uint8_t i;

    // Initialize the power index.
    power_index = 0;

    // Initialize the power array.
    for (i = 0; i < 8; ++i) power_array[i] = 0;

    // Initialize the power values within the system registers.
    registers_write_word(POWER_HI, POWER_LO, 0);
}


void power_update(uint16_t power)
// Update the servo motor power value.  The actual value reported
// is averaged with the seven previous power values.
{
    uint8_t i;

    // Insert the power value into the power array.
    power_array[power_index] = power;

    // Keep the index within the array bounds.
    power_index = (power_index + 1) & 7;

    // Reset the power value.
    power = 0;

    // Determine the power values across the power array.
    for (i = 0; i < 8; ++i) power += power_array[i];

    // Shift the sum of power values to find the average.
    power >>= 3;

    // Update the power values within the system registers.
    registers_write_word(POWER_HI, POWER_LO, power);
}

