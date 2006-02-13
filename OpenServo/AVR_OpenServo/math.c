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
// The following is needed until WINAVR supports the ATtinyX5 MCUs.
#undef __AVR_ATtiny2313__
#define __AVR_ATtiny45__

#include <inttypes.h>

#include "math.h"

static int32_t shift_right32(int32_t val, uint8_t cnt)
// Arithmetic shift right of the signed 32-bit value.
{
    asm volatile (
        "L_asr1%=:" "\n\t"
        "cp %1,__zero_reg__" "\n\t"
        "breq L_asr2%=" "\n\t"
        "dec %1" "\n\t"
        "asr %D0" "\n\t"
        "ror %C0" "\n\t"
        "ror %B0" "\n\t"
        "ror %A0" "\n\t"
        "rjmp L_asr1%=" "\n\t"
        "L_asr2%=:" "\n\t"
        : "=&r" (val)
        : "r" (cnt), "0" (val)
        );

    return val;
}

int16_t multiply(int16_t num1, int16_t num2, int8_t shift)
// Multiplies the two values num1 and num2 as int32 and shifts 
// the result to the right to correct the fixpoint.
// While multiplying two fixpoint values with fixpoints at 
// bit fp_1 and fp_2 (counted from the lowest bit), the result 
// will have the fixpoint at bit
//   fp_result = fp_1 + fp_2
// To fit it to a desired fixpoint fp_3 the result must shifted by
//   shift = fp_result - fp_3
// bits to the right.
{
    int32_t result;
    result = (int32_t) num1 * (int32_t) num2;
    result = shift_right32(result,shift);
    return (int16_t) result;
}
