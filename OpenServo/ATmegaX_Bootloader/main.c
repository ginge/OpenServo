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
#include <avr/interrupt.h>

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

