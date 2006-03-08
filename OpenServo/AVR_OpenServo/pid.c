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

#include <inttypes.h>

#include "openservo.h"
#include "config.h"
#include "pid.h"
#include "registers.h"

// Compile following for PID motion control algorithm.
#if PID_MOTION_ENABLED

// The minimum and maximum servo position as defined by 10-bit ADC values.
#define MIN_POSITION            (0)
#define MAX_POSITION            (1023)

// The minimum and maximum output.
#define MAX_OUTPUT              (255)
#define MIN_OUTPUT              (-MAX_OUTPUT)

// Defines for maximum and minimum integral error.  These values
// are used to keep the integral error from winding up or down 
// to unreasonable values.
#define MAX_INTEGRAL_ERROR  (4000)
#define MIN_INTEGRAL_ERROR  (-4000)

// Borrow a reserved register.
#define REG_PID_OFFSET          REG_RESERVED_21

// Values preserved across multiple PID iterations.
static int16_t integral_error = 0;
static int16_t previous_position = 0;

static int16_t gain_multiply(int16_t error, uint16_t gain)
// Multiplies the PID error value by the PID gain value.
// The result is scaled and bounds checked to fit within
// a signed sixteen bit return value.
{
    int32_t result;

    // Multiply the error times the gain.
    result = (int32_t) error * (int32_t) gain;

    // Divide result by 256 to account for fixed point gain.
    result /= 256;

    // Perform bounds checking against reasonable miniums and maximums.
    // We keep the individual results between -10000 and 10000 so that
    // the combined proportional, integral and derivative results fit
    // within signed sixteen bit values of -32768 and 32767.
    if (result > 10000) result = 10000;
    if (result < -10000) result = -10000;

    return (int16_t) result;
}


void pid_init(void)
// Initialize the PID algorithm module.
{
    // Initialize preserved values.
    integral_error = 0;
    previous_position = 0;
}


void pid_registers_defaults(void)
// Initialize the PID algorithm related register values.  This is done 
// here to keep the PID related code in a single file.  
{
    // Default gain values.
    registers_write_word(REG_PID_PGAIN_HI, REG_PID_PGAIN_LO, 0x0600);
    registers_write_word(REG_PID_DGAIN_HI, REG_PID_DGAIN_LO, 0x7000);
    registers_write_word(REG_PID_IGAIN_HI, REG_PID_IGAIN_LO, 0x0001);

    // Default offset value.
    registers_write_byte(REG_PID_OFFSET, 0x80);

    // Default position limits.
    registers_write_word(REG_MIN_SEEK_HI, REG_MIN_SEEK_LO, 0x0060);
    registers_write_word(REG_MAX_SEEK_HI, REG_MAX_SEEK_LO, 0x03A0);

    // Default reverse seek setting.
    registers_write_byte(REG_REVERSE_SEEK, 0x00);
}


