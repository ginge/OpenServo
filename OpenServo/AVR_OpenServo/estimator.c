/*
    Copyright (c) 2006 Stefan Engelke

    Permission is hereby granted, free of charge, to any
    person obtaining a copy of this software and associated
    documentation files (the "Software"), to deal in the Software
    without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to
    whom the Software is furnished to do so, subject to the
    following conditions:

    The above copyright notice and this permission notice shall
    be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
    KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
    PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
    OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
    OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
    OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// The following is needed until WINAVR supports the ATtinyX5 MCUs.
#undef __AVR_ATtiny2313__
#define __AVR_ATtiny45__

#include <inttypes.h>

#include "config.h"
#include "registers.h"

#if ESTIMATOR_ENABLED

// This file implements the Luenberg state estimator algorithm
// for determining servo speed.  It is a realtime simulation of
// the servo behavior with a internal controller which keeps 
// track of the difference between the simulated position and
// the measured one.  Currently, the constants are hardcoded
// for the OpenServo hardware derived from a Futaba S3003 servo.
// These constants will soon be moved to a header file and 
// instructions created for how to determine constants for other
// servo hardware.

// Simulation states preserved across multiple estimation iterations.
static int16_t z1;      // Position
static int16_t z2;      // Velocity


int16_t multiply(int16_t num1, int16_t num2, int32_t denum)
// Multiplies numerator1 and numerator2 as int32 and devide it through 
// the denumerator
{
    int32_t result;
    result = (int32_t) num1 * (int32_t) num2;
    result /= denum;
    return (int16_t) result;
}


void estimator_init(void)
// Initialize the state estimator module.
{
    // Initialize simulation states
    z1 = 0; // Position
    z2 = 0; // Velocity
}


void estimator_registers_defaults(void)
// Initialize the state estimation related register values.
{
    // System parameters
    // x1' = x2
    // x2' = a * x2 + b * PWM
    registers_write_word(REG_RESERVED_30, REG_RESERVED_31,  -1115); // a
    registers_write_word(REG_RESERVED_32, REG_RESERVED_33,   8849); // b

    // Estimator parameters
    // z1' = L1 * (x1-z1) + z2
    // z2' = L2 * (x1-z1) + a * z2 + b * PWM
    registers_write_word(REG_RESERVED_34, REG_RESERVED_35,   3144); // L1
    registers_write_word(REG_RESERVED_36, REG_RESERVED_37,   3915); // L2

    /*registers_write_word(REG_RESERVED_30, REG_RESERVED_31,  -1115); // sys a
    registers_write_word(REG_RESERVED_32, REG_RESERVED_33,  32767); // sys b
    registers_write_word(REG_RESERVED_34, REG_RESERVED_35,   3144); // L1
    registers_write_word(REG_RESERVED_36, REG_RESERVED_37,   3915); // L2*/

    // Initialize a velocity of zero
    registers_write_word(REG_VELOCITY_HI, REG_VELOCITY_LO,   0);    
}


void estimate_velocity(int16_t current_position)
// Take the 10-bit position as input and estimates the velocity
// The last PWM signal is taken from the registers and the
// calculated velocity is written back to the registers
{
    // Right hand site of the differential equations
    int16_t z1d;
    int16_t z2d;

    // Estimation error 
    int16_t estim_error;

    // Read parameters from registers
    int16_t a  = (int16_t) registers_read_word(REG_RESERVED_30, REG_RESERVED_31);
    int16_t b  = (int16_t) registers_read_word(REG_RESERVED_32, REG_RESERVED_33);
    int16_t L1 = (int16_t) registers_read_word(REG_RESERVED_34, REG_RESERVED_35);
    int16_t L2 = (int16_t) registers_read_word(REG_RESERVED_36, REG_RESERVED_37);
    
    
    // Read last PWM
    int16_t lastPWM = (int16_t) registers_read_byte(REG_PWM_CCW) - 
                      (int16_t) registers_read_byte(REG_PWM_CW);
    
    // ** Bits of fixed point **
    // fp_a           = 16
    // fp_b           = 27
    // fp_L1          = 16
    // fp_L2          = 24
    // fp_z1          = 5 
    // fp_z2          = 11
    // fp_z1d         = 5
    // fp_z2d         = 11
    // fp_estim_error = 5
    
    // Estimation_error = real_position - simulated_position
    estim_error  = current_position * 32;       // fp: 0     -> 5  : 2^(+ 5) = 32
    estim_error -= z1;                          // fp: 5     -> 5  : 2^(  0) = 1

    // z1' = L1 * (x1-z1) + z2
    z1d  = multiply(L1, estim_error, 0xFFFF);   // fp: 16+5  -> 5  : 2^(-16) = 1/0xFFFF
    z1d += z2 / 64;                             // fp: 11    -> 5  : 2^(- 6) = 1/64

    // z2' = L2 * (x1-z1) + a * z2 + b * PWM
    z2d  = multiply(L2, estim_error, 262144);   // fp: 24+5  -> 11 : 2^(-18) = 1/262144
    z2d += multiply(a, z2, 0xFFFF);             // fp: 16+11 -> 11 : 2^(-16) = 1/0xFFFF
    z2d += multiply(b, lastPWM, 0xFFFF);        // fp: 27+0  -> 11 : 2^(-16) = 1/0xFFFF

    // Numerical Integration: Euler forward with step width of 1
    z1  += z1d;                                 // fp: 5     -> 5  : 2^(  0) = 1
    z2  += z2d;                                 // fp: 11    -> 11 : 2^(  0) = 1
  
    // Write estimated velocity to the registers
    registers_write_word(REG_VELOCITY_HI, REG_VELOCITY_LO, z2);
}

#endif // ESTIMATOR_ENABLED
