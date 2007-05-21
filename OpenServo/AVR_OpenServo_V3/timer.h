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

#ifndef _OS_TIMER_H_
#define _OS_TIMER_H_ 1

#include "registers.h"

static inline void timer_set(uint16_t value)
{
    // Set the timer to the specified value.
    registers_write_word(REG_TIMER_HI, REG_TIMER_LO, value);
}

static inline uint16_t timer_get(void)
{
    // Get the timer value.
    return registers_read_word(REG_TIMER_HI, REG_TIMER_LO);
}

static inline uint16_t timer_delta(uint16_t time_stamp)
{
    uint16_t delta_time;

    // Get the current time.
    uint16_t current_time = registers_read_word(REG_TIMER_HI, REG_TIMER_LO);

    // Determine the time delta from the time stamp.
    if (current_time > time_stamp)
        delta_time = current_time - time_stamp;
    else
        delta_time = 0xffff - (time_stamp - current_time) + 1;

    return delta_time;
}

static inline void timer_increment(void)
{
    uint16_t value;

    // Read the timer value.
    value = registers_read_word(REG_TIMER_HI, REG_TIMER_LO);

    // Increment the timer value.
    ++value;

    // Write the increment timer value.
    registers_write_word(REG_TIMER_HI, REG_TIMER_LO, value);
}


#endif // _OS_TIMER_H_

