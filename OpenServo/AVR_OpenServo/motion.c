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
#include "motion.h"
#include "registers.h"

// Compile following for IPD motion control algorithm.
#if MOTION_IPD_ENABLED

// The minimum and maximum servo position.
#define MIN_POSITION            (0)
#define MAX_POSITION            (1023)

// The minimum and maximum output.
#define MAX_OUTPUT              (255)
#define MIN_OUTPUT              (-MAX_OUTPUT)

// Static structure for managing position and velocity values.
static uint8_t velocity_index;
static int16_t velocity_array[8];
static int16_t previous_position;

// The accumulator is a structure arranged so that both the most significant
// and least significant words can be indepently accessed.  The accumulator
// maintains 32-bit resolution, but only 16-bits are needed for output.
static union
{
    struct
    {
        int16_t lo;
        int16_t hi;
    } small;
    int32_t big;
} integral_accumulator;


static int16_t fixed_multiply(int16_t component, uint16_t fixed_gain)
// Multiplies the signed 16-bit component by the unsigned 16-bit fixed
// point gain to return a 32-bit output. The result is scaled to account
// for the fixed point value.
{
    int32_t output;

    // Multiply the value by the fixed gain value.
    output = (int32_t) component * (int32_t) fixed_gain;

    // Scale result by 256 to account for fixed point gain.
    output /= 256;

    return (int16_t) output;
}


static inline int16_t integral_accumulator_get(void)
// This function returns the most significant word of the integral gain.
{
    return integral_accumulator.small.hi;
}


static inline void integral_accumulator_update(int16_t command_error, uint16_t integral_gain)
// This function updates the integral accumulator with the command error.
// Note: The command error is maintained with 32 bit precision, although 
// only the upper 16 bits are used for output calculations.
{
    int32_t temp;

    // Multiply the command error by the fixed integral gain value.
    temp = (int32_t) command_error * (int32_t) integral_gain;

    // Add to the accumulator adjusting for multiplication.
    integral_accumulator.big += temp;
}


static inline void integral_accumulator_reset(int16_t new_value)
// This function resets the integral accumulator to a new value.
{
    integral_accumulator.small.hi = new_value;
    integral_accumulator.small.lo = 0;
}


void motion_init(void)
// Initialize the motion module.
{
    uint8_t i;

    // Initialize the velocity index.
    velocity_index = 0;

    // Initialize the velocity array.
    for (i = 0; i < 8; ++i) velocity_array[i] = 0;

    // Initialize accumulator.
    integral_accumulator.big = 0;
}


void motion_registers_defaults(void)
// Initialize the motion related register values.  This is done here to
// keep the motion related code in a single file.  
{
    // Default gain values.
    registers_write_word(REG_PID_PGAIN_HI, REG_PID_PGAIN_LO, 0x0800);
    registers_write_word(REG_PID_DGAIN_HI, REG_PID_DGAIN_LO, 0x2800);
    registers_write_word(REG_PID_IGAIN_HI, REG_PID_IGAIN_LO, 0x0B80);

    // Default position limits.
    registers_write_word(REG_MIN_SEEK_HI, REG_MIN_SEEK_LO, 0x0060);
    registers_write_word(REG_MAX_SEEK_HI, REG_MAX_SEEK_LO, 0x03A0);

    // Default reverse seek setting.
    registers_write_byte(REG_REVERSE_SEEK, 0x00);
}


