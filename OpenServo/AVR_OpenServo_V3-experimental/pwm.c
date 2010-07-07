/*
    Copyright (c) 2006 Michael P. Thompson <mpthompson@gmail.com>
        Portions of this code 
    Copyright (c) 2007 Barry Carter <barry.carter@gmail.com>


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
#include "backemf.h"
#include "config.h"
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include "openservo.h"
#include "pwm.h"
#include "registers.h"
#include "banks.h"

#if PWM_ENH_ENABLED

//
// ATmega168
// =========
//
// PWM output to the servo motor utilizes Timer/Counter1 in 8-bit mode.
// Output to the motor for the stock v3 is assigned as follows:
//
//  PB1/OC1A - PWM_A    - PWM pulse output direction A
//  PB2/OC1B - PWM_B    - PWM pulse output direction B
//  PD2      - EN_A     - PWM enable direction A
//  PD3      - EN_B     - PWM enable direction B
//  PD4      - SMPLn_B  - BEMF sample enable B
//  PD7      - SMPLn_A  - BEMF sample enable A
//

// Determine the top value for timer/counter1 from the frequency divider.
#define PWM_TOP_VALUE(div,pwm_max)      (uint16_t)((uint32_t)(((uint16_t) div << 4) - 1)+(uint32_t)(((((uint32_t) div << 4) - 1)/100)*(100-pwm_max)))

// Determines the compare value associated with the duty cycle for timer/counter1.
#define PWM_OCRN_VALUE(div,pwm) (uint16_t) (((uint32_t) pwm * (((uint32_t) div << 4) - 1)) / 255)

// Flags that indicate PWM output in A and B direction.
static uint8_t pwm_a;
static uint8_t pwm_b;

// Pwm frequency divider value.
static uint16_t pwm_div;
static uint8_t  pwm_max;

//
// The delay_loop function is used to provide a delay. The purpose of the delay is to
// allow changes asserted at the AVRs I/O pins to take effect in the H-bridge (for example
// turning on or off one of the MOSFETs). This is to prevent improper states from occurring
// in the H-bridge and leading to brownouts or "crowbaring" of the supply. This was more
// of a problem, prior to the introduction of the delay, with the faster processor clock
// rate that was introduced with the V3 boards (20MHz) than it was with the older8Hhz V2.1
// boards- there was still a problem with the old code, with that board, it was just less
// pronounced.
//
// NOTE: Lower impedance motors may increase the lag of the H-bridge and thus require longer
//       delays.
//
#define DELAYLOOP 20 // TODO: This needs to be adjusted to account for the clock rate.
                     //       This value of 20 gives a 5 microsecond delay and was chosen
                     //       by experiment with an "MG995".
inline static void delay_loop(int n)
{
    uint8_t i;
    for(i=0; i<n; i++)
    {
        asm("nop");
    }
}


static void pwm_dir_a(uint8_t pwm_duty)
// Send PWM signal for rotation with the indicated pwm ratio (0 - 255).
// This function is meant to be called only by pwm_update.
{
    // Determine the duty cycle value for the timer.
    uint16_t duty_cycle = PWM_OCRN_VALUE(pwm_div, pwm_duty);

    // Disable interrupts.
    cli();

    // Do we need to reconfigure PWM output for direction A?
    if (!pwm_a)
    { // Yes...

        // Set SMPLn_B (PWM_PIN_SMPLB) and SMPLn_A (PWM_PIN_SMPLA) to high.
        PWM_CTL_PORT |= ((1<<PWM_PIN_SMPLB) | (1<<PWM_PIN_SMPLA));

        // Set EN_B (PWM_PIN_ENB) to low.
        PWM_CTL_PORT &= ~(1<<PWM_PIN_ENB);

        // Disable PWM_A (PWM_DUTY_PWMA/OC1A) and PWM_B (PWM_DUTY_PWMB/OC1B) output.
        // NOTE: Actually PWM_A should already be disabled...
        TCCR1A = 0;

        // Make sure PWM_A (PWM_DUTY_PWMA/OC1A) and PWM_B (PWM_DUTY_PWMB/OC1B) are low.
        PWM_DUTY_PORT &= ~((1<<PWM_DUTY_PWMA) | (1<<PWM_DUTY_PWMB));

        // Give the H-bridge time to respond to the above, failure to do so or to wait long
        // enough will result in brownouts as the power is "crowbarred" to varying extents.
        // The delay required is also dependant on factors which may affect the speed with
        // which the MOSFETs can respond, such as the impedance of the motor, the supply
        // voltage, etc.
        //
        // Experiments (with an "MG995") have shown that 5microseconds should be sufficient
        // for most purposes.
        //
        delay_loop(DELAYLOOP);

        // Enable PWM_A (PWM_DUTY_PWMA/OC1A)  output.
        TCCR1A |= (1<<COM1A1);

        // Set EN_A (PWM_PIN_ENA) to high.
        PWM_CTL_PORT |= (1<<PWM_PIN_ENA);

        // NOTE: The PWM driven state of the H-bridge should not be switched to b-mode or braking
        //       without a suffient delay.

        // Reset the B direction flag.
        pwm_b = 0;
    }

    // Update the A direction flag.  A non-zero value keeps us from
    // recofiguring the PWM output A when it is already configured.
    pwm_a = pwm_duty;

    // Update the PWM duty cycle.
    OCR1B = 0;
    OCR1A = duty_cycle;

    // Restore interrupts.
    sei();
}


static void pwm_dir_b(uint8_t pwm_duty)
// Send PWM signal for rotation with the indicated pwm ratio (0 - 255).
// This function is meant to be called only by pwm_update.
{
    // Determine the duty cycle value for the timer.
    uint16_t duty_cycle = PWM_OCRN_VALUE(pwm_div, pwm_duty);

    // Disable interrupts.
    cli();

    // Do we need to reconfigure PWM output for direction B?
    if (!pwm_b)
    { // Yes...

        // Set SMPLn_B (PWM_PIN_SMPLB) and SMPLn_A (PWM_PIN_SMPLA) to high.
        PWM_CTL_PORT |= ((1<<PWM_PIN_SMPLB) | (1<<PWM_PIN_SMPLA));

        // Set EN_A (PWM_PIN_ENA) to low.
        PWM_CTL_PORT &= ~(1<<PWM_PIN_ENA);

        // Disable PWM_A (PWM_DUTY_PWMA/OC1A) and PWM_B (PWM_DUTY_PWMB/OC1B) output.
        // NOTE: Actually PWM_B should already be disabled...
        TCCR1A = 0;

        // Make sure PWM_A (PWM_DUTY_PWMA/OC1A) and PWM_B (PWM_DUTY_PWMB/OC1B) are low.
        PWM_DUTY_PORT &= ~((1<<PWM_DUTY_PWMA) | (1<<PWM_DUTY_PWMB));

        // Give the H-bridge time to respond to the above, failure to do so or to wait long
        // enough will result in brownouts as the power is "crowbarred" to varying extents.
        // The delay required is also dependant on factors which may affect the speed with
        // which the MOSFETs can respond, such as the impedance of the motor, the supply
        // voltage, etc.
        //
        // Experiments (with an "MG995") have shown that 5microseconds should be sufficient
        // for most purposes.
        //
        delay_loop(DELAYLOOP);

        // Enable PWM_B (PWM_DUTY_PWMB/OC1B) output.
        TCCR1A = (1<<COM1B1);

        // Set EN_B (PWM_PIN_ENB) to high.
        PWM_CTL_PORT |= (1<<PWM_PIN_ENB);

        // NOTE: The PWM driven state of the H-bridge should not be switched to a-mode or braking
        //       without a suffient delay.

        // Reset the A direction flag.
        pwm_a = 0;
    }

    // Update the B direction flag.  A non-zero value keeps us from
    // recofiguring the PWM output B when it is already configured.
    pwm_b = pwm_duty;

    // Update the PWM duty cycle.
    OCR1A = 0;
    OCR1B = duty_cycle;

    // Restore interrupts.
    sei();
}


void pwm_registers_defaults(void)
// Initialize the PWM algorithm related register values.  This is done
// here to keep the PWM related code in a single file.
{
    // PWM divider is a value between 1 and 1024.  This divides the fundamental
    // PWM frequency (500 kHz for 8MHz clock, 1250 kHz for 20MHz clock) by a
    // constant value to produce a PWM frequency suitable to drive a motor.  A
    // small motor with low inductance and impedance such as those found in an
    // RC servo will my typically use a divider value between 16 and 64.  A larger
    // motor with higher inductance and impedance may require a greater divider.
    banks_write_word(POS_PID_BANK, REG_PWM_FREQ_DIVIDER_HI, DEFAULT_PWM_FREQ_DIVIDER);

    // This sets the maximum PWM output as a percentage of the calculated PWM divider values
    banks_write_byte(POS_PID_BANK, REG_PWM_MAX, DEFAULT_PWM_MAX);

    // Enable the H-Bridge brake by default.
    pwm_brake_enable();
}


void pwm_init(void)
// Initialize the PWM module for controlling a DC motor.
{
    // Initialize the pwm frequency divider value.
    pwm_div = banks_read_word(POS_PID_BANK, REG_PWM_FREQ_DIVIDER_HI);
    pwm_max = banks_read_byte(CONFIG_BANK,  REG_PWM_MAX);

    // Set EN_A (PWM_PIN_ENA) and EN_B (PWM_PIN_ENB) to low.
    PWM_CTL_PORT &= ~((1<<PWM_PIN_ENA) | (1<<PWM_PIN_ENB));

    // Set SMPLn_B (PWM_PIN_SMPLB) and SMPLn_A (PWM_PIN_SMPLA) to high.
    PWM_CTL_PORT |= ((1<<PWM_PIN_SMPLB) | (1<<PWM_PIN_SMPLA));

    // Enable PWM_PIN_ENA, PWM_PIN_ENB, PWM_PIN_SMPLB and PWM_PIN_SMPLA as outputs.
    PWM_CTL_DDR |= PWM_CRL_DDR_CONF;

    // Set PWM_A (PWM_DUTY_PWMA/OC1A) and PWM_B (PWM_DUTY_PWMB/OC1B) are low.
    PWM_DUTY_PORT &= ~((1<<PWM_DUTY_PWMA) | (1<<PWM_DUTY_PWMB));

    // Enable PWM_DUTY_PWMA/OC1A and PWM_DUTY_PWMB/OC1B as outputs.
    PWM_DUTY_DDR |= PWM_DUTY_DDR_CONF;

    // Reset the timer1 configuration.
    TCNT1 = 0;
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1C = 0;
    TIMSK1 = 0;

    // Set timer top value.
    ICR1 = PWM_TOP_VALUE(pwm_div, pwm_max);

    // Set the PWM duty cycle to zero.
    OCR1A = 0;
    OCR1B = 0;

    // Configure timer 1 for PWM, Phase and Frequency Correct operation, but leave outputs disabled.
    TCCR1A = (0<<COM1A1) | (0<<COM1A0) |                    // Disable OC1A output.
             (0<<COM1B1) | (0<<COM1B0) |                    // Disable OC1B output.
             (0<<WGM11)  | (0<<WGM10);                      // PWM, Phase and Frequency Correct, TOP = ICR1
    TCCR1B = (0<<ICNC1)  | (0<<ICES1)  |                    // Input on ICP1 disabled.
             (1<<WGM13)  | (0<<WGM12)  |                    // PWM, Phase and Frequency Correct, TOP = ICR1
             (0<<CS12)   | (0<<CS11)   | (1<<CS10);         // No prescaling.

    // Update the pwm values.
    registers_write_byte(REG_PWM_DIRA, 0);
    registers_write_byte(REG_PWM_DIRB, 0);

    // Enable the H-Bridge brake by default.
    pwm_brake_enable();
}


void pwm_update(uint16_t position, int16_t pwm, uint8_t braking)
//
// Update the PWM signal being sent to the motor.  The PWM value should be
// a signed integer in the range of -255 to -1 for clockwise movement,
// 1 to 255 for counter-clockwise movement or zero to stop all movement.
// The braking parameter allows the pid function(s) to control braking
// during controlled positioning of the servo, a non-zero value enables
// braking (when PWM is 0) and a zero value disables it. If the PWM
// drive is disabled, the braking flag is locally forced to the default
// setting.
//
// This function provides a sanity check against the servo position and
// will prevent the servo from being driven past a minimum and maximum
// position.
{
    uint8_t pwm_width;
    uint16_t min_position;
    uint16_t max_position;

    // Quick check to see if the frequency divider changed.  If so we need to
    // configure a new top value for timer/counter1.  This value should only
    // change infrequently so we aren't too elegant in how we handle updating
    // the value.  However, we need to be careful that we don't configure the
    // top to a value lower than the counter and compare values.
    if ((banks_read_word(POS_PID_BANK, REG_PWM_FREQ_DIVIDER_HI) != pwm_div) ||
       (banks_read_byte(POS_PID_BANK, REG_PWM_MAX) != pwm_max))
    {

        // Hold EN_A (PWM_PIN_ENA) and EN_B (PWM_PIN_ENB) low.
        PWM_CTL_PORT &= ~((1<<PWM_PIN_ENA) | (1<<PWM_PIN_ENB));

        // Give the H-bridge time to respond to the above, failure to do so or to wait long
        // enough will result in brownouts as the power is "crowbarred" to varying extents.
        // The delay required is also dependant on factors which may affect the speed with
        // which the MOSFETs can respond, such as the impedance of the motor, the supply
        // voltage, etc.
        //
        // Experiments (with an "MG995") have shown that 5microseconds should be sufficient
        // for most purposes.
        //
        delay_loop(DELAYLOOP);

        // Make sure that PWM_A (PWM_DUTY_PWMA/OC1A) and PWM_B (PWM_DUTY_PWMB/OC1B) are held low.
        PWM_DUTY_PORT &= ~((1<<PWM_DUTY_PWMA) | (1<<PWM_DUTY_PWMB));

        // Disable OC1A and OC1B outputs.
        TCCR1A &= ~((1<<COM1A1) | (1<<COM1A0));
        TCCR1A &= ~((1<<COM1B1) | (1<<COM1B0));

        // Reset the A and B direction flags.
        pwm_a = 0;
        pwm_b = 0;

        // Update the pwm frequency divider value.
        pwm_div = banks_read_word(POS_PID_BANK, REG_PWM_FREQ_DIVIDER_HI);
        pwm_max = banks_read_byte(POS_PID_BANK, REG_PWM_MAX);

        // Update the timer top value.
        ICR1 = PWM_TOP_VALUE(pwm_div, pwm_max);

        // Reset the counter and compare values to prevent problems with the new top value.
        TCNT1 = 0;
        OCR1A = 0;
        OCR1B = 0;
    }

// TODO: Delete:    // Are we reversing the seek sense?
// TODO: Delete:    if (banks_read_byte(POS_PID_BANK, REG_REVERSE_SEEK) != 0)
// TODO: Delete:    {
// TODO: Delete:        // Yes. Swap the minimum and maximum position.
// TODO: Delete:
// TODO: Delete:        // Get the minimum and maximum seek position.
// TODO: Delete:        min_position = banks_read_word(POS_PID_BANK, REG_MAX_SEEK_HI, REG_MAX_SEEK_LO);
// TODO: Delete:        max_position = banks_read_word(POS_PID_BANK, REG_MIN_SEEK_HI, REG_MIN_SEEK_LO);
// TODO: Delete:
// TODO: Delete:        // Make sure these values are sane 10-bit values.
// TODO: Delete:        if (min_position > 0x3ff) min_position = 0x3ff;
// TODO: Delete:        if (max_position > 0x3ff) max_position = 0x3ff;
// TODO: Delete:
// TODO: Delete:        // Adjust the values because of the reverse sense.
// TODO: Delete:        min_position = 0x3ff - min_position;
// TODO: Delete:        max_position = 0x3ff - max_position;
// TODO: Delete:    }
// TODO: Delete:    else
    {
        // No. Use the minimum and maximum position as is.

        // Get the minimum and maximum seek position.
        min_position = banks_read_word(POS_PID_BANK, REG_MIN_SEEK_HI);
        max_position = banks_read_word(POS_PID_BANK, REG_MAX_SEEK_HI);

        // Make sure these values are sane 10-bit values.
        if (min_position > 0x3ff) min_position = 0x3ff;
        if (max_position > 0x3ff) max_position = 0x3ff;
    }

    // Disable clockwise movements when position is below the minimum position.
    if ((position < min_position) && (pwm < 0)) pwm = 0;

    // Disable counter-clockwise movements when position is above the maximum position.
    if ((position > max_position) && (pwm > 0)) pwm = 0;

    // Determine if PWM is disabled in the registers.
    if (!(registers_read_byte(REG_FLAGS_LO) & (1<<FLAGS_LO_PWM_ENABLED)))
    {
       pwm = 0;
       braking=(registers_read_byte(REG_FLAGS_LO) & (1<<FLAGS_LO_PWM_BRAKE_ENABLED));
    }

    // Determine direction of servo movement or stop.
    if (pwm < 0)
    {
        // Less than zero. Turn "clockwise" (the direction in which the motor actually turns depends on how it is wired and your POV)

        // Get the PWM width from the PWM value.
        pwm_width = (uint8_t) -pwm;

        // Turn clockwise.
#if SWAP_PWM_DIRECTION_ENABLED
        pwm_dir_b(pwm_width);
#else
        pwm_dir_a(pwm_width);
#endif
    }
    else if (pwm > 0)
    {
        // More than zero. Turn "counter-clockwise".

        // Get the PWM width from the PWM value.
        pwm_width = (uint8_t) pwm;

        // Turn counter-clockwise.
#if SWAP_PWM_DIRECTION_ENABLED
        pwm_dir_a(pwm_width);
#else
        pwm_dir_b(pwm_width);
#endif

    }
    else
    {
        // Stop all PWM activity to the motor.
        pwm_stop(braking);
    }
    // Save the pwm A and B duty values.
    registers_write_byte(REG_PWM_DIRA, pwm_a);
    registers_write_byte(REG_PWM_DIRB, pwm_b);
}


void pwm_stop(uint8_t braking)
// Stop all PWM signals to the motor.
{
    // Disable interrupts.
    cli();

    // Are we moving in the A or B direction?
    if (pwm_a || pwm_b)
    {
        // Make sure that SMPLn_B (PWM_PIN_SMPLB) and SMPLn_A (PWM_PIN_SMPLA) are held high.
        PWM_CTL_PORT |= ((1<<PWM_PIN_SMPLB) | (1<<PWM_PIN_SMPLA));

        // Disable PWM_A (PWM_DUTY_PWMA/OC1A) and PWM_B (PWM_DUTY_PWMB/OC1B) output.
        TCCR1A = 0;

        // Make sure that PWM_A (PWM_DUTY_PWMA/OC1A) and PWM_B (PWM_DUTY_PWMB/OC1B) are held low.
        PWM_DUTY_PORT &= ~((1<<PWM_DUTY_PWMA) | (1<<PWM_DUTY_PWMB));

        // Before enabling braking (which turns on the "two lower MOSFETS"), introduce
        // sufficient delay to give the H-bridge time to respond to the change of state 
        // that has just been made.
        delay_loop(DELAYLOOP);

        // Do we want to enable braking?
        if (braking)
        {
            // Hold EN_A (PWM_PIN_ENA) and EN_B (PWM_PIN_ENB) high.
            PWM_CTL_PORT |= ((1<<PWM_PIN_ENA) | (1<<PWM_PIN_ENB));
        }
        else
        {
            // Hold EN_A (PWM_PIN_ENA) and EN_B (PWM_PIN_ENB) low.
            PWM_CTL_PORT &= ~((1<<PWM_PIN_ENA) | (1<<PWM_PIN_ENB));
        }

        // Reset the A and B direction flags.
        pwm_a = 0;
        pwm_b = 0;
    }

    // Set the PWM duty cycle to zero.
    OCR1A = 0;
    OCR1B = 0;

    // Restore interrupts.
    sei();
}

#endif

