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
#include "motion.h"

#if CURVE_MOTION_ENABLED

// Local types.
typedef struct motion_key
{
    uint16_t delta;
    float position;
    float in_velocity;
    float out_velocity;
} motion_key;


// Exported variables.
uint8_t motion_enable;
uint8_t motion_head;
uint8_t motion_tail;
uint32_t motion_counter;
uint32_t motion_duration;

// Local variables.
static motion_key keys[MOTION_BUFFER_SIZE];

static float
int_to_float(int16_t a)
// 16bit unsigned integer to float.
{
    return (float) a;
}


static int16_t
float_to_int(float a)
// Float to 6:10 signed fixed.
{
    return (int16_t) (a + 0.5);
}


static float
fixed_to_float(int16_t a)
// 6:10 signed fixed point to float.
{
    return ((float) a) / 1024.0;
}


static int16_t
float_to_fixed(float a)
// Float to 6:10 signed fixed.
{
    return (int16_t) (a * 1024.0);
}


void
motion_init(int16_t position)
// Initialize the curve buffer.
{
    // Start out disabled.
    motion_enable = 0;

    // Initialize the counter.
    motion_counter = 0;

    // Initialize the duration.
    motion_duration = 0;

    // Initialize the queue.
    motion_head = 0;
    motion_tail = 0;

    // Initialize the keypoint.
    keys[0].delta = 0;
    keys[0].position = int_to_float(position);
    keys[0].in_velocity = 0.0;
    keys[0].out_velocity = 0.0;

    // Initialize an empty hermite curve.  This is a degenerate case for the hermite
    // curve that will always return the position of the curve without velocity.
    curve_init(0, 0, position, position, 0, 0);

    // XXX Update the space available in the buffer.
}


void
motion_reset(int16_t position)
// Reset the motion buffer.  The enabled state is preserved.
{
    // Save the enabled state.
    uint8_t enable = motion_enable;

    // Initialize the curve buffer.
    motion_init(position);

    // Restore the enabled state.
    motion_enable = enable;
}


uint8_t motion_get_buffer_left(void)
// Get the remaining space available in the buffer.
{
    uint8_t space_left;

    // Determine the points left to store curve data.
    if (motion_head < motion_tail)
    {
        space_left = (MOTION_BUFFER_SIZE - 1) - (MOTION_BUFFER_SIZE + motion_head - motion_tail);
    }
    else
    {
        space_left = (MOTION_BUFFER_SIZE - 1) - (motion_head - motion_tail);
    }

    return space_left;
}


uint8_t
motion_append(uint16_t delta, int16_t position, int16_t in_velocity, int16_t out_velocity)
// Append a new keypoint to the motion buffer offset from the previous curve by the
// specified delta.  An error is returned if there is no more room to store the
// new keypoint in the buffer or if the delta is less than one (a zero delta is not
// allowed).
{
    // Get the next position.
    uint8_t next = (motion_head + 1) & MOTION_BUFFER_MASK;

    // Return error if we have looped the head to the tail.
    if (next == motion_tail) return 0;

    // Keypoint delta must be greater than zero.
    if (delta < 1) return 0;

    // Fill in the next keypoint.
    keys[next].delta = delta;
    keys[next].position = int_to_float(position);
    keys[next].in_velocity = fixed_to_float(in_velocity);
    keys[next].out_velocity = fixed_to_float(out_velocity);

    // Is this keypoint being added to an empty buffer?
    if (motion_tail == motion_head)
    {
        // Initialize a new hermite curve that gets us from the current position to the new position.
        // We use a velocity of zero at each end to smoothly transition from one to the other.
        curve_init(0, delta, curve_get_p1(), keys[next].position, 0.0, 0.0);
    }

    // Increase the duration of the buffer.
    motion_duration += delta;

    // Set the new head index.
    motion_head = next;

    // XXX Update the space available in the buffer.

    return 1;
}


uint8_t
motion_next(uint16_t delta, int16_t *position, int16_t *velocity)
// Increment the buffer counter by the indicated delta and return the position
// and velocity from the buffered curves.  If the delta is zero the current
// position and velocity is returned.
{
    float fposition;
    float fvelocity;

    // Just return if we are not enabled or the delta is less than zero.
    if (!motion_enable || (delta < 0)) return 0;

    // Are we processing an empty curve?
    if (motion_tail == motion_head)
    {
        // Yes. Keep the counter and duration at zero.
        motion_counter = 0;
        motion_duration = 0;
    }
    else
    {
        // Increment the counter.
        motion_counter += delta;

        // Have we exceeded the duration of the currently buffered curve?
        while (motion_counter > curve_get_duration())
        {
            // Reduce the buffer counter by the currently buffered curve duration.
            motion_counter -= curve_get_duration();

            // Reduce the buffer duration by the currently buffered curve duration.
            motion_duration -= curve_get_duration();

            // Increment the tail to process the next buffered curve.
            motion_tail = (motion_tail + 1) & MOTION_BUFFER_MASK;

            // Has the tail caught up with the head?
            if (motion_tail == motion_head)
            {
                // Initialize an empty hermite curve with a zero duration.  This is a degenerate case for
                // the hermite cuve that will always return the position of the curve without velocity.
                curve_init(0, 0, keys[motion_head].position, keys[motion_head].position, 0.0, 0.0);

                // Reset the buffer counter and duration to zero.
                motion_counter = 0;
                motion_duration = 0;
            }
            else
            {
                uint8_t curr_point;
                uint8_t next_point;

                // Get the current point and next point for the curve.
                curr_point = motion_tail;
                next_point = (curr_point + 1) & MOTION_BUFFER_MASK;

                // Initialize the hermite curve from the current and next point.
                curve_init(0, keys[next_point].delta,
                           keys[curr_point].position, keys[next_point].position,
                           keys[curr_point].out_velocity, keys[next_point].in_velocity);
            }

            // XXX Update the space available in the buffer.
        }
    }

    // Get the position and velocity from the hermite curve.
    curve_solve(motion_counter, &fposition, &fvelocity);

    // The velocity is in position units a millisecond, but we really need the
    // velocity to be measured in position units every 10 milliseconds to match
    // the sample period of the ADC.
    fvelocity *= 10.0;

    // Convert position to integer and velocity to fixed.
    *position = float_to_int(fposition);
    *velocity = float_to_fixed(fvelocity);

    return 1;
}

#endif // CURVE_MOTION_ENABLED

