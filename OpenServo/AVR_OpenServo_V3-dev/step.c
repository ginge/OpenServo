/*
    Copyright (c) 2008 Jay Ragsdale <j.ragsdale@jayragsdale.com>
    Copyright (c) 2008 Barry Carter <barry.carter@robotfuzz.com>

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

/* Stepper motor control output for the OpenServo
   This is currently outside of the mainline branch until
   it is tested on target hardware. It is the intention to merge
   into the Version 2/3 branches when completed
*/

#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "openservo.h"
#include "config.h"
#include "step.h"
#include "registers.h"
#include "banks.h"

#if (STEP_ENABLED)

// Determines the compare value associated with the duty cycle for timer/counter1.
// This takes the top pwm and the bottom pwm and scales the difference between 0 and 255
#define PWM_OCRN_VALUE(div_hi, div_lo, pwm) (uint16_t) (((uint32_t) (pwm * ((((uint32_t) div_hi) - (uint32_t) div_lo) - 1)) / 255))

// Define our step modes
// This allows us to skip certain sections of the step table by
// incrementing by more than one table entry at a time

// This stores the incrementor value to be added to the table
// incremental value
#define STEP_SING_INC    1
#define STEP_DUAL_INC    1
#define STEP_M400_INC    0

// This is the table offset value
#define STEP_SING_OFFSET 0
#define STEP_DUAL_OFFSET 1
#define STEP_M400_OFFSET 0

// The current mode of operation
#define STEP_MODE_SINGLE 0
#define STEP_MODE_DUAL   1
#define STEP_MODE_M400   2

#define R_STOP              0
#define R_CLOCKWISE         1
#define R_COUNTER_CLOCKWISE 2
#define R_ACCEL             4
#define R_ACCEL_CLEAR       8

// Keep the current step available through context switches
static uint8_t direction;

static uint8_t offset;

// The current step in the sequence
volatile static uint8_t step_incrementor;

//Tacking of the current step in the step table
static int8_t current_step;

// The top value of the step frequency
static uint16_t pwm_top_value;

// Step sequencing to the motor is provided based on standard stepping sequences.
// The tables below illustrate the excitation sequences for each type of stepping motor.
//
//
//  Single-Phase Stepper
//  ----------------
//  | STEP | 1A | 1B | 2A | 2B |
//  |------+----+----+----+----|
//  |  1   | H  | L  | L  | L  |
//  |------+----+----+----+----|
//  |  2   | L  | H  | L  | L  |
//  |------+----+----+----+----|
//  |  3   | L  | L  | H  | L  |
//  |------+----+----+----+----|
//  |  4   | L  | L  | L  | H  |
//  |------+----+----+----+----|
//  |Clockwise Rotation        |
//
//  Dual-Phase Stepper
//  ----------------
//  | STEP | 1A | 1B | 2A | 2B |
//  |------+----+----+----+----|
//  |  1   | H  | L  | L  | H  |
//  |------+----+----+----+----|
//  |  2   | H  | H  | L  | L  |
//  |------+----+----+----+----|
//  |  3   | L  | H  | H  | L  |
//  |------+----+----+----+----|
//  |  4   | L  | L  | H  | H  |
//  |------+----+----+----+----|
//  |Clockwise Rotation        |
//
//  400 point Micro-Stepper
//  ----------------
//  | STEP | 1A | 1B | 2A | 2B |
//  |------+----+----+----+----|
//  |  1   | H  | L  | L  | L  |
//  |------+----+----+----+----|
//  |  2   | H  | H  | L  | L  |
//  |------+----+----+----+----|
//  |  3   | L  | H  | L  | L  |
//  |------+----+----+----+----|
//  |  4   | L  | H  | H  | L  |
//  |------+----+----+----+----|
//  |  5   | L  | L  | H  | L  |
//  |------+----+----+----+----|
//  |  6   | L  | L  | H  | H  |
//  |------+----+----+----+----|
//  |  7   | L  | L  | L  | H  |
//  |------+----+----+----+----|
//  |  8   | H  | L  | L  | H  |
//  |------+----+----+----+----|
//  |Clockwise Rotation        |
//
//  This can be translated into a single array using the microstep 400
//  table and using a little logic to interpret the correct step. It is
//  done this way to save on AVR code space, and allow the step table
//  to grow with higher res step tables.

// TODO store in flash
uint8_t step_sequence[8] =  {0x08,    //1,0,0,0
                             0x0C,    //1,1,0,0
                             0x04,    //0,1,0,0
                             0x06,    //0,1,1,0
                             0x02,    //0,0,1,0
                             0x03,    //0,0,1,1
                             0x01,    //0,0,0,1
                             0x09};   //1,0,0,1

