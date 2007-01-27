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

#include <stdint.h>

#include "openservo.h"
#include "config.h"
#include "curve.h"

#if CURVE_MOTION_ENABLED

// Hermite timing parameters.
uint16_t curve_t0;
uint16_t curve_t1;
uint16_t curve_duration;
static float curve_duration_float;

// Hermite curve parameters.
float curve_p0;
float curve_p1;
float curve_v0;
float curve_v1;

// Hermite curve cubic polynomial coefficients.
static float curve_a;
static float curve_b;
static float curve_c;
static float curve_d;

void
curve_init(uint16_t t0, uint16_t t1, float p0, float p1, float v0, float v1)
{
    // Set the time parameters.
    curve_t0 = t0;
    curve_t1 = t1;
    curve_duration = t1 - t0;
    curve_duration_float = (float) curve_duration;

    // The tangents are expressed as slope of value/time.  The time span will 
    // be normalized to 0.0 to 1.0 range so correct the tangents by scaling 
    // them by the duration of the curve.
    v0 *= curve_duration_float;
    v1 *= curve_duration_float;

    // Set the curve parameters.
    curve_p0 = p0;
    curve_p1 = p1;
    curve_v0 = v0;
    curve_v1 = v1;

    // Set the cubic coefficients by multiplying the matrix form of
    // the Hermite curve by the curve parameters p0, p1, v0 and v1.
    // 
    // | a |   |  2  -2   1   1 |   |       p0       |
    // | b |   | -3   3  -2  -1 |   |       p1       |
    // | c | = |  0   0   1   0 | . | (t1 - t0) * v0 |
    // | d |   |  1   0   0   0 |   | (t1 - t0) * v1 |
    //
    // a = 2p0 - 2p1 + v0 + v1
    // b = -3p0 + 3p1 -2v0 - v1
    // c = v0
    // d = p0
    //
    curve_a = (2.0 * p0) - (2.0 * p1) + v0 + v1;
    curve_b = -(3.0 * p0) + (3.0 * p1) - (2.0 * v0) - v1;
    curve_c = v0;
    curve_d = p0;
}


void
curve_solve(uint16_t t, float *x, float *dx)
{
    // Handle cases where t is outside and indise the curve.
    if (t <= curve_t0)
    {
        // Set x and in and out dx.
        *x = curve_p0;
        *dx = t < curve_t0 ? 0.0 : curve_v0;
    }
    else if (t >= curve_t1)
    {
        // Set x and in and out dx.
        *x = curve_p1;
        *dx = t > curve_t1 ? 0.0 : curve_v1;
    }
    else
    {
        // Subtract out the t0 value from t.
        float t1 = ((float) (t - curve_t0)) / curve_duration_float;
        float t2 = t1 * t1;
        float t3 = t2 * t1;

        // Determine the cubic polynomial.
        // x = at^3 + bt^2 + ct + d
        *x = (curve_a * t3) + (curve_b * t2) + (curve_c * t1) + curve_d;
 
        // Determine the cubic polynomial derivative.
        // dx = 3at^2 + 2bt + c
        *dx = (3.0 * curve_a * t2) + (2.0 * curve_b * t1) + curve_c;

        // The time span has been normalized to 0.0 to 1.0 range so correct
        // the derivative the duration of the curve.
        *dx /= curve_duration_float;
    }
}

#endif // CURVE_MOTION_ENABLED

