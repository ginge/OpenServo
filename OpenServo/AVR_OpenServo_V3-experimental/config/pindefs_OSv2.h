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
 * Configuration file for the OpenServo Version 2 module.
 * The configuration file below controls which elements are enabled 
 * for this platform. Each option overrides the default so we need 
 * not include them all.
 */

#define PWM_STD_ENABLED     1
#define PWM_ENH_ENABLED     0
#define STEP_ENABLED        0
#define BACKEMF_ENABLED     0
#define TEMPERATURE_ENABLED 0

#ifndef F_CPU 
#define F_CPU 8000000L
#endif

/*
* Configs for the PWM output pins
*/
#define PWM_DUTY_PORT     PORTB
#define PWM_DUTY_DDR      DDRB
#define PWM_DUTY_DDR_CONF ((1<<DDB1) | (1<<DDB2))
#define PWM_DUTY_PWMA     PB1
#define PWM_DUTY_PWMB     PB2

/*
* ADC
*/
// Defines for the power and position channels.
// The channel the connection is on should be defined here
//  ADC0 (PC0) - Motor Current
//  ADC1 (PC1) - Position
//  ADC2 (PC2) - Battery Voltage
#define ADC_CHANNEL_CURRENT         0
#define ADC_CHANNEL_POSITION        1
#define ADC_CHANNEL_BATTERY         2
#define ADC_PORT                    PORTC
#define ADC_IO_PINS                 ((1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3))
#define ADC_DIDR                    DIDR0
#define ADC_DIGITAL                 (1<<ADC0D) | (1<<ADC1D) | (1<<ADC2D)

/*
// RC PWM input defines
*/
#define RCPULSE_PORT                PORTB
#define RCPULSE_PIN                 PINB
#define RCPULSE_DDR                 DDRB
#define RCPULSE_DDR_DIR             DDB3
#define RCPULSE_INPUT_PIN           PB3
#define RCPULSE INPUT_PIN_I         PINB3
#define RCPULSE_INT_PIN             PCINT3


// Defines for clock frequency calculations
// The ADC clock prescaler of 64 is selected to yield a 125 KHz ADC clock
// from an 8 MHz system clock.
#define ADPS		((1<<ADPS2) | (1<<ADPS1) | (0<<ADPS0))

// The timer clock prescaler of 1024 is selected to yield a 7.8125 KHz ADC clock
// from a 8 MHz system clock.
#define CSPS		((1<<CS02) | (0<<CS01) | (1<<CS00))

// Define the compare register value to generate a timer interrupt and initiate
// an ADC sample every 9.987 milliseconds and yield a 100.1603 Hz sample rate.
#define CRVALUE		78

#endif
