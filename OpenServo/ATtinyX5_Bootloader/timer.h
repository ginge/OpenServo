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

#ifndef _TIMER_H_
#define _TIMER_H_ 1


static inline void timer_init(void)
{
    // Configure the Timer Control Register A.
    TCCR0A = (0<<COM0A1) | (0<<COM0A0) |        // OC0A disconnected.
             (0<<COM0B1) | (0<<COM0B0) |        // OC0B disconnected.
             (0<<WGM01) | (0<<WGM00);           // Normal operation.

    // Configure the Timer Control Register B.
    TCCR0B = (0<<WGM02) |                       // Normal operation.
             (1<<CS02) | (0<<CS01) | (1<<CS00); // Use pre/scaled (1024) clock source.
}


static inline void timer_deinit(void)
{
    // Clear timer related registers.
    TCCR0A = 0;
    TCCR0B = 0;
    TCNT0 = 0;
    TIFR = (1<<OCF0A) | (1<<OCF0B) | (1<<TOV0);
}


static inline uint8_t timer_check_elapsed(void)
{
    // Has the timer overflowed?
    if (TIFR & (1<<TOV0))
    {
        // Reset the overflow flag.
        TIFR |= (1<<TOV0);

        // Return that one time period has elapsed.
        return 1;
    }

    return 0;
}

#endif // _TIMER_H_
