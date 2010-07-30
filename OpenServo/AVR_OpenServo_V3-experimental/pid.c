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

// The minimum and maximum servo position as defined by 10-bit ADC values.
#define MIN_POSITION            (0)
#if ENCODER_ENABLED
#define MAX_POSITION            (4095)
#else
#define MAX_POSITION            (1023)
#endif

// The minimum and maximum output.
#define MAX_OUTPUT              (255)
#define MIN_OUTPUT              (-MAX_OUTPUT)

// Values preserved across multiple PID iterations.
static int16_t previous_seek=-1; // NOTE: previous_seek==-1 used to indicate initialisation required
static int16_t previous_position=0;
static int16_t i_component=0;
static int16_t seek_delta=-1;
static int16_t position_at_start_of_new_seek=-1;
static uint8_t previous_pwm_is_enabled=0;

#if FULL_ROTATION_ENABLED
static int16_t normalize_position_difference(int16_t posdiff)
{
    if (posdiff > ((MAX_POSITION - MIN_POSITION) / 2))
    {
        posdiff -= (MAX_POSITION - MIN_POSITION);
    }

    if (posdiff < -((MAX_POSITION - MIN_POSITION) / 2))
    {
        posdiff += (MAX_POSITION - MIN_POSITION);
    }

    return posdiff;
}
#endif


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
#if 0
    // Update the filter with the current input.
#if FULL_ROTATION_ENABLED
    filter_reg += normalize_position_difference(input - (filter_reg >> FILTER_SHIFT));
#else
    filter_reg = filter_reg - (filter_reg >> FILTER_SHIFT) + input;
#endif

    // Scale output for unity gain.
    return (int16_t) (filter_reg >> FILTER_SHIFT);
#else
    return input;
#endif
}

void pid_init(void)
// Initialize the PID algorithm module.
{
   previous_seek = -1;
}


void pid_registers_defaults(void)
// Initialize the PID algorithm related register values.  This is done
// here to keep the PID related code in a single file.
{
    // Default deadband.
    registers_write_byte(REG_PID_DEADBAND, DEFAULT_PID_DEADBAND);

    // Default gain values.
    registers_write_word(REG_PID_PGAIN_HI, REG_PID_PGAIN_LO, DEFAULT_PID_PGAIN);
    registers_write_word(REG_PID_IGAIN_HI, REG_PID_IGAIN_LO, DEFAULT_PID_IGAIN);
    registers_write_word(REG_PID_DGAIN_HI, REG_PID_DGAIN_LO, DEFAULT_PID_DGAIN);

    // Default position limits.
    registers_write_word(REG_MIN_SEEK_HI, REG_MIN_SEEK_LO, DEFAULT_MIN_SEEK);
    registers_write_word(REG_MAX_SEEK_HI, REG_MAX_SEEK_LO, DEFAULT_MAX_SEEK);

    // Default reverse seek setting.
    registers_write_byte(REG_REVERSE_SEEK, 0x00);
}


