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

// Back EMF measurement module
// 
// 

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

static uint8_t previous_tccr1a;
static uint8_t bemf_index;
static uint16_t bemf_array[8];


void backemf_init(void)
// Initialise the back EMF module
{
    uint8_t i;

    // Initialize the back emf index.
    bemf_index = 0;

    // Initialize the back emf array.
    for (i = 0; i < 8; ++i) bemf_array[i] = 0;

    // Setup the default charge times
    banks_write_byte(CONFIG_BANK,      REG_EMF_COLLAPSE_DELAY,         2);
    banks_write_byte(CONFIG_BANK,      REG_EMF_CHARGE_TIME,            3);
}


void backemf_defaults(void)
{
    banks_write_byte(CONFIG_BANK,      REG_EMF_COLLAPSE_DELAY,         2);
    banks_write_byte(CONFIG_BANK,      REG_EMF_CHARGE_TIME,            3);
}


void backemf_get_sample(void)
// Sets up the sampling mechanism for the back EMF reading.
{
    uint16_t bemf;
    uint8_t i;
    uint8_t j;
    uint8_t pwm_a;
    uint8_t pwm_b;

    pwm_a = registers_read_byte(REG_PWM_DIRA);
    pwm_b = registers_read_byte(REG_PWM_DIRB);

    // Delay for back EMF field collapse recovery. This is interruptable
    _delay_ms(banks_read_byte(CONFIG_BANK, REG_EMF_COLLAPSE_DELAY));

    // Sample the back emf 7 times to get an average
    for (j=0; j<7; j++)
    {
        // Get one sample of the EMF value
        bemf = (uint16_t)backemf_do_sample(pwm_a, pwm_b);

        // Insert the back emf value into the back emf array.
        bemf_array[bemf_index] = bemf;

        // Keep the index within the array bounds.
        bemf_index = (bemf_index + 1) & 7;

        // Reset the back emf value.
        bemf = 0;

        // Determine the back emf values across the back emf array.
        for (i = 0; i < 7; ++i) bemf += bemf_array[i];

        // Shift the sum of back emf values to find the average.
        bemf >>= 3;
    }

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
