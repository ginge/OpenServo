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

    Implements a queue of curve data for motion control.
*/

#ifndef _OS_MOTION_H_
#define _OS_MOTION_H_ 1

#include "registers.h"

// Buffer size must be a power of two.
#define MOTION_BUFFER_SIZE       8
#define MOTION_BUFFER_MASK       (MOTION_BUFFER_SIZE - 1)

// Exported variables.
extern uint8_t motion_head;
extern uint8_t motion_tail;
extern uint32_t motion_counter;
extern uint32_t motion_duration;

// Motion functions.
void motion_init(void);
void motion_reset(int16_t position);
void motion_registers_reset(void);
uint8_t motion_append(void);
void motion_next(uint16_t delta);
uint8_t motion_buffer_left(void);

// Motion inline functions.

inline static void motion_enable(void)
{
    uint8_t flags_lo = registers_read_byte(REG_FLAGS_LO);

    // Enable PWM to the servo motor.
    registers_write_byte(REG_FLAGS_LO, flags_lo | (1<<FLAGS_LO_MOTION_ENABLED));
}


inline static void motion_disable(void)
{
    uint8_t flags_lo = registers_read_byte(REG_FLAGS_LO);

    // Disable PWM to the servo motor.
    registers_write_byte(REG_FLAGS_LO, flags_lo & ~(1<<FLAGS_LO_MOTION_ENABLED));
}


inline static uint32_t motion_time_left(void)
// Get the remaining time of the buffered curves.
{
    // The time left is the buffer duration minus the buffer counter.
    return motion_duration - motion_counter;
}

#endif // _OS_MOTION_H_

