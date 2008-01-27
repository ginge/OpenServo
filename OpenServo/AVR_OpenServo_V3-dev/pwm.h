/*
    Copyright (c) 2006 Michael P. Thompson <mpthompson@gmail.com>
        Portions of this code 
    Copyright (c) 2007 Barry Carter <barry.carter@gmail.com>


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

#ifndef _OS_PWM_H_
#define _OS_PWM_H_ 1

#include "registers.h"

void pwm_registers_defaults(void);
void pwm_init(void);
void pwm_update(uint16_t position, int16_t pwm);
void pwm_stop(void);

inline static void pwm_enable(void)
{
    uint8_t flags_lo = registers_read_byte(REG_FLAGS_LO);

    // Enable PWM to the servo motor.
    registers_write_byte(REG_FLAGS_LO, flags_lo | (1<<FLAGS_LO_PWM_ENABLED));
}


inline static void pwm_disable(void)
{
    uint8_t flags_lo = registers_read_byte(REG_FLAGS_LO);

    // Disable PWM to the servo motor.
    registers_write_byte(REG_FLAGS_LO, flags_lo & ~(1<<FLAGS_LO_PWM_ENABLED));

    // Stop now!
    pwm_stop();
}


inline static void pwm_brake_enable(void)
{
    uint8_t flags_lo = registers_read_byte(REG_FLAGS_LO);

    // Enable Hardware brake
    registers_write_byte(REG_FLAGS_LO, flags_lo | (1<<FLAGS_LO_GENERALCALL_ENABLED));
}


inline static void pwm_brake_disable(void)
{
    uint8_t flags_lo = registers_read_byte(REG_FLAGS_LO);

    // Disable hardware brake.
    registers_write_byte(REG_FLAGS_LO, flags_lo & ~(1<<FLAGS_LO_GENERALCALL_ENABLED));
}

#endif // _OS_PWM_H_