void step_registers_defaults(void)
// Initialize the Stepping algorithm related register values.  This is done
// here to keep the Stepping related code in a single file.
{
    //Default step mode: Dual Phase
    direction = R_STOP;
    banks_write_byte(CONFIG_BANK, REG_STEP_MODE, STEP_MODE_M400);
    banks_write_word(CONFIG_BANK, REG_PWM_FREQ_DIVIDER_HI, REG_PWM_FREQ_DIVIDER_LO, DEFAULT_PWM_FREQ_DIVIDER);
}

void step_init(void)
// Initialize the Step module for controlling a stepper motor.
{

    uint16_t pwm_div;

    // Enable port a pins as outputs.
    STEP_PORT_DDR |= STEP_PORT_DDR_MASK;

    // Set motor outputs 1A (PA1), 1B (PA0), 2A (PA3), and 2B (PA2) to low.
    STEP_PORT &= ~(STEP_PORT_MASK);

    // Reset the timer1 configuration.
    TCNT1 = 0;
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1C = 0;
    TIMSK1 = 0;

    // Configure timer 1 for CTC mode (Clear Time on Compare).
    // The old pwm value that was generated is now a variable called step that is
    // used to determine a number of cycles between steps.
    TCCR1A = (0<<COM1A1) | (0<<COM1A0) |                    // Disable OC1A output.
             (0<<COM1B1) | (0<<COM1B0) |                    // Disable OC1B output.
             (0<<WGM11)  | (0<<WGM10);                      // CTC (Clear Timer on Compare) Mode, TOP = OCR1A
    TCCR1B = (0<<ICNC1)  | (0<<ICES1)  |                    // Input on ICP1 disabled.
             (0<<WGM13)  | (1<<WGM12)  |                    // CTC (Clear Timer on Compare) Mode, TOP = OCR1A
             (0<<CS12)   | (1<<CS11)   | (0<<CS10);         // No prescaling.
    TIMSK1 |= (1 << OCIE1A);                                // Enable CTC interrupt.
    OCR1A   = 0;                                            // Initialize CTC compare value to zero.

    // Update the step value
    uint8_t step_mode = banks_read_byte(CONFIG_BANK, REG_STEP_MODE);

    // Setup the step mode
    switch (step_mode)
    {
        case STEP_MODE_SINGLE:
            offset = STEP_SING_OFFSET;
            step_incrementor = STEP_SING_INC;
            break;
        case STEP_MODE_DUAL:
            offset = STEP_DUAL_OFFSET;
            step_incrementor = STEP_DUAL_INC;
            break;
        case STEP_MODE_M400:
            offset = STEP_M400_OFFSET;
            step_incrementor = STEP_M400_INC;
            break;
        default:
            offset = STEP_SING_OFFSET;
            step_incrementor = STEP_SING_INC;
    }

    current_step = 0;

#if STEP_ENABLE_BRIDGE_PIN
    // Set enable pin as output
    STEP_ENABLE_DDR |= (1<<STEP_ENABLE_PIN);

    step_enable_bridge();
#endif
}

