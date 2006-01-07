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

#ifndef _OS_PWM_H_
#define _OS_PWM_H_ 1

#include "registers.h"

void pwm_init(void);
void pwm_update(uint16_t position, int16_t pwm);
void pwm_stop(void);
void pwm_cw(uint8_t pwm_width);
void pwm_ccw(uint8_t pwm_width);

inline static void pwm_enable(void)
{
	// Enable PWM to the servo motor.
	registers_write_byte(PWM_ENABLE, 1);
}


inline static void pwm_disable(void)
{
	// Disable PWM to the servo motor.
	registers_write_byte(PWM_ENABLE, 0);

	// Stop now!
	pwm_stop();
}


inline static void pwm_output(int16_t pwm)
{
	// Determine direction or stop.
	if (pwm < 0)
	{
		// Turn clockwise.
		pwm_cw(-pwm);
	}
	else if (pwm > 0)
	{
		// Turn counter-clockwise.
		pwm_ccw(pwm);
	}
	else
	{
		// Stop the motor.
		pwm_stop();
	}
}

#endif // _OS_PWM_H_
