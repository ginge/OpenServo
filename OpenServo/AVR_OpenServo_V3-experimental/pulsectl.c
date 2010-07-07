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
#include <avr/io.h>
#include <avr/interrupt.h>

#include "openservo.h"
#include "config.h"
#include "registers.h"
#include "banks.h"
#include "pulsectl.h"
#include "pwm.h"
#include "timer.h"
#include "filter.h"

#if PULSE_CONTROL_ENABLED

// The minimum and maximum servo position as defined by 10-bit ADC values.
#define MIN_POSITION            (0)
#define MAX_POSITION            (1023)

// The timer clock prescaler of 8 is selected to yield a 1MHz ADC clock
// from an 8 MHz system clock.
//#define CSPS        ((0<<CS22) | (1<<CS21) | (0<<CS20))

// Globals used for pulse measurement.
static volatile uint8_t overflow_count;
static volatile uint8_t pulse_flag;
static volatile uint16_t pulse_time;
static volatile uint16_t pulse_duration;

// Hold the running filter value
static int32_t filter_reg_pulse = 0;

void pulse_control_init(void)
// Initialize servo pulse control.
{
    // Initialize the pulse time values.
    pulse_flag = 0;
    pulse_duration = 0;
    overflow_count = 0;

    // Initialize the pulse time.
    pulse_time = timer_get();

    // Set timer/counter2 control register A.
    TCCR2A = (0<<COM2A1) | (0<<COM2A0) |                    // Disconnect OC2A.
             (0<<COM2B1) | (0<<COM2B0) |                    // Disconnect OC2B.
             (0<<WGM21) | (0<<WGM20);                       // Mode 0 - normal operation.

    // Configure PCINT3 to interrupt on change.
    PCMSK0 = (1<<PCINT3);
    PCMSK1 = 0;
    PCMSK2 = 0;
    PCICR = (0<<PCIE2) |
            (0<<PCIE1) |
            (1<<PCIE0);

    // Configure PB0/PCINT3 as an input.
    DDRB &= ~(1<<DDB3);
    PORTB &= ~(1<<PB3);
}


void pulse_control_update(void)
// Update the servo seek position and velocity based on any received servo pulses.
{
    int16_t pulse_position;

    // Did we get a pulse?
    if (pulse_flag)
    {
        // Ignore unusual pulse durations as a sanity check.
        if ((pulse_duration > 500) && (pulse_duration < 2500))
        {
            // Convert the pulse duration to a pulse time.
            pulse_position = pulse_duration - 998;

            // Limit the pulse position.
            if (pulse_position < MIN_POSITION) pulse_position = MIN_POSITION;
            if (pulse_position > MAX_POSITION) pulse_position = MAX_POSITION;

            // Apply a low pass filter to the pulse position.
            pulse_position = filter_update(pulse_position, &filter_reg_pulse);

// TODO: Delete:            // Are we reversing the seek sense?
// TODO: Delete:            if (registers_read_byte(REG_REVERSE_SEEK) != 0)
// TODO: Delete:            {
// TODO: Delete:                // Yes. Update the seek position using reverse sense.
// TODO: Delete:                registers_write_word(REG_SEEK_POSITION_HI, REG_SEEK_POSITION_LO, (MAX_POSITION - pulse_position));
// TODO: Delete:            }
// TODO: Delete:            else
            {
                // No. Update the seek position using normal sense.
                registers_write_word(REG_SEEK_POSITION_HI, pulse_position);
            }

            // The seek velocity will always be zero.
            registers_write_word(REG_SEEK_VELOCITY_HI, 0);

            // Make sure pwm is enabled.
            pwm_enable();

            // Update the pulse time used as a time stamp.
            pulse_time = timer_get();
        }

        // Reset the pulse time flag.
        pulse_flag = 0;
    }
    else
    {
        // If we haven't seen a pulse in .5 seconds disable pwm.
        if (timer_delta(pulse_time) > 50)
        {
            // Disable pwm.
            pwm_disable();

            // Update the pulse time used as a time stamp.
            pulse_time = timer_get();
        }
    }

    return;
}


SIGNAL(SIG_OVERFLOW2)
// Handles timer/counter2 overflow interrupt.
{
    // Increment the upper byte of the pulse timer.
    overflow_count += 1;
}


SIGNAL(SIG_PIN_CHANGE0)
// Handles pin change 0 interrupt.
{
    // Make sure we don't overwrite pulse times.
    if (!pulse_flag)
    {
        // We have to be careful how we handle Timer2 as we are using it as
        // a 16 bit counter with the overflow incrementing the upper 8 bits.
        // We need to be careful that TOV2 is checked to accurately update the
        // overflow count before the overflow count is used.

        // Did we go high or low?
        if (PINB & (1<<PINB3))
        {
            // Reset timer 2 count values.
            TCNT2 = 0;
            overflow_count = 0;

            // Set timer/counter2 control register B.
            TCCR2B = (0<<FOC2A) | (0<<FOC2B) |                      // No force output compare A or B.
                     (0<<WGM22) |                                   // Mode 0 - normal operation.
                     (0<<CS22) | (1<<CS21) | (0<<CS20);             // Clk/8 prescaler -- a 1MHz clock rate.

            // Set the timer/counter2 interrupt masks.
            TIMSK2 = (0<<OCIE2A) |                                  // No interrupt on compare match A.
                     (0<<OCIE2B) |                                  // No interrupt on compare match B.
                     (1<<TOIE2);                                    // Interrupt on overflow.

            // Clear any pending overflow interrupt.
            TIFR2 = (1<<TOV2);
        }
        else
        {
            // Stop timer2.
            TCCR2B = (0<<FOC2A) | (0<<FOC2B) |                      // No force output compare A or B.
                     (0<<WGM22) |                                   // Mode 0 - normal operation.
                     (0<<CS22) | (0<<CS21) | (0<<CS20);             // Disable clock source.

            // Check for one last overflow.
            if (TIFR2 & (1<<TOV2)) overflow_count += 1;

            // Save the pulse duration.
            pulse_duration = ((uint16_t) overflow_count << 8) | TCNT2;

            // We timed a pulse.
            pulse_flag = 1;

            // Set the timer/counter2 interrupt masks.
            TIMSK2 = (0<<OCIE2A) |                                  // No interrupt on compare match A.
                     (0<<OCIE2B) |                                  // No interrupt on compare match B.
                     (0<<TOIE2);                                    // No interrupt on overflow.

            // Clear any pending overflow interrupt.
            TIFR2 = (1<<TOV2);
        }
    }
}

#endif // PULSE_CONTROL_ENABLED
