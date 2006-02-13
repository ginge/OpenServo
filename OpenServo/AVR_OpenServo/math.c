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

#include "config.h"
#include "math.h"

#if FIXED_MATH_ENABLED

int16_t fixed_multiply(int16_t num1, int16_t num2, int8_t shift)
// Multiplies the two 16-bit values num1 and num2 into a 32-bit
// product.  The product is then right shifted by the specified 
// amount and the lower 16-bits of the value returned.  The purpose
// of this function is to facilitate fixed point arithmetic.  When
// multiplying two fixed point numbers with fixed points at bit 
// fp_1 and fp_2 (counted from the least significant bit), the 
// result will have the fixed point bit at:
//
//    fp_result = fp_1 + fp_2
//
// To fit to a desired fixed point precision fp_3 the result must
// be shifted to the right by:
//
//    shift = fp_result - fp_3
//
{
    int32_t result;

    // Multiply the 16-bit values into a 32-bit product.
    result = (int32_t) num1 * (int32_t) num2;

    // Right shift to adjust fixed point precision.
    result = shift_right32(result,shift);

    // Return lower 16-bits of result.  The caller should be 
    // careful to not overflow the result.
    return (int16_t) result;
}

#endif