void step_update(uint16_t position, int16_t step_in)
//  Update the timer delay that trigers a step.  The delay time is determined by the step value, which represents a value of the maximum delay.
//  The farther the step value is from zero, the longer the delay.
{
    uint16_t min_position;
    uint16_t max_position;
    uint8_t step_mode;
    static uint8_t prev_step_mode;
    static uint16_t prev_pwm_div;
    static int16_t prev_seek_position;
    static uint8_t step_accel_idx = 0;
    uint16_t duty_cycle;
    uint16_t pwm_div_hi;
    uint16_t pwm_div_lo;

    // Read the high and low values for the PWM frequencies. pwm_div_hi is the top frequency and
    // pwm_div_lo is the bottom frequency
    pwm_div_hi = (uint16_t)banks_read_byte(CONFIG_BANK, REG_PWM_FREQ_DIVIDER_HI);
    pwm_div_lo = (uint16_t)banks_read_byte(CONFIG_BANK, REG_PWM_FREQ_DIVIDER_LO);

    // Store these in the correct 16 bit size 
    pwm_div_hi = (uint8_t)pwm_div_hi << 8;
    pwm_div_lo = (uint8_t)pwm_div_lo << 8;

    // Set up for the configured step mode
    step_mode = banks_read_byte(CONFIG_BANK, REG_STEP_MODE);

    // Check to see if the step mode has changed in the registers on the fly
    if (prev_step_mode != step_mode)
        step_init();

    prev_step_mode = step_mode;

    min_position = banks_read_word(CONFIG_BANK, REG_MIN_SEEK_HI, REG_MIN_SEEK_LO);
    max_position = banks_read_word(CONFIG_BANK, REG_MAX_SEEK_HI, REG_MAX_SEEK_LO);

    // Make sure these values are sane 10-bit values.
    if (min_position > 0x3ff) min_position = 0x3ff;
    if (max_position > 0x3ff) max_position = 0x3ff;

    // Disable clockwise movements when position is below the minimum position.
    if ((position < min_position) && (step_in < 0)) step_in = 0;

    // Disable counter-clockwise movements when position is above the maximum position.
    if ((position > max_position) && (step_in > 0)) step_in = 0;

    // Determine if Stepping is disabled in the registers.
    // TODO cheeck for braking and disable bridge
    if (!(registers_read_byte(REG_FLAGS_LO) & (1<<FLAGS_LO_PWM_ENABLED))) step_in = 0;

    // Determine and set the direction: Stop (0), Clockwise (1), Counter-Clockwise (2).
    if (step_in < -5)
    {
        // Less than zero. Set the direction to clockwise.
        direction |=  R_COUNTER_CLOCKWISE;
        direction &=  ~R_CLOCKWISE;

        // Calculate our duty cycle value
        duty_cycle = PWM_OCRN_VALUE(pwm_div_hi, pwm_div_lo, -step_in);
    }
    else if (step_in > 5)
    {
        // More than zero. Set the direction to counter-clockwise.
        direction |= R_CLOCKWISE; //DIRECTION = 2
        direction &= ~R_COUNTER_CLOCKWISE;

        // Calculate our duty cycle value
        duty_cycle = PWM_OCRN_VALUE(pwm_div_hi, pwm_div_lo, step_in);
    }
    else
    {
        // Stop all stepping output to the motor by setting motor outputs to low.
        step_stop();
        return;
    }

    // If acceleration mode is set, the current speed is divided to all incremental
    // speed. This function also waits for the R_ACCEL_CLEAR flag from the timer interrupt
    // before starting the next increment. It should take 10 pulses to get up to speed.
    if ((direction & R_ACCEL) && (direction & R_ACCEL_CLEAR))
    {
        // Scale the current duty cycle by 64 and increment over a loop
        duty_cycle = (duty_cycle/64)*(step_accel_idx+1);
        step_accel_idx++;
        if (step_accel_idx >= 64)
        {
            step_accel_idx = 0;
            direction &= ~R_ACCEL;
        }
        // clear the acceleration flag ready for next toggle
        direction &= ~R_ACCEL_CLEAR;
    }

    // If the motor changes direction we need to wait a small amount of time to discharge,
    // and then start accelerating in the new direction.
    uint16_t seek_position = registers_read_word(REG_SEEK_POSITION_HI, REG_SEEK_POSITION_LO);
    if (seek_position != prev_seek_position)
    {
        direction |= R_ACCEL;
        step_accel_idx = 0;
        OCR1A = 6500;  // Wait 1ms (at 20mhz)
    }
    else
    {
        // A slower speed is a larger delay, so calculate the top pwm frequency, and take away the step delay.
        OCR1A =  65535 - pwm_div_lo - duty_cycle; //Update the CTC compare value with the modified value of step.
    }

    prev_seek_position = seek_position;

    // Enable the timer mask
    TIMSK1 |= (1 << OCIE1A);
}

void step_stop(void)
// Stop all stepping signals to the motor.
{
    //Disable CTC interrupt to save cpu cycles
    TIMSK1 &= ~(1<< OCIE1A);

    //  Disable outputs and set the Step Register to zero.  This will make the motor come to a stop.
    direction = R_STOP;  //DIRECTION = 0
//    // TODO
//     uint8_t brake = registers_read_byte(
//    // Check if we are braking or full stopping
//     if (!brake)
//         PORTA &= ~(1<<(STEP_PORT & STEP_MASK));

}

#if defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__) || defined(__AVR_ATTiny84__)
ISR(TIM1_COMPA_vect)
#else
ISR(TIMER1_COMPA_vect)
#endif
// Interrupt overflow routine using Compare to trigger output on the port
{
    // Clear the accelerator flag to signal the next increment.
    direction |= R_ACCEL_CLEAR;

    // We wait until Timer 1 reaches the time set by the step_value, then increment/decrement the step.
    if (direction & R_CLOCKWISE) // Clockwise
    {
        // Set the appropriate pins based on the Current Step in the sequence.
        STEP_PORT = ((step_sequence[current_step + offset]<<4) | (STEP_PORT & ~STEP_PORT_MASK));

        // Add the incremental value to the accumulated table index
        current_step += step_incrementor + 1;

        // Check for table overflows
        if (current_step > sizeof(step_sequence)-1) current_step = 0;
    }
    else if (direction & R_COUNTER_CLOCKWISE) // Counter-clockwise
    {
        // Set the appropriate pins based on the Current Step in the sequence.
        STEP_PORT = ((step_sequence[current_step - offset]<<4) | (STEP_PORT & ~STEP_PORT_MASK));

        // Going backwards through the table, so remove the incremted index
        current_step -= step_incrementor + 1;

        // Check for underflow reset back to the end of the step table
        if (current_step < 0) current_step = (sizeof(step_sequence)-1);
    }
}

#endif

