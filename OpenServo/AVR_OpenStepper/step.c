/*
    Copyright (c) 2008 Jay Ragsdale <j.ragsdale@jayragsdale.com>
    Copyright (c) 2008 Barry Carter <barry.carter@gmail.com>

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

// Define the connected pins for this AVR
// Please ensure the motor is connected as follows for this setup:
//    AVR | Motor
//    --------- 
//    PA1 -> 1A
//    PA0 -> 1B
//    PA3 -> 2A
//    PA2 -> 2B

#define STEP_PORT PORTA
#define STEP_MASK 0x10         // TODO: recalculate me!!

// setup DDR for port from above setup
#define DDRPORT DDRA
#define DDRMASK 0x0F           // TODO: recalculate me!!

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

// Keep the current step available through context switches
static uint8_t direction;

static uint8_t offset;
static uint8_t step_incrementor;

//
// ATtiny84
// =========
//
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
    direction = 0;
    registers_write_byte(REG_STEP_MODE, STEP_MODE_M400);
}

void step_init(void)
// Initialize the Step module for controlling a stepper motor.
{
    // Set motor outputs 1A (PA1), 1B (PA0), 2A (PA3), and 2B (PA2) to low.
    PORTA &= ~(1<<(STEP_PORT & STEP_MASK));

    // Enable port a pins as outputs.
    DDRA |= DDRMASK;

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
             (0<<CS12)   | (0<<CS11)   | (1<<CS10);         // No prescaling.
    TIMSK1 |= (1 << OCIE1A);                                // Enable CTC interrupt.
    OCR1A   = 0;                                            // Initialize CTC compare value to zero.

    // Update the step value
    uint8_t step_mode = registers_read_byte(REG_STEP_MODE);

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

// The following code is the initialization of an array which
// represents the stepping sequence to be used.  The mode selection determines 
// on which line in the table to start.
}

void step_update(uint16_t position, int16_t step_in)
//  Update the timer delay that trigers a step.  The delay time is determined by the step value, which represents a percentage of the maximum delay.  
//  The farther the step value is from zero, the shorter the delay.  Maximum delay for this application is 65535 cycles.
// JAY! this is -255 to +255 as defined in pid.c we need to rethink this!
{
    uint16_t min_position;
    uint16_t max_position;
    static uint8_t prev_step_mode;
    uint8_t step_mode = registers_read_byte(REG_STEP_MODE);

    min_position = registers_read_word(REG_MIN_SEEK_HI, REG_MIN_SEEK_LO);
    max_position = registers_read_word(REG_MAX_SEEK_HI, REG_MAX_SEEK_LO);

    // Make sure these values are sane 10-bit values.
    if (min_position > 0x3ff) min_position = 0x3ff;
    if (max_position > 0x3ff) max_position = 0x3ff;

    // Disable clockwise movements when position is below the minimum position.
    if ((position < min_position) && (step_in < 0)) step_in = 0;

    // Disable counter-clockwise movements when position is above the maximum position.
    if ((position > max_position) && (step_in > 0)) step_in = 0;

    // Determine if Stepping is disabled in the registers.
    if (!(registers_read_byte(REG_FLAGS_LO) & (1<<FLAGS_LO_STEP_ENABLED))) step_in = 0;

    //Where are we changing the timer?

    // Check to see if the step mode has changed in the registers on the fly
    if (prev_step_mode != step_mode)
       step_init();

    prev_step_mode = step_mode;

    // TODO: Fix me for proper scaling once there is real hardware! Make tunable via register?
    OCR1A = step_in * (65530/255); //Update the CTC compare value with the modified value of step.

    // Determine and set the direction: Stop (0), Clockwise (1), Counter-Clockwise (2).
    if (step_in < 0)
    {
        // Less than zero. Set the direction to clockwise.
        direction =  1; //DIRECTION = 1
    }
    else if (step_in > 0)
    {
        // More than zero. Set the direction to counter-clockwise.
        direction = 2; //DIRECTION = 2
    }
    else
    {
        // Stop all stepping output to the motor by setting motor outputs 1A (PA1), 1B (PA0), 2A (PA3), and 2B (PA2) to low.
        step_stop();
    }
}

void step_stop(void)
// Stop all stepping signals to the motor.
{
    //  Disable outputs and set the Step Register to zero.  This will make the motor come to a stop.
    direction = 0; //DIRECTION = 0
    PORTA &= ~(1<<(STEP_PORT & STEP_MASK));
    //Disable CTC interrupt to save cpu cycles
    TIMSK1 |= (0 << OCIE1A);
}

ISR(TIM1_COMPA_vect)
// Interrupt overflow routine using Compare to trigger output on the port
{
    static uint8_t current_step;

    // We wait until Timer 1 reaches the time set by the step_value, then increment/decrement the step. 
    if (direction == 1) // Clockwise
    {
        // Set the appropriate pins based on the Current Step in the sequence.
        PORTA |= step_sequence[current_step + offset];

        // Add the incremental value to the accumulated table index
        current_step += step_incrementor + 1;

        // Check for table overflows
        if (current_step > sizeof(step_sequence)) current_step = 0;
    }
    else if (direction == 2) // Counter-clockwise
    {
        // Set the appropriate pins based on the Current Step in the sequence.
        PORTA |= step_sequence[current_step - offset];

        // Going backwards through the table, so remove the incremted index
        current_step -= step_incrementor - 1;

        // Check for underflow
        if (current_step < 0) current_step = 0;
    }
    else //Stopped
    {
        //Disable outputs to the motor.
        step_stop();
    }
}
