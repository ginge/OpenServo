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

    Implements hermite cubic curve object.
*/

#ifndef _OS_CURVE_H_
#define _OS_CURVE_H_ 1

// Curve exported variables.
extern uint16_t curve_t0;
extern uint16_t curve_t1;
extern uint16_t curve_duration;

// Hermite curve parameters.
extern float curve_p0;
extern float curve_p1;
extern float curve_v0;
extern float curve_v1;

// Curve methods.
void curve_init(uint16_t t0, uint16_t t1, float p0, float p1, float v0, float v1);
void curve_solve(uint16_t t, float *x, float *dx);

// Inline methods.
inline static uint16_t curve_get_t0(void) { return curve_t0; }
inline static uint16_t curve_get_t1(void) { return curve_t1; }
inline static uint16_t curve_get_duration(void) { return curve_duration; }
inline static float curve_get_p0(void) { return curve_p0; }
inline static float curve_get_p1(void) { return curve_p1; }
inline static float curve_get_v0(void) { return curve_v0; }
inline static float curve_get_v1(void) { return curve_v1; }

#endif // _OS_CURVE_H_
