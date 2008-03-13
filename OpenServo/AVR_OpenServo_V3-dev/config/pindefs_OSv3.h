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
 * Configuration file for the OpenServo Version 3 module, which has 
 * independant PWM driver control and back EMF support.
 * The configuration file below controls which elements are enabled 
 * for this platform. Each option overrides the default so we need 
 * not include them all.
 * /
// You can swap these to change the type of hardware output and switch
// between the stepper controller and the standard H-Bridge.
#define PWM_STD_ENABLED   0
#define PWM_ENH_ENABLED   1
#define STEP_ENABLED      0

// This is entirely optional, but is on by deault.
#define BACKEMF_ENABLED   1

/*
* Config for the PWM control output pins
* These include the bridge enable and PWM sample enable pins
*/
//  PB1/OC1A - PWM_A    - PWM pulse output direction A
//  PB2/OC1B - PWM_B    - PWM pulse output direction B
//  PD2      - EN_A     - PWM enable direction A
//  PD3      - EN_B     - PWM enable direction B
//  PD4      - SMPLn_B  - BEMF sample enable B
//  PD7      - SMPLn_A  - BEMF sample enable A
//
#define PWM_CTL_PORT      PORTD
#define PWM_CTL_DDR       DDRD
#define PWM_CRL_DDR_CONF  ((1<<DDD2) | (1<<DDD3) | (1<<DDD4) | (1<<DDD7))
#define PWM_PIN_SMPLA     PD7
#define PWM_PIN_SMPLB     PD4
#define PWM_PIN_ENA       PD2
#define PWM_PIN_ENB       PD3

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
//  ADC1 (PC1) - Battery Voltage
//  ADC2 (PC2) - Position
//  ADC3 (PC3) - Temperature
//  ADC7 (PC7) - Back EMF
#define ADC_CHANNEL_CURRENT         0
#define ADC_CHANNEL_BATTERY         1
#define ADC_CHANNEL_POSITION        2
#define ADC_CHANNEL_TEMPERATURE     3
#define ADC_CHANNEL_BACKEMF         7
#define ADC_PORT                    PORTC
#define ADC_IO_PINS                 ((1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3))
#define ADC_DIDR                    DIDR0
#define ADC_DIGITAL                 (1<<ADC0D) | (1<<ADC1D) | (1<<ADC2D) | (1<<ADC3D)

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
