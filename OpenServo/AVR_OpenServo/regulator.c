/*
    Copyright (c) 2006 Stefan Engelke

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
#include "regulator.h"
#include "registers.h"

// Compile following for state regulator motion control algorithm.
#if REGULATOR_MOTION_ENABLED

// The minimum and maximum servo position as defined by 10-bit ADC values.
#define MIN_POSITION            (0)
#define MAX_POSITION            (1023)

// The minimum and maximum output.
#define MAX_OUTPUT              (255)
#define MIN_OUTPUT              (-MAX_OUTPUT)


void regulator_init(void)
// Initialize the motion module.
{
    // nothing to do
}


void regulator_registers_defaults(void)
// Initialize the motion related register values.  This is done here to
// keep the motion related code in a single file.  
{
    // Default control parameters.
    registers_write_word(REG_PID_PGAIN_HI, REG_PID_PGAIN_LO, 510);      // k1
    registers_write_word(REG_PID_DGAIN_HI, REG_PID_DGAIN_LO, 23287);    // k2   

    // Default position limits.
    registers_write_word(REG_MIN_SEEK_HI, REG_MIN_SEEK_LO, 0x0060);
    registers_write_word(REG_MAX_SEEK_HI, REG_MAX_SEEK_LO, 0x03A0);

    // Default reverse seek setting.
    registers_write_byte(REG_REVERSE_SEEK, 0x00);
}


int16_t regulator_position_to_pwm(int16_t current_position)
// This function takes the current servo position as input and outputs a pwm
// value for the servo motors.  The current position value must be within the
// range 0 and 1023. The output will be within the range of -255 and 255 with
// values less than zero indicating clockwise rotation and values more than
// zero indicating counter-clockwise rotation.
{
    int16_t k1;
    int16_t k2;
    int16_t output;
    int16_t command_position;
    int16_t current_velocity;
    int16_t current_error;

    // Get the command position to where the servo is moving to from the registers.
    command_position = (int16_t) registers_read_word(REG_SEEK_HI, REG_SEEK_LO);
    
    // Get estimated velocity
    current_velocity = (int16_t) registers_read_word(REG_VELOCITY_HI, REG_VELOCITY_LO);

    // Are we reversing the seek sense?
    if (registers_read_byte(REG_REVERSE_SEEK) != 0)
    {
        // Yes. Update the system registers with an adjusted reverse sense
        // position. With reverse sense, the position value to grows from
        // a low value to high value in the clockwise direction.
        registers_write_word(REG_POSITION_HI, REG_POSITION_LO, (uint16_t) (MAX_POSITION - current_position));

        // Adjust command position for the reverse sense.
        command_position = MAX_POSITION - command_position;
    }
    else
    {
        // No. Update the system registers with a non-reverse sense position.
        // Normal position value grows from a low value to high value in the
        // counter-clockwise direction.
        registers_write_word(REG_POSITION_HI, REG_POSITION_LO, (uint16_t) current_position);
    }
    
    // Get the control parameters.
    k1 = (int16_t) registers_read_word(REG_PID_PGAIN_HI, REG_PID_PGAIN_LO);
    k2 = (int16_t) registers_read_word(REG_PID_DGAIN_HI, REG_PID_DGAIN_LO);

    // Determine the current error.
    current_error = command_position - current_position;

	// The following operations are fixed point operations. To add/substract
	// two fixed point values they must have the same fractional precision
    // (the same number of bits behind the decimal).  When two fixed point
    // values are multiplied the fractional precision of the result is the sum
    // of the fractional precision of the the the factors (the sum of the bits
    // behind the decimal of each factor).  To reach the best possible precision
    // the fixed point bit is chosen for each variable separately according to 
    // its maximum and dimension.  A shift factor is then applied after
    // multiplication in the fixed_multiply() function to adjust the fractional
    // precision of the product for addition or subtraction.

    // Used fixed point bits, counted from the lowest bit:
    // Control Param. k1:  fp_k1     =  5
    // Control Param. k2:  fp_k2     =  5
    // Position state z1:  fp_z1     =  5 
    // Velocity state z2:  fp_z2     = 11
	// Real Position  x1:  fp_x1     =  0
	// PWM output:         fp_output =  0

	// output = k1 * x1 + k2 * x2
    output  = fixed_multiply(k1, current_error, 5);         // fp: 5 + 0  -> 0 : rshift = 5
    output += fixed_multiply(k2, -current_velocity, 16);    // fp: 5 + 11 -> 0 : rshift = 16

    // Check for output saturation.
    if (output > MAX_OUTPUT) output = MAX_OUTPUT;
    if (output < MIN_OUTPUT) output = MIN_OUTPUT;

    return output;
}

#endif // REGULATOR_MOTION_ENABLED
