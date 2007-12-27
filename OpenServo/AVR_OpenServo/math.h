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

#ifndef _OS_MATH_H_
#define _OS_MATH_H_ 1

// Multiplies the two values num1 and num2 as int32 and shifts 
// the result to the right to correct the fixpoint.
// While multiplying two fixpoint values with fixpoints at 
// bit fp_1 and fp_2 (counted from the lowest bit), the result 
// will have the fixpoint at bit
//   fp_result = fp_1 + fp_2
// To fit it to a desired fixpoint fp_3 the result must shifted by
//   shift = fp_result - fp_3
// bits to the right.
int16_t multiply(int16_t num1, int16_t num2, int8_t shift);

#endif // _OS_MATH_H_

