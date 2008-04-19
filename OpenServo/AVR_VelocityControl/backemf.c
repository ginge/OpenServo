/*
    Copyright (c) 2007 Barry Carter <barry.carter@headfuzz.co.uk>

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

/* Back EMF measurement module
   The speed of the servo can be determinied by disabling PWM output
   and sampling the voltage across the motor. 
*/

#include "backemf.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "openservo.h"
#include "config.h"
#include "power.h"
#include "registers.h"
#include "banks.h"
#include "adc.h"
#include "pwm.h"

#if BACKEMF_ENABLED

#define BACKEMF_FILTER_AVERAGE 0
#define BACKEMF_FILTER_LOWPASS 1

#define BACKEMF_FILTER_SELECTION BACKEMF_FILTER_LOWPASS

static uint8_t previous_tccr1a;


#if BACKEMF_FILTER_SELECTION==BACKEMF_FILTER_AVERAGE

#define AVERAGES 10
uint16_t backemf_averages[AVERAGES];
volatile uint8_t backemf_average_index=0;
static int16_t filter_update(int16_t input)
{
    int16_t bemf=0;
    uint8_t i;
    backemf_averages[backemf_average_index] = input;
    if (backemf_average_index<AVERAGES) backemf_average_index++;
    else backemf_average_index = 0;
    for (i=0;i<AVERAGES;i++) bemf+=backemf_averages[i];
    return bemf/AVERAGES;
}

#elif BACKEMF_FILTER_SELECTION==BACKEMF_FILTER_LOWPASS
//
// Digital Lowpass Filter Implementation
//
// See: A Simple Software Lowpass Filter Suits Embedded-system Applications
// http://www.edn.com/article/CA6335310.html
//
// k    Bandwidth (Normalized to 1Hz)   Rise Time (samples)
// 1    0.1197                          3
// 2    0.0466                          8
// 3    0.0217                          16
// 4    0.0104                          34
// 5    0.0051                          69
// 6    0.0026                          140
// 7    0.0012                          280
// 8    0.0007                          561
//
#define FILTER_SHIFT 2
static int32_t filter_reg_emf = 0;
static int16_t filter_update(int16_t input, int32_t *filter_source)
{
    // Update the filter with the current input.
    *filter_source = *filter_source - (*filter_source >> FILTER_SHIFT) + input;

    // Scale output for unity gain.
    return (int16_t) (*filter_source >> FILTER_SHIFT);
}

#endif //BACKEMF_FILTER_SELECTION==BACKEMF_FILTER_LOWPASS

void backemf_init(void)
// Initialise the back EMF module
{
#if BACKEMF_FILTER_SELECTION==BACKEMF_FILTER_AVERAGE
    int8_t i;
    for (i=0;i<AVERAGES;i++) backemf_averages[i] = 0;
#endif
}


void backemf_defaults(void)
{
    banks_write_byte(CONFIG_BANK,      REG_EMF_COLLAPSE_DELAY,         DEFAULT_EMF_COLLAPSE_DELAY);
    banks_write_byte(CONFIG_BANK,      REG_EMF_CHARGE_TIME,            DEFAULT_EMF_CHARGE_TIME);
}


void backemf_get_sample(void)
// Sets up the sampling mechanism for the back EMF reading.
{
    uint16_t bemf;
    uint8_t pwm_a;
    uint8_t pwm_b;

    pwm_a = registers_read_byte(REG_PWM_DIRA);
    pwm_b = registers_read_byte(REG_PWM_DIRB);

    // Delay for back EMF field collapse recovery. This is interruptable
    _delay_ms(banks_read_byte(CONFIG_BANK, REG_EMF_COLLAPSE_DELAY));

    // Sample the back emf
    bemf = (uint16_t)backemf_do_sample(pwm_a, pwm_b);

    // Filter noise on back emf singal        
#if BACKEMF_FILTER_SELECTION==BACKEMF_FILTER_AVERAGE
    bemf = filter_update(bemf);
#elif BACKEMF_FILTER_SELECTION==BACKEMF_FILTER_LOWPASS
    bemf = filter_update(bemf, &filter_reg_emf);
#endif

    // Write back emf to registers
    banks_write_word(INFORMATION_BANK, REG_BACKEMF_HI, REG_BACKEMF_LO, bemf);
}


uint16_t backemf_do_sample(uint8_t pwm_a, uint8_t pwm_b)
// Do one sample of the back EMF
{
    uint8_t emf_charge_time;
    uint8_t n;

    // Disable interrupts.
    cli();

    // Detect the direction of the EMF
    if (pwm_a == 0 && pwm_b > 0)
    {
        // Enable BEMF sample ON
        // Make sure that SMPLn_B (PD4) is held low.
        PORTD &= ~(1<<PD4);
        PORTD |= (1<<PD7);
    }
    else if (pwm_a >0 && pwm_b == 0)
    {
        // Enable BEMF sample ON
        // Make sure that SMPLn_A (PD7) is held low.
        PORTD &= ~(1<<PD7);
        PORTD |= (1<<PD4);
    }

    emf_charge_time = banks_read_byte(CONFIG_BANK, REG_EMF_CHARGE_TIME);
    // Delay to allow the BEMF sample hold circuit to charge to a reasonable level
    for (n=0; n<emf_charge_time; n++)
        _delay_us(38);

    // Enable BEMF sample OFF
    // Make sure that SMPLn_B (PD4) and SMPLn_A (PD7) are held high.
    PORTD |= ((1<<PD4) | (1<<PD7));

    // Enable interrupts here so we dont interrupt the cap charge delay
    sei();

    // Sample the BEMF ADC channel
    adc_start(ADC_CHANNEL_BACKEMF);

    // Wait for the samples to complete
    while(!adc_backemf_value_is_ready())
        ;;

    return (uint16_t) adc_get_backemf_value();
}


void backemf_coast_motor(void)
// Disables the PWM timer to coast the motor
{
    // Disable interrupts.
    cli();

    // Back up the old TCCR1A register configuration for later restore
    previous_tccr1a = TCCR1A;

    // Disable OC1A and OC1B outputs.
    TCCR1A &= ~((1<<COM1A1) | (1<<COM1A0));
    TCCR1A &= ~((1<<COM1B1) | (1<<COM1B0));

    // Restore interrupts.
    sei();
}


void backemf_restore_motor(void)
// Enable the PWM timer
{
    // Disable interrupts.
    cli();

    // ENABLE OC1A and OC1B outputs.
    TCCR1A = previous_tccr1a;

    // Restore interrupts.
    sei();
}

#endif