int16_t pid_position_to_pwm(int16_t current_position, uint8_t tick)
// This is a modified pid algorithm by which the seek position and seek
// velocity are assumed to be a moving target.  The algorithm attempts to
// output a pwm value that will achieve a predicted position and velocity.
{
    // We declare these static to keep them off the stack.
    static int16_t deadband;
    static int16_t p_component;
    static int16_t d_component;
    static int16_t seek_position;
    static int16_t seek_velocity;
    static int16_t minimum_position;
    static int16_t maximum_position;
    static int16_t current_velocity;
    static int16_t filtered_position;
    static int32_t pwm_output;
    static uint16_t p_gain;
    static uint16_t i_gain;
    static uint16_t d_gain;
    static uint8_t pwm_is_enabled;

//
// Get flag indicating if PWM is enabled or not. This saves fetching it each time
// we want to know.
//
    pwm_is_enabled=registers_read_byte(REG_FLAGS_LO)&(1<<FLAGS_LO_PWM_ENABLED);

    // Filter the current position thru a digital low-pass filter.
    filtered_position = filter_update(current_position);

    // Use the filtered position to determine velocity.
#if FULL_ROTATION_ENABLED
    current_velocity = normalize_position_difference(filtered_position - previous_position);
#else
    current_velocity = filtered_position - previous_position;
#endif

    // Get the seek position and velocity.
    seek_position = (int16_t) registers_read_word(REG_SEEK_POSITION_HI, REG_SEEK_POSITION_LO);
    seek_velocity = (int16_t) registers_read_word(REG_SEEK_VELOCITY_HI, REG_SEEK_VELOCITY_LO);

    // Get the minimum and maximum position.
    minimum_position = (int16_t) registers_read_word(REG_MIN_SEEK_HI, REG_MIN_SEEK_LO);
    maximum_position = (int16_t) registers_read_word(REG_MAX_SEEK_HI, REG_MAX_SEEK_LO);

    // Are we reversing the seek sense?
// TODO: What is the point of this? Surely it is better to correct the wires to the motor than
//       risk accidents?
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

    // Get the deadband.
    deadband = (int16_t) registers_read_byte(REG_PID_DEADBAND);

//
// Keep the seek position bound within the set minimum and maximum position and the hardware
// defined limits.
//
// NOTE: Even when full rotation is enabled, the limit of the set min and max
//       positions are still applied: these must be set appropriately to allow
//       full rotation to occur.
//
//       TODO: When full rotation is enabled, it may be that the user desires the
//             range to be exclusive rather than inclusive.
//
    if (seek_position < minimum_position) seek_position = minimum_position;
    if (seek_position > maximum_position) seek_position = maximum_position;
    if (seek_position < MIN_POSITION) seek_position = MIN_POSITION;
    if (seek_position > MAX_POSITION) seek_position = MAX_POSITION;

//
// Check for new seek target
//
    if(previous_seek != seek_position ||             // New seek position has been set...
       previous_pwm_is_enabled != pwm_is_enabled)    // PWM enable state has changed...
    { 
       if(previous_seek == -1)                       // Initialisation
       {
          previous_position = current_position;
          i_component = 0;
       }
       previous_seek = seek_position;
       seek_delta = current_position;
       position_at_start_of_new_seek = current_position;
       previous_pwm_is_enabled = pwm_is_enabled;
    }

//
// Update seek target
//
    if(tick && seek_delta!=seek_position) // Tick is our time constant
    {
       if(position_at_start_of_new_seek<seek_position)
       {
          seek_delta+=seek_velocity;
          if(seek_delta>=seek_position)
          {
             seek_delta=seek_position;
          }
       } else
       {
          if(position_at_start_of_new_seek>seek_position)
          {
             seek_delta-=seek_velocity;
             if(seek_delta<=seek_position)
             {
                seek_delta=seek_position;
             }
          }
       }
    }
    if(seek_delta==seek_position)
    {
       current_position = filtered_position;
    }

//
// Calculate PWM
//
#if FULL_ROTATION_ENABLED
    p_component = normalize_position_difference(seek_delta - current_position);
#else
    // The proportional component to the PID is the position error.
    p_component = seek_delta - current_position;
#endif

    // The integral component
    if(tick) // Tick is our time constant
    {
       i_component += p_component;
       if(i_component<-128) // Somewhat arbitrary anti integral wind-up; we're experimenting
       {
          i_component=-128;
       } else
       {
          if(i_component>128)
          {
             i_component=128;
          }
       }
    }

    // The derivative component to the PID is the change in position.
    d_component = previous_position - current_position;
    previous_position = current_position;

    // Get the proportional, derivative and integral gains.
    p_gain = registers_read_word(REG_PID_PGAIN_HI, REG_PID_PGAIN_LO);
    i_gain = registers_read_word(REG_PID_IGAIN_HI, REG_PID_IGAIN_LO);
    d_gain = registers_read_word(REG_PID_DGAIN_HI, REG_PID_DGAIN_LO);

    // Start with zero PWM output.
    pwm_output = 0;

    // Apply proportional component to the PWM output if outside the deadband.
    if ((p_component > deadband) || (p_component < -deadband))
    {
        // Apply the proportional component of the PWM output.
        pwm_output += (int32_t) p_component * (int32_t) p_gain;

    // Apply the integral component of the PWM output.
        pwm_output += (int32_t) i_component * (int32_t) i_gain;

    // Apply the derivative component of the PWM output.
        pwm_output += (int32_t) d_component * (int32_t) d_gain;
    } else
    {
       i_component = 0;
    }

    // Shift by 8 to account for the multiply by the 8:8 fixed point gain values.
    // NOTE: When OpenEncoder is enabled an extra 1 place of shift is applied
    //       to account for the increased precision (which is approximately a
    //       factor of 2) so that the magnitude of the gain values are similar
    //       across the two different platforms.
    //
    //       Factor of 2: pot measurements are typically approaching 180 degrees
    //       across the 0 to 1023 ADC range. OpenEncoder is 360 degrees across the
    //       0 to 4096 range.
    //
#if ENCODER_ENABLED
    pwm_output >>= 9;
#else
    pwm_output >>= 8;
#endif

    // Check for output saturation.
    if (pwm_output > MAX_OUTPUT)
    {
        // Can't go higher than the maximum output value.
        pwm_output = MAX_OUTPUT;
    }
    else if (pwm_output < MIN_OUTPUT)
    {
        // Can't go lower than the minimum output value.
        pwm_output = MIN_OUTPUT;
    }

    // Return the PID output.
    return (int16_t) pwm_output;
}

