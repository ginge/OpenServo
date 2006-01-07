/*
   Copyright (c) 2005, Mike Thompson <mpthompson@gmail.com>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

// The following is needed until WINAVR supports the ATtinyX5 MCUs.
#undef __AVR_ATtiny2313__
#define __AVR_ATtiny45__

#include <inttypes.h>

#include "motion.h"
#include "registers.h"

// The minimum and maximum servo position.
#define MIN_POSITION			(0)
#define MAX_POSITION			(1023)

// The minimum and maximum output.
#define MAX_OUTPUT				(255)
#define MIN_OUTPUT				(-MAX_OUTPUT)

// Static structure for managing position and velocity values.
static uint8_t velocity_index;
static int16_t velocity_array[8];
static int16_t previous_position;

// The accumulator is a structure arranged so that both the most significant
// and least significant words can be indepently accessed.  The accumulator
// maintains 32-bit resolution, but only 16-bit is needed for output.
static union
{
	struct
	{
		int16_t	lo;
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

#if 0
static inline int16_t shift_right(int16_t val, uint8_t cnt)
// Arithmetic shift right of the signed 16-bit value.
{
    asm volatile (
        "L_asr1%=:" "\n\t"
		"cp %1,__zero_reg__" "\n\t"
		"breq L_asr2%=" "\n\t"
        "dec %1" "\n\t"
		"asr %B0" "\n\t"
		"ror %A0" "\n\t"
        "rjmp L_asr1%=" "\n\t"
        "L_asr2%=:" "\n\t"
        : "=&r" (val)
        : "r" (cnt), "0" (val)
        );

	return val;
}
#endif

static inline int16_t integral_accumulator_get(void)
// This function returns the most significant word of the integral gain.
{
	return integral_accumulator.small.hi;
}


static inline void integral_accumulator_update(int16_t command_error, uint16_t fixed_gain)
// This function updates the integral accumulator with the command error.  
// The fixed gain scales the integral accumulator 
{
	int32_t temp;

	// Multiply the command error by the fixed gain value.
	temp = (int32_t) command_error * (int32_t) fixed_gain;

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
// diagram below should give a picture of how it is intended to work.
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
//
//  Apply no gain to the integral output
//
{
	uint8_t i;
	int16_t current_velocity;
	int16_t command_position;
	int16_t command_error;
	int16_t output_range;
	int16_t output_offset;
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
	command_position = registers_read_word(SEEK_HI, SEEK_LO);

	// Get the positional, velocity and integral gains from the registers.
	position_gain = registers_read_word(PID_PGAIN_HI, PID_PGAIN_LO);
	velocity_gain = registers_read_word(PID_DGAIN_HI, PID_DGAIN_LO);
	integral_gain = registers_read_word(PID_IGAIN_HI, PID_IGAIN_LO);

	// Get the output offset from the registers.  This value offsets the output 
	// to compensate for static friction and other drag within the servo.
	output_offset = (int16_t) registers_read_byte(PID_OFFSET);

	// Sanity check the command position. We do this because this value is
	// passed from the outside to the servo and it could be an invalid value.
	if (command_position < MIN_POSITION) command_position = MIN_POSITION;
	if (command_position > MAX_POSITION) command_position = MAX_POSITION;

	// Are we reversing the seek sense?
	if (registers_read_byte(REVERSE_SEEK) != 0)
	{
		// Yes. Update the system registers with an adjusted reverse sense
		// position. With reverse sense, the position value to grows from 
		// a low value to high value in the clockwise direction.
		registers_write_word(POSITION_HI, POSITION_LO, (uint16_t) (MAX_POSITION - current_position));

		// Adjust command position for the reverse sense.
		command_position = MAX_POSITION - command_position;
	}
	else
	{
		// No. Update the system registers with a non-reverse sense position.
		// Normal position value grows from a low value to high value in the
		// counter-clockwise direction.
		registers_write_word(POSITION_HI, POSITION_LO, (uint16_t) current_position);
	}

	// The command error is the difference between the
	// command position and current position.
	command_error = command_position - current_position;

	// Get the integral output.  There is no gain applied to the integral output.
	integral_output = integral_accumulator_get();

	// Determine the position output component.  We multiply the position gain
	// by the current position of the servo to create the position output.
	position_output = fixed_multiply(current_position, position_gain);

	// Determine the velocity output component.  We multiply the velocity gain
	// by the current velocity of the servo to create the velocity output.
	velocity_output = fixed_multiply(current_velocity, velocity_gain);

	registers_write_word(RESERVED_0A, RESERVED_0B, (uint16_t) position_output);
	registers_write_word(RESERVED_0C, RESERVED_0D, (uint16_t) velocity_output);
	registers_write_word(RESERVED_0E, RESERVED_0F, (uint16_t) integral_output);

	// Add the command error scaled by the position gain to the integral accumulator.
	// The integral accumulator maintains a sum of total error over each interation.
	integral_accumulator_update(command_error, integral_gain);

	// The integral output drives the output and the position and velocity outputs
	// function as a frictional component to counter the integral output.
	output = integral_output - position_output - velocity_output;

	// Determine the output range which factors out the output offset.
	output_range = MAX_OUTPUT - output_offset;

	// Is the output saturated? If so we need limit the output and clip
	// the integral accumulator just at the saturation level.
	if (output < -output_range)
	{
		// Calculate a new integral accumulator based on the output range.  This value
		// is calculated to keep the integral output just on the verge of saturation.
		integral_accumulator_reset(position_output + velocity_output - output_range);

		// Limit the output.
		output = -output_range;
	}
	else if (output > output_range)
	{
		// Calculate a new integral accumulator based on the output range.  This value
		// is calculated to keep the integral output just on the verge of saturation.
		integral_accumulator_reset(position_output + velocity_output + output_range);

		// Limit the output.
		output = output_range;
	}

	// Apply the output offset adjustment.
	if (output > 0) output += output_offset;
	if (output < 0) output -= output_offset;

	// Return the output.
	return output;
}

