/*
    Copyright (c) 2008 Barry Carter <mpthompson@gmail.com>

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
#ifndef _OS_PINDEFS_H_
#define _OS_PINDEFS_H_ 1


/*
 * Configuration file for the HeadFuzz OpenStep module, which has PWM
 * and Stepper motor control. The configuration file below controls which
 * elements are enabled for this platform. Each option overrides the default
 * so we need not include them all.
 * /

// You can swap these to change the type of hardware output and switch
// between the stepper controller and the standard H-Bridge.
#define PWM_STD_ENABLED       0
#define PWM_ENH_ENABLED       0
#define STEP_ENABLED          1
#define PULSE_CONTROL_ENABLED 0

// We should never enable this for this hardware
#define BACKEMF_ENABLED   0

/*
 * ADC
 */
// Defines for the power and position channels.
// The channel the connection is on should be defined here
//  ADC0 (PC0) - Motor Current
//  ADC1 (PC1) - Battery Voltage
//  ADC2 (PC2) - Position
//  ADC6 (PC6) - Temperature
//  ADC7 (PC7) - Back EMF
#define ADC_CHANNEL_CURRENT         0
#define ADC_CHANNEL_BATTERY         1
#define ADC_CHANNEL_POSITION        2
#define ADC_CHANNEL_TEMPERATURE     6
#define ADC_CHANNEL_BACKEMF         7
#define ADC_PORT                    PORTC
#define ADC_IO_PINS                 ((1<<PC0) | (1<<PC1) | (1<<PC2))
#define ADC_DIDR                    DIDR0
#define ADC_DIGITAL                 (1<<ADC0D) | (1<<ADC1D) | (1<<ADC2D)

/*
 * RC PWM input defines
 */
#define RCPULSE_PORT                PORTB
#define RCPULSE_PIN                 PINB
#define RCPULSE_DDR                 DDRB
#define RCPULSE_DDR_DIR             DDB0
#define RCPULSE_INPUT_PIN           PB0
#define RCPULSE INPUT_PIN_I         PINB0
#define RCPULSE_INT_PIN             PCINT0

/*
 * Stepper motor defines
 */
#define STEP_PORT                   PORTD
#define STEP_PORT_DDR               DDRD
#define STEP_PORT_DDR_MASK          ((1<<DDD7) | (1<<DDD6 ) | (1<<DDD5 ) | (1<<DDD4))
#define STEP_PORT_MASK              0xF0


/*
 * ADC Clock defines
 */

// The ADC clock prescaler of 128 is selected to yield a 156.25 KHz ADC clock
// from an 20 MHz system clock.
#define ADPS		((1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0))

// The timer clock prescaler of 1024 is selected to yield a 19.531 KHz ADC clock
// from a 20 MHz system clock.
#define CSPS		((1<<CS02) | (0<<CS01) | (1<<CS00))

// Define the compare register value to generate a timer interrupt and initiate
// an ADC sample every 9.984 milliseconds and yield a 100.1603 Hz sample rate.
#define CRVALUE		195

#endif
