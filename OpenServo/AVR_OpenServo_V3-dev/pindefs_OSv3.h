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

#endif
