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

// Values preserved across multiple PID iterations.
static int16_t previous_seek;
static int16_t previous_position;

//
// Digital Lowpass Filter Implementation
//
// See: A Simple Software Lowpass Filter Suits Embedded-system Applications
// http://www.edn.com/article/CA6335310.html
//
// k    Bandwidth (Normalized to 1Hz)   Rise Time (samples)
// 1    0.1197                          3
// 2    0.0466                          8
// 3    0.0217                          16
// 4    0.0104                          34
// 5    0.0051                          69
// 6    0.0026                          140
// 7    0.0012                          280
// 8    0.0007                          561
//

#define FILTER_SHIFT 1

static int32_t filter_reg = 0;

static int16_t filter_update(int16_t input)
{
    // Update the filter with the current input.
    filter_reg = filter_reg - (filter_reg >> FILTER_SHIFT) + input;

    // Scale output for unity gain.
    return (int16_t) (filter_reg >> FILTER_SHIFT);
}

static int16_t gain_multiply(int16_t error, uint16_t gain)
// Multiplies the PID error value by the PID gain value.
// The result is scaled and bounds checked to fit within
// a signed sixteen bit return value.
{
    int32_t result;

    // Multiply the error times the gain.
    result = (int32_t) error * (int32_t) gain;

    // Shift by 8 to account for fixed point gain.
    result >>= 8;

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
    previous_seek = 0;
    previous_position = 0;
}


void pid_registers_defaults(void)
// Initialize the PID algorithm related register values.  This is done 
// here to keep the PID related code in a single file.  
{
    // Default offset value.
    registers_write_byte(REG_PID_OFFSET, 0x30);

    // Default gain values.
    registers_write_word(REG_PID_PGAIN_HI, REG_PID_PGAIN_LO, DEFAULT_PID_PGAIN);
    registers_write_word(REG_PID_DGAIN_HI, REG_PID_DGAIN_LO, DEFAULT_PID_DGAIN);
    registers_write_word(REG_PID_IGAIN_HI, REG_PID_IGAIN_LO, DEFAULT_PID_IGAIN);

    // Default position limits.
    registers_write_word(REG_MIN_SEEK_HI, REG_MIN_SEEK_LO, DEFAULT_MIN_SEEK);
    registers_write_word(REG_MAX_SEEK_HI, REG_MAX_SEEK_LO, DEFAULT_MAX_SEEK);

    // Default reverse seek setting.
    registers_write_byte(REG_REVERSE_SEEK, 0x00);
}


int16_t pid_position_to_pwm(int16_t current_position)
// This is a modified pid algorithm by which the seek position and seek
// velocity are assumed to be a moving target.  The algorithm attempts to
// output a pwm value that will achieve a predicted position and velocity.
{
    int16_t output;
    int16_t output_minimum;
    int16_t current_velocity;
    int16_t filtered_position;
    int16_t seek_position;
    int16_t seek_velocity;
    int16_t minimum_position;
    int16_t maximum_position;
    int16_t p_component;
    int16_t d_component;
    uint16_t d_gain;
    uint16_t p_gain;

    // Filter the current position thru a digital low-pass filter.
    filtered_position = filter_update(current_position);

    // Use the filtered position to determine velocity.
    current_velocity = filtered_position - previous_position;
    previous_position = filtered_position;

    // Get the seek position and velocity.
    seek_position = (int16_t) registers_read_word(REG_SEEK_POSITION_HI, REG_SEEK_POSITION_LO);
    seek_velocity = (int16_t) registers_read_word(REG_SEEK_VELOCITY_HI, REG_SEEK_VELOCITY_LO);

    // Get the minimum and maximum position.
    minimum_position = (int16_t) registers_read_word(REG_MIN_SEEK_HI, REG_MIN_SEEK_LO);
    maximum_position = (int16_t) registers_read_word(REG_MAX_SEEK_HI, REG_MAX_SEEK_LO);

    // Are we reversing the seek sense?
    if (registers_read_byte(REG_REVERSE_SEEK) != 0)
    {
        // Yes. Update the position and velocity using reverse sense.
        registers_write_word(REG_POSITION_HI, REG_POSITION_LO, (uint16_t) (MAX_POSITION - current_position));
        registers_write_word(REG_VELOCITY_HI, REG_VELOCITY_LO, (uint16_t) -current_velocity);

        // Reverse sense the seek and other position values.
        seek_position = MAX_POSITION - seek_position;
        minimum_position = MAX_POSITION - minimum_position;
        maximum_position = MAX_POSITION - maximum_position;
    }
    else
    {
        // No. Update the position and velocity registers without change.
        registers_write_word(REG_POSITION_HI, REG_POSITION_LO, (uint16_t) current_position);
        registers_write_word(REG_VELOCITY_HI, REG_VELOCITY_LO, (uint16_t) current_velocity);
    }

    // Use the filtered position when the seek position is not changing.
    if (seek_position == previous_seek) current_position = filtered_position;
    previous_seek = seek_position;

    // Keep the seek position bound within the minimum and maximum position.
    if (seek_position < minimum_position) seek_position = minimum_position;
    if (seek_position > maximum_position) seek_position = maximum_position;

    // The proportional component to the PID is the position error.
    p_component = seek_position - current_position;

    // The derivative component to the PID is the velocity.
    d_component = seek_velocity - current_velocity;

    // Get the proportional, derivative and integral gains.
    p_gain = registers_read_word(REG_PID_PGAIN_HI, REG_PID_PGAIN_LO);
    d_gain = registers_read_word(REG_PID_DGAIN_HI, REG_PID_DGAIN_LO);

    // Determine the proportional and derivative components of the pwm output.
    output = gain_multiply(p_component, p_gain);
    output += gain_multiply(d_component, d_gain);

    // Get the PID offset.  This value offsets the output to compensate for
    // static friction and other drag within the servo.  If the offset is
    // not great enough to move the servo the output may not be of sufficient
    // strength to move the motor -- inducing motor vibrations and wasting power.
    output_minimum = (int16_t) registers_read_byte(REG_PID_OFFSET);

    // Apply the PID offset adjustment to compensate for static friction within the servo.
    if (output > 0) output += output_minimum;
    else if (output < 0) output -= output_minimum;

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

    // Return the PID output.
    return output;
}

#endif // PID_MOTION_ENABLED
