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
#include <avr/interrupt.h>
#include <avr/io.h>

#include "openservo.h"
#include "config.h"
#include "pwm.h"
#include "registers.h"

//
// ATtiny25/45/85
// ==============
//
// PWM output to the servo motor utilizes Timer/Counter1.  Output to the
// motor is assigned as follows:
//
//  OC1A (PB1) - Servo counter-clockwise output
//  OC1B (PB4) - Servo clockwise output
//
// ATmega168
// =========
//
// PWM output to the servo motor utilizes Timer/Counter1 in 8-bit mode.  
// Output to the motor is assigned as follows:
//
//  OC1A (PB1) - Servo counter-clockwise output
//  OC1B (PB2) - Servo clockwise output
//


// PWM output to the servo motor utilizes the AVR ATtiny45/85 Timer/Counter1.
// Output OC1B (PB4) is used to turn the servo clockwise and output OC1A (PB1)
// is used to turn the motor counter-clockwise.

// Stored values for the PWM.
volatile uint8_t pwm_cw_save;
volatile uint8_t pwm_ccw_save;

static void pwm_cw(uint8_t pwm_width)
// Send PWM signal for clockwise rotation with the indicated pwm ratio (0 - 255).
// This function is meant to be called only by pwm_update.
{
    // Are we changing the pwm?
    if (pwm_width != pwm_cw_save)
    {
        // Disable interrupts.
        cli();

        // Store the new pwm.
        pwm_cw_save = pwm_width;
        pwm_ccw_save = 0;

#if defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
        // Set PB1/OC1A to low.
        PORTB &= ~(1<<PB1);

        // Disable timer 1 pulse width modulator A.
        TCCR1 = (0<<CTC1) |                                     // Don't reset after compare match.
                (0<<PWM1A) |                                    // Disable timer 1A PWM.
                (0<<COM1A1) | (0<<COM1A0) |                     // Disconnect timer 1A from outputs.
                (0<<CS13) | (0<<CS12) | (1<<CS11) | (0<<CS10);  // Prescale divide by 2.

        // Set the timer 1 output compare register C (OCR1C).  This values
        // sets how many clock cycles compose an entire PWM cycle.
        OCR1C = 0xFF;

        // Set the timer 1 ouput compare register B (OCR1B).  The ratio of this value to
        // 0xFF set in OCR1C determines the duty cycle of the PWM signal on OC1B/PB4.
        OCR1B = pwm_width;

        // Enable timer 1 pulse width modulator B.  This also configures OC1B/PB4
        // to be set when timer counter 1 (TCNT1) equals 0x01 and cleared when timer
        // counter 1 (TCNT1) equals value in output compare register B (OCR1B).
        GTCCR = (1<<PWM1B) |                                    // Enable timer 1B PWM.
                (1<<COM1B1) | (0<<COM1B0) |                     // OC1B cleared on compare match. Set on TCNT1=$01.
                (0<<FOC1B) | (0<<FOC1A) | (0<<PSR1);            // Unused timer 1 features.
#endif // __AVR_ATtiny45__ || __AVR_ATtiny85____

#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega168__)
        // Set PB1/OC1A to low.
        PORTB &= ~(1<<PB1);

        // Stop the timer counter.
        TCCR1B = 0;

        // Set the timer compare registers.  The ratio of this value to 0xFF
        // determines the duty cycle of the PWM signal on OC1A and OC1B.
        OCR1A = 0;
        OCR1B = pwm_width;

        // Disable timer 1 and enable timer 2.
        TCCR1A = (0<<COM1A1) | (0<<COM1A0) |                    // OC1A disconnected.
                 (1<<COM1B1) | (0<<COM1B0) |                    // Clear OC1B on compare match on up-count.
                 (0<<WGM11) | (1<<WGM10);                       // Phase correct PWM 8-bit - waveform generation mode 1.

        // Set clock select bits to start timer.
        TCCR1B = (0<<ICNC1) | (0<<ICES1) |                      // Input on ICP1 disabled.
                 (0<<WGM13) | (0<<WGM12) |                      // Select waveform mode 1.
                 (0<<CS22) | (0<<CS21) | (1<<CS20);             // No prescaling.
#endif // __AVR_ATmega8 || __AVR_ATmega168__

        // Restore interrupts.
        sei();
    }
}


