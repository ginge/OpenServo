/*
    Copyright (c) 2009 Barry Carter <barry.carter@gmail.com>
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

#include "openservo.h"
#include "config.h"
#include "heartbeat.h"
#include "timer.h"

volatile uint8_t heartbeat_ready;

void heartbeat_init(void)
{
    // Initialize flags and values.
    heartbeat_ready = 0;

    // Clear any pending interrupts.
    TIFR0 = (1<OCF0B) | (1<<OCF0A) | (1<TOV0);

    // Set the timer/counter0 interrupt masks.
    TIMSK0 = (1<<OCIE0A) |                                  // Interrupt on compare match A.
             (0<<OCIE0B) |                                  // No interrupt on compare match B.
             (0<<TOIE0);                                    // No interrupt on overflow.

    // Initialize the count and compare values.
    TCNT0 = 0;
    OCR0A = CRVALUE;
    OCR0B = 0;

    // Set timer/counter0 control register A.
    TCCR0A = (0<<COM0A1) | (0<<COM0A0) |                    // Disconnect OCOA.
             (0<<COM0B1) | (0<<COM0B0) |                    // Disconnect OCOB.
             (1<<WGM01) | (0<<WGM00);                       // Mode 2 - clear timer on compare match.

    // Set timer/counter0 control register B.
    TCCR0B = (0<<FOC0A) | (0<<FOC0B) |                      // No force output compare A or B.
             (0<<WGM02) |                                   // Mode 2 - clear timer on compare match.
             CSPS;                                          // Timer clock prescale -- see above.

}

SIGNAL(SIG_OUTPUT_COMPARE0A)
// Handles timer/counter0 compare match A.
{
    // Increment the timer.
    timer_increment();
    TCNT0 = 0;
    heartbeat_ready = 1;
}
