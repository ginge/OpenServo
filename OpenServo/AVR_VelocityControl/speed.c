/*
    Copyright (c) 2008 Stefan Engelke <stefan@tinkerer.eu>

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
#include "speed.h"
#include "registers.h"
#include "banks.h"

// The minimum and maximum output.
#define MAX_OUTPUT              (255)
#define MIN_OUTPUT              (-MAX_OUTPUT)

static int32_t i_component;

void speed_init(void)
// Initialize the PID algorithm module.
{
    // Reset the integrator of the i_component to zero
    i_component = 0;
}


void speed_registers_defaults(void)
// Initialize the automatic control algorithm related register values.  This is done
// here to keep the automatic control related code in a single file.
{
    // Default deadband.
    banks_write_byte(POS_PID_BANK, REG_PID_DEADBAND, DEFAULT_PID_DEADBAND);

    // Default gain values.
    banks_write_word(POS_PID_BANK, REG_PID_PGAIN_HI, REG_PID_PGAIN_LO, DEFAULT_PID_PGAIN);
    banks_write_word(POS_PID_BANK, REG_FEED_FORWARD_HI, REG_FEED_FORWARD_LO, DEFAULT_FEED_FORWARD);
    banks_write_word(POS_PID_BANK, REG_PID_IGAIN_HI, REG_PID_IGAIN_LO, DEFAULT_PID_IGAIN);

    banks_write_word(POS_PID_BANK, REG_ANTI_WINDUP_HI, REG_ANTI_WINDUP_LO, DEFAULT_ANTI_WINDUP);

    // Default speed limits.
    banks_write_word(POS_PID_BANK, REG_MAX_SPEED_HI, REG_MAX_SPEED_LO, DEFAULT_MAX_SPEED);

    // Default reverse seek setting.
    banks_write_byte(POS_PID_BANK, REG_REVERSE_SEEK, 0x00);
}


int16_t speed_position_to_pwm(int16_t current_position)
// This is a modified pi algorithm with feed forward by which the seek
// veolcity is assumed to be a moving target. The algorithm attempts to
// output a pwm value that will achieve a predicted velocity.
{
    // We declare these static to keep them off the stack.
    static int16_t p_component;
    static int16_t i_component;
    static int16_t seek_velocity;
    static int16_t current_velocity;
    static int32_t pwm_output;
    static uint16_t i_gain;
    static uint16_t p_gain;
    static int16_t anti_windup;
    static uint16_t feed_forward_gain;
    static int16_t max_speed;

       
    // Get the seek velocity.
    seek_velocity = (int16_t) registers_read_word(REG_SEEK_VELOCITY_HI, REG_SEEK_VELOCITY_LO);

    max_speed = (int16_t)banks_read_word(POS_PID_BANK, REG_MAX_SPEED_HI, REG_MAX_SPEED_LO);

    if (seek_velocity> max_speed) seek_velocity =  max_speed;
    if (seek_velocity<-max_speed) seek_velocity = -max_speed;

    // Get current velocity from measurement of back-emf
    current_velocity = banks_read_word(INFORMATION_BANK, REG_BACKEMF_HI, REG_BACKEMF_LO);
    // Determine rotation sense by last PWM value
    if (registers_read_byte(REG_PWM_DIRA)) current_velocity = - current_velocity;


    // Are we reversing the seek sense?
    if (banks_read_byte(POS_PID_BANK, REG_REVERSE_SEEK) != 0)
    {
        registers_write_word(REG_VELOCITY_HI, REG_VELOCITY_LO, (uint16_t) -current_velocity);
    
    }
    else
    {
        // No. Update the position and velocity registers without change.
        registers_write_word(REG_VELOCITY_HI, REG_VELOCITY_LO, (uint16_t) current_velocity);
    }

    
    // Get the proportional and integral gains.
    p_gain = banks_read_word(POS_PID_BANK, REG_PID_PGAIN_HI, REG_PID_PGAIN_LO);
    i_gain = banks_read_word(POS_PID_BANK, REG_PID_IGAIN_HI, REG_PID_IGAIN_LO);
    feed_forward_gain = banks_read_word(POS_PID_BANK, REG_FEED_FORWARD_HI, REG_FEED_FORWARD_LO);
    anti_windup = (int16_t)banks_read_word(POS_PID_BANK, REG_ANTI_WINDUP_HI, REG_ANTI_WINDUP_LO);


    // The proportional component to the PID is the position error.
    p_component  = seek_velocity - current_velocity;
   
    // Increment the integral component of the PID
    i_component += (int32_t) p_component * (int32_t) i_gain;

    // Saturate the integrator for anti wind-up
    if (i_component >  anti_windup) i_component =  anti_windup;
    if (i_component < -anti_windup) i_component = -anti_windup;

    // Start with zero PWM output.
    pwm_output = 0;
    
    // Apply the feed forward component of the PWM output.
    pwm_output += (int32_t) seek_velocity * (int32_t) feed_forward_gain;

    // Apply the proportional component of the PWM output.
    pwm_output += (int32_t) p_component * (int32_t) p_gain;

    // Apply the integral component of the PWM output.
    pwm_output += i_component;

    // Shift by 8 to account for the multiply by the 8:8 fixed point gain values.
    pwm_output >>= 8;

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
    //return (int16_t) registers_read_word(REG_SEEK_VELOCITY_HI, REG_SEEK_VELOCITY_LO);

}