static void pwm_ccw(uint8_t pwm_width)
// Send PWM signal for counter-clockwise rotation with the indicated pwm ratio (0 - 255).
// This function is meant to be called only by pwm_update.
{
    // Are we changing the pwm?
    if (pwm_width != pwm_ccw_save)
    {
        // Disable interrupts.
        cli();

        // Store the new pwm.
        pwm_cw_save = 0;
        pwm_ccw_save = pwm_width;

#if defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
        // Set PB4/OC1B to low.
        PORTB &= ~(1<<PB4);

        // Disable timer 1 pulse width modulator B.
        GTCCR = (0<<PWM1B) |                                    // Disable timer 1B PWM.
                (0<<COM1B1) | (0<<COM1B0) |                     // Disconnect timer 1B from outputs.
                (0<<FOC1B) | (0<<FOC1A) | (0<<PSR1);            // Unused timer 1 features.

        // Set the timer 1 output compare register C (OCR1C).  This values
        // sets how many clock cycles compose an entire PWM cycle.
        OCR1C = 0xFF;

        // Set the timer 1 ouput compare register B (OCR1A).  The ratio of this value to
        // 0xFF set in OCR1C determines the duty cycle of the PWM signal on OC1A/PB1.
        OCR1A = pwm_width;

        // Enable timer 1 pulse width modulator A.  This also configures OC1A/PB1
        // to be set when timer counter 1 (TCNT1) equals 0x01 and cleared when timer
        // counter 1 (TCNT1) equals value in output compare register A (OCR1A).
        TCCR1 = (0<<CTC1) |                                     // Don't reset after compare match.
                (1<<PWM1A) |                                    // Enable timer 1A PWM.
                (1<<COM1A1) | (0<<COM1A0) |                     // OC1A cleared on compare match. Set on TCNT1=$01.
                (0<<CS13) | (0<<CS12) | (1<<CS11) | (0<<CS10);  // Prescale divide by 2.
#endif // __AVR_ATtiny45__ || __AVR_ATtiny85____

#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega168__)
        // Set PB2/OC1B to low.
        PORTB &= ~(1<<PB2);

        // Stop the timer counter.
        TCCR1B = 0;

        // Set the timer compare registers.  The ratio of this value to 0xFF
        // determines the duty cycle of the PWM signal on OC1A and OC1B.
        OCR1A = pwm_width;
        OCR1B = 0;

        // Disable timer 1 and enable timer 2.
        TCCR1A = (1<<COM1A1) | (0<<COM1A0) |                    // Clear OC1A on compare match on up-count.
                 (0<<COM1B1) | (0<<COM1B0) |                    // OC1B disconnected.
                 (0<<WGM11) | (1<<WGM10);                       // Phase correct PWM 8-bit - waveform generation mode 1.

        // Set clock select bits to start timer.
        TCCR1B = (0<<ICNC1) | (0<<ICES1) |                      // Input on ICP1 disabled.
                 (0<<WGM13) | (0<<WGM12) |                      // Select waveform mode 1.
                 (0<<CS22) | (0<<CS21) | (1<<CS20);             // No prescaling.
#endif // __AVR_ATmega8 || __AVR_ATmega168__

        // Restore interrupts.
        sei();
    }
}


void pwm_init(void)
// Initialize the PWM module for controlling a DC motor.
{
    // Init is same as stop.
    pwm_stop();
}


