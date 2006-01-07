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

#ifndef _TIMER_H_
#define _TIMER_H_ 1


static inline void timer_init(void)
{
	// Configure the Timer Control Register A.
	TCCR0A = (0<<COM0A1) | (0<<COM0A0) |		// OC0A disconnected.
			 (0<<COM0B1) | (0<<COM0B0) |		// OC0B disconnected.
			 (0<<WGM01) | (0<<WGM00);			// Normal operation.

	// Configure the Timer Control Register B.
	TCCR0B = (0<<WGM02) |						// Normal operation.
			 (1<<CS02) | (0<<CS01) | (1<<CS00);	// Use pre/scaled (1024) clock source.
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