int16_t pid_position_to_pwm(int16_t current_position)
// This function takes the current servo position as input and outputs a pwm
// value for the servo motors.  The current position value must be within the
// range 0 and 1023. The output will be within the range of -255 and 255 with
// values less than zero indicating clockwise rotation and values more than
// zero indicating counter-clockwise rotation.
{
    int16_t output;
    int16_t output_range;
    int16_t output_offset;
    int16_t command_position;
    int16_t minimum_position;
    int16_t maximum_position;
    int16_t derivative_error;
    int16_t proportional_error;
    uint16_t integral_gain;
    uint16_t derivative_gain;
    uint16_t proportional_gain;

    // Initialize the output.
    output = 0;

    // Get the command position to where the servo is moving to from the registers.
    command_position = (int16_t) registers_read_word(REG_SEEK_HI, REG_SEEK_LO);
    minimum_position = (int16_t) registers_read_word(REG_MIN_SEEK_HI, REG_MIN_SEEK_LO);
    maximum_position = (int16_t) registers_read_word(REG_MAX_SEEK_HI, REG_MAX_SEEK_LO);

    // Are we reversing the seek sense?
    if (registers_read_byte(REG_REVERSE_SEEK) != 0)
    {
        // Yes. Update the system registers with an adjusted reverse sense
        // position. With reverse sense, the position value to grows from
        // a low value to high value in the clockwise direction.
        registers_write_word(REG_POSITION_HI, REG_POSITION_LO, (uint16_t) (MAX_POSITION - current_position));

        // Adjust command position for the reverse sense.
        command_position = MAX_POSITION - command_position;
        minimum_position = MAX_POSITION - minimum_position;
        maximum_position = MAX_POSITION - maximum_position;
    }
    else
    {
        // No. Update the system registers with a non-reverse sense position.
        // Normal position value grows from a low value to high value in the
        // counter-clockwise direction.
        registers_write_word(REG_POSITION_HI, REG_POSITION_LO, (uint16_t) current_position);
    }

    // Sanity check the command position. We do this because this value is
    // passed from the outside to the servo and it could be an invalid value.
    if (command_position < minimum_position) command_position = minimum_position;
    if (command_position > maximum_position) command_position = maximum_position;

    // Determine the proportional error as the difference between the
    // command position and the current position.
    proportional_error = command_position - current_position;

    // Determine the derivative error as the difference between the 
    // current position and the previous position.  This is value is
    // essentially the velocity the servo is currently moving. Normally
    // the derivative error is the change in proportional error, but we
    // use velocity instead so a large change in the command position
    // doesn't induce a large derivative error.
    derivative_error = current_position - previous_position;

    // Update the previous position.
    previous_position = current_position;

    // Are we in an overshoot situation?  Overshoot is detected when the
    // signs of the integral error and proportional error disagree.
    if (((proportional_error > 0) && (integral_error < 0)) || 
        ((proportional_error < 0) && (integral_error > 0)))
    {
        // Yes. Dampen the integral error.  The integral wants to integrate to 
        // zero -- any positive error must be matched by an equal amount of 
        // negative error.  Friction within the servo may not provide enough 
        // dampening and the servo will overshoot and oscillate about the seek
        // position. Overshoot is detected when the signs of the integral error
        // and proportional error disagree and we can immediately dampen 
        // the integral error to reduce oscillations around the seek point.
        integral_error = 0;
    }

    // Get the proportional, derivative and integral gains.
    proportional_gain = registers_read_word(REG_PID_PGAIN_HI, REG_PID_PGAIN_LO);
    derivative_gain = registers_read_word(REG_PID_DGAIN_HI, REG_PID_DGAIN_LO);
    integral_gain = registers_read_word(REG_PID_IGAIN_HI, REG_PID_IGAIN_LO);

    // Add the proportional and derivative components of the output.
    // The derivative component is subtracted as it is meant to serve 
    // as friction/drag within the system opposed to the proportional component.
    output += gain_multiply(proportional_error, proportional_gain);
    output -= gain_multiply(derivative_error, derivative_gain);

    // Get the PID offset.  This value offsets the output to compensate for 
    // static friction and other drag within the servo.  If the offset is
    // not great enough to move the servo the output may not be of sufficient
    // strength to move the motor -- inducing motor vibrations and wasting power.
    output_offset = (int16_t) registers_read_byte(REG_PID_OFFSET);

    // This value determines the true range of the PID values to be the
    // maximum PID value minus the PID offset.
    output_range = MAX_OUTPUT - output_offset;

    // Clear the integral error if output is already saturated.
    if (output > output_range) integral_error = 0;
    if (output < -output_range) integral_error = 0;

    // Add the integral error to the PID output.
    output += gain_multiply(integral_error, integral_gain);

    // Apply the PID offset adjustment to compensate for static friction within the servo.
    if (output > 0) output += output_offset;
    if (output < 0) output -= output_offset;

    // Check for output saturation.
    if (output > MAX_OUTPUT)
    {
        // Can't go higher than the maximum output value.
        output = MAX_OUTPUT;
    }
    else if (output < MIN_OUTPUT)
    {
        // Can't go lower than the minimum output value.
        output = MIN_OUTPUT;
    }
    else
    {
        // Sum the proportional errors into the integral error.
        integral_error += proportional_error;

        // Prevent unreasonable integral wind-up.
        if (integral_error > MAX_INTEGRAL_ERROR) integral_error = MAX_INTEGRAL_ERROR;
        if (integral_error < MIN_INTEGRAL_ERROR) integral_error = MIN_INTEGRAL_ERROR;
    }

    // Return the PID output.
    return output;
}

#endif // PID_MOTION_ENABLED