void pwm_update(uint16_t position, int16_t pwm)
// Update the PWM signal being sent to the motor.  The PWM value should be
// a signed integer in the range of -255 to -1 for clockwise movement,
// 1 to 255 for counter-clockwise movement or zero to stop all movement.
// This function provides a sanity check against the servo position and
// will prevent the servo from being driven past a minimum and maximum
// position.
{
    uint8_t pwm_width;
    uint16_t min_position;
    uint16_t max_position;

    // Are we reversing the seek sense?
    if (registers_read_byte(REG_REVERSE_SEEK) != 0)
    {
        // Yes. Swap the minimum and maximum position.

        // Get the minimum and maximum seek position.
        min_position = registers_read_word(REG_MAX_SEEK_HI, REG_MAX_SEEK_LO);
        max_position = registers_read_word(REG_MIN_SEEK_HI, REG_MIN_SEEK_LO);

        // Make sure these values are sane 10-bit values.
        if (min_position > 0x3ff) min_position = 0x3ff;
        if (max_position > 0x3ff) max_position = 0x3ff;

        // Adjust the values because of the reverse sense.
        min_position = 0x3ff - min_position;
        max_position = 0x3ff - max_position;
    }
    else
    {
        // No. Use the minimum and maximum position as is.

        // Get the minimum and maximum seek position.
        min_position = registers_read_word(REG_MIN_SEEK_HI, REG_MIN_SEEK_LO);
        max_position = registers_read_word(REG_MAX_SEEK_HI, REG_MAX_SEEK_LO);

        // Make sure these values are sane 10-bit values.
        if (min_position > 0x3ff) min_position = 0x3ff;
        if (max_position > 0x3ff) max_position = 0x3ff;
    }

    // Disable clockwise movements when position is below the minimum position.
    if ((position < min_position) && (pwm < 0)) pwm = 0;

    // Disable counter-clockwise movements when position is above the maximum position.
    if ((position > max_position) && (pwm > 0)) pwm = 0;

    // Determine if PWM is disabled in the registers.
    if (!(registers_read_byte(REG_FLAGS_LO) & (1<<FLAGS_LO_PWM_ENABLED))) pwm = 0;

    // Determine direction of servo movement or stop.
    if (pwm < 0)
    {
        // Less than zero. Turn clockwise.

        // Get the PWM width from the PWM value.
        pwm_width = (uint8_t) -pwm;

        // Turn clockwise.
        pwm_cw(pwm_width);
    }
    else if (pwm > 0)
    {
        // More than zero. Turn counter-clockwise.

        // Get the PWM width from the PWM value.
        pwm_width = (uint8_t) pwm;

        // Turn counter-clockwise.
        pwm_ccw(pwm_width);
    }
    else
    {
        // Stop all PWM activity to the motor.
        pwm_stop();
    }

    // Update the pwm values.
    registers_write_byte(REG_PWM_CW, pwm_cw_save);
    registers_write_byte(REG_PWM_CCW, pwm_ccw_save);
}


void pwm_stop(void)
// Stop all PWM signals to the motor.
{
    // Disable interrupts.
    cli();

#if defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    // Set PB4/OC1B and PB1/OC1A to low.
    PORTB &= ~((1<<PB4) | (1<<PB1));

    // Enable PB4/OC1B and PB1/OC1A as outputs.
    DDRB |= ((1<<DDB4) | (1<<DDB1));

    // Disable timer1/counter2.
    TCCR1 = 0;
    GTCCR = 0;
#endif // __AVR_ATtiny45__ || __AVR_ATtiny85____

#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega168__)
    // Set PB1/OC1A and PB2/OC1B to low.
    PORTB &= ~((1<<PB2) | (1<<PB1));

    // Enable PB1/OC1A and PB2/OC1B as outputs.
    DDRB |= ((1<<DDB2) | (1<<DDB1));

    // Reset count and compare registers.
    TCNT1 = 0;
    OCR1A = 0;
    OCR1B = 0;

    // Disable timer/counter1.
#if defined(__AVR_ATmega168__)
    TCCR1C = 0;
#endif
    TCCR1B = 0;
    TCCR1A = 0;
#endif // __AVR_ATmega8 || __AVR_ATmega168__

    // Set the saved pwm values to zero.
    pwm_cw_save = 0;
    pwm_ccw_save = 0;

    // Update the pwm values.
    registers_write_byte(REG_PWM_CCW, pwm_cw_save);
    registers_write_byte(REG_PWM_CW, pwm_ccw_save);

    // Restore interrupts.
    sei();
}