int16_t motion_position_to_pwm(int16_t current_position)
// This function takes the current servo position as input and outputs a pwm
// value for the servo motors.  The current position value must be within the
// range 0 and 1023. The output will be within the range of -255 and 255 with
// values less than zero indicating clockwise rotation and values more than
// zero indicating counter-clockwise rotation.
//
// The feedback approach implemented here was first published in Richard Phelan's
// Automatic Control Systems, Cornell University Press, 1977 (ISBN 0-8014-1033-9)
//
// The theory of operation of this function will be filled in later, but the
// diagram below should gives a general picture of how it is intended to work.
//
//
//                           +<------- bounds checking -------+
//                           |                                |
//             |¯¯¯¯¯|   |¯¯¯¯¯¯¯¯|   |¯¯¯¯¯|   |¯¯¯¯¯¯¯¯¯|   |
//  command -->|  -  |-->|integral|-->|  -  |-->|  motor  |-->+-> actuator
//             |_____|   |________|   |_____|   |_________|   |
//                |                      |                    |
//                |                      +<-- Kv * velocity --+
//                |                      |                    |
//                |                      +<-- Kp * position --+
//                |                                           |
//                +<-------------Ki * position ---------------+
//
//
{
    uint8_t i;
    int16_t command_position;
    int16_t maximum_position;
    int16_t minimum_position;
    int16_t current_velocity;
    int16_t command_error;
    int16_t output;
    int16_t position_output;
    int16_t velocity_output;
    int16_t integral_output;
    uint16_t position_gain;
    uint16_t velocity_gain;
    uint16_t integral_gain;

    // Determine the velocity as the difference between the current and previous position.
    velocity_array[velocity_index] = current_position - previous_position;

    // Increment the velocity index, but wrap if bounds exceeded necessary.
    velocity_index += 1;
    velocity_index &= 7;

    // Reset the velocity value.
    current_velocity = 0;

    // Determine the sum of velocities across the positions.
    for (i = 0; i < 8; ++i) current_velocity += velocity_array[i];

    // Update the previous position.
    previous_position = current_position;

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

    // The command error is the difference between the command position and current position.
    command_error = command_position - current_position;

    // Add a bit of deadband into the command error to minimize twitching.  The 
    // potentiometer readings are a bit noisy and there is typically one or two
    // units of difference from reading to reading when the servo is holding 
    // position.  Adding deadband decreases some of the twitchiness in the servo
    // caused by this noise.
    if (command_error == 1) command_error = 0;
    if (command_error == -1) command_error = 0;

    // Get the positional, velocity and integral gains from the registers.
    position_gain = registers_read_word(REG_PID_PGAIN_HI, REG_PID_PGAIN_LO);
    velocity_gain = registers_read_word(REG_PID_DGAIN_HI, REG_PID_DGAIN_LO);
    integral_gain = registers_read_word(REG_PID_IGAIN_HI, REG_PID_IGAIN_LO);

    // Add the command error scaled by the position gain to the integral accumulator.
    // The integral accumulator maintains a sum of total error over each iteration.
    integral_accumulator_update(command_error, integral_gain);

    // Get the integral output. There is no gain applied to the integral output.
    integral_output = integral_accumulator_get();

    // Determine the position output component.  We multiply the position gain
    // by the current position of the servo to create the position output.
    position_output = fixed_multiply(current_position, position_gain);

    // Determine the velocity output component.  We multiply the velocity gain
    // by the current velocity of the servo to create the velocity output.
    velocity_output = fixed_multiply(current_velocity, velocity_gain);

    // registers_write_word(REG_RESERVED_18, REG_RESERVED_19, (uint16_t) position_output);
    // registers_write_word(REG_RESERVED_1A, REG_RESERVED_1B, (uint16_t) velocity_output);
    // registers_write_word(REG_RESERVED_1C, REG_RESERVED_1D, (uint16_t) integral_output);

    // The integral output drives the output and the position and velocity outputs
    // function as a frictional component to counter the integral output.
    output = integral_output - position_output - velocity_output;

    // Is the output saturated? If so we need limit the output and clip
    // the integral accumulator just at the saturation level.
    if (output < MIN_OUTPUT)
    {
        // Calculate a new integral accumulator based on the output range.  This value
        // is calculated to keep the integral output just on the verge of saturation.
        integral_accumulator_reset(position_output + velocity_output + MIN_OUTPUT);

        // Limit the output.
        output = MIN_OUTPUT;
    }
    else if (output > MAX_OUTPUT)
    {
        // Calculate a new integral accumulator based on the output range.  This value
        // is calculated to keep the integral output just on the verge of saturation.
        integral_accumulator_reset(position_output + velocity_output + MAX_OUTPUT);

        // Limit the output.
        output = MAX_OUTPUT;
    }

    // Return the output.
    return output;
}

#endif // MOTION_IPD_ENABLED

// Compile following for PID motion control algorithm.
#if MOTION_PID_ENABLED

// The minimum and maximum positions are defined by 10-bit ADC values.
#define MIN_POSITION        (0)
#define MAX_POSITION        (1023)

// Defines for maximum and minimum integral error.  These values
// are used to keep the integral error from winding up or down 
// to unreasonable values.
#define MAX_INTEGRAL_ERROR  (4000)
#define MIN_INTEGRAL_ERROR  (-4000)

// The minimum and maximum output.
#define MAX_OUTPUT              (255)
#define MIN_OUTPUT              (-MAX_OUTPUT)

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


void motion_init(void)
// Initialize the motion module.
{
    // Initialize preserved values.
    integral_error = 0;
    previous_position = 0;
}


void motion_registers_defaults(void)
// Initialize the motion related register values.  This is done here to
// keep the motion related code in a single file.  
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


int16_t motion_position_to_pwm(int16_t current_position)
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

#endif // MOTION_PID_ENABLED
