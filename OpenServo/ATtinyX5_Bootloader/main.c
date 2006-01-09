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
#include <avr/interrupt.h>
#include <avr/signal.h>

#include "bootloader.h"
#ifdef BOOTSTRAPPER
#include "prog.h"
#include "twi.h"
#endif

int
main (void)
// This is just a place holder application that will be replaced by
// bootloader programming.  When built with BOOTSTRAPPER defined the
// application will allow the bootloader itself to be programmed.
{

#ifdef BOOTSTRAPPER
    // Initialize the bootloader exit and active flags.
    bootloader_exit = 0;
    bootloader_active = 0;

    // Initialize programming module.
    prog_init();

    // Initialize TWI module.
    twi_init();
#endif

    // Loop forever.
    for (;;)
    {
#ifdef BOOTSTRAPPER
        // Check for TWI conditions that require handling.
        twi_check_conditions();
#endif
    }

#ifdef BOOTSTRAPPER
    // Restore TWI interface to powerup defaults.
    twi_deinit();
#endif

    return 0;
}

