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
#include <avr/io.h>
#include <avr/interrupt.h>

#include "openservo.h"
#include "config.h"
#include "adc.h"
#include "timer.h"

//
// ATmega168
// =========
//
// The 10-bit Analog to Digital Converter (ADC) on the ATmega MCU is used
// to provide power and position feedback from the servo circuitry.  The
// analog inputs are assigned as follows:
//
//  ADC0 (PC0) - Motor Current
//  ADC1 (PC1) - Battery Voltage
//  ADC2 (PC2) - Position
//  ADC3 (PC3) - Temperature
//  ADC7 (PC7) - Back EMF

// Defines for the power and position channels.
#define ADC_CHANNEL_CURRENT         0
#define ADC_CHANNEL_BATTERY         1
#define ADC_CHANNEL_POSITION        2
#define ADC_CHANNEL_TEMPERATURE     3
#define ADC_CHANNEL_BACKEMF         7

// The ADC clock prescaler of 128 is selected to yield a 156.25 KHz ADC clock
// from an 20 MHz system clock.
#define ADPS		((1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0))

// The timer clock prescaler of 1024 is selected to yield a 19.531 KHz ADC clock
// from a 20 MHz system clock.
#define CSPS		((1<<CS02) | (0<<CS01) | (1<<CS00))

// Define the compare register value to generate a timer interrupt and initiate
// an ADC sample every 9.984 milliseconds and yield a 100.1603 Hz sample rate.
#define CRVALUE		195

// Globals used to maintain ADC state and values.
volatile uint8_t adc_power_ready;
volatile uint16_t adc_power_value;
volatile uint8_t adc_position_ready;
volatile uint16_t adc_position_value;
volatile uint8_t adc_voltage_needed;

void adc_init(void)
// Initialize ADC conversion for reading current monitoring and position.
{
    // Initialize flags and values.
    adc_power_ready = 0;
    adc_power_value = 0;
    adc_position_ready = 0;
    adc_position_value = 0;
    adc_voltage_needed = 1;

    // Make sure ports PC0 (ADC0), PC1 (ADC1) and PC2 (ADC2) are set low.
    PORTC &= ~((1<<PC0) | (1<<PC1) | (1<<PC2));

    // Disable digital input for ADC0, ADC1 and ADC2.
    DIDR0 |= (1<<ADC0D) | (1<<ADC1D) |(1<<ADC2D);

    // Set the ADC multiplexer selection register.
    ADMUX = (0<<REFS1) | (1<<REFS0) |                       // Select AVCC as voltage reference.
            (0<<ADLAR) |                                    // Keep high bits right adjusted.
            ADC_CHANNEL_POSITION;                           // Position as the first channel to sample.

    // Set the ADC control and status register B.
    ADCSRB = (0<<ADTS2) | (1<<ADTS1) | (1<<ADTS0);          // Timer/Counter0 Compare Match A.

    // Set the ADC control and status register A.
    ADCSRA = (1<<ADEN) |                                    // Enable ADC.
             (0<<ADSC) |                                    // Don't start yet, will be triggered manually.
             (0<<ADATE) |                                   // Don't start auto triggering.
             (1<<ADIE) |                                    // Activate ADC conversion complete interrupt.
             ADPS;											// Prescale -- see above.

    // Clear any pending interrupts.
    TIFR0 = (1<OCF0B) | (1<<OCF0A) | (1<TOV0);

    // Set the timer/counter0 interrupt masks.
    TIMSK0 = (1<<OCIE0A) |                                  // Interrupt on compare match A.
             (0<<OCIE0B) |                                  // No interrupt on compare match B.
             (0<<TOIE0);                                    // No interrupt on overflow.

    // Initialize the count and compare values.
    TCNT0 = 0;
    OCR0A = CRVALUE;
    OCR0B = 0;

    // Set timer/counter0 control register A.
    TCCR0A = (0<<COM0A1) | (0<<COM0A0) |                    // Disconnect OCOA.
             (0<<COM0B1) | (0<<COM0B0) |                    // Disconnect OCOB.
             (1<<WGM01) | (0<<WGM00);                       // Mode 2 - clear timer on compare match.

    // Set timer/counter0 control register B.
    TCCR0B = (0<<FOC0A) | (0<<FOC0B) |                      // No force output compare A or B.
             (0<<WGM02) |                                   // Mode 2 - clear timer on compare match.
             CSPS;											// Timer clock prescale -- see above.

}


SIGNAL(SIG_OUTPUT_COMPARE0A)
// Handles timer/counter0 compare match A.
{
    // Increment the timer.
    timer_increment();

    // Set the ADC multiplexer selection register.
    ADMUX = (0<<REFS1) | (1<<REFS0) |                       // Select AVCC as voltage reference.
            (0<<ADLAR) |                                    // Keep high bits right adjusted.
            ADC_CHANNEL_POSITION;                           // Position as the first channel to sample.

    // Start the ADC reading of the position channel.
    ADCSRA |= (1<<ADSC);
}


SIGNAL(SIG_ADC)
// Handles ADC interrupt.
{
    // Read the 10-bit ADC value.
    uint16_t new_value = ADCW;

    // Which channel is being read?
    switch (ADMUX & 0x0f)
    {

        case ADC_CHANNEL_POSITION:

            // Save the new position value.
            adc_position_value = new_value;

            // Flag the position value as ready.
            adc_position_ready = 1;

            // Set the ADC multiplexer selection register.
            ADMUX = (0<<REFS1) | (1<<REFS0) |                       // Select AVCC as voltage reference.
                    (0<<ADLAR) |                                    // Keep high bits right adjusted.
                    ADC_CHANNEL_CURRENT;                            // Power as the next channel to sample.

            // Start the ADC of the power channel now
            ADCSRA |= (1<<ADSC);

            break;


        case ADC_CHANNEL_CURRENT:

            // Save the new power value.
            adc_power_value = new_value;

            // Flag the power value as ready.
            adc_power_ready = 1;

            // Check the flag to see if a battery voltage measurement is required.
            if (adc_voltage_needed)
            {
                // Set the ADC multiplexer selection register.
                ADMUX = (0<<REFS1) | (1<<REFS0) |                       // Select AVCC as voltage reference.
                        (0<<ADLAR) |                                    // Keep high bits right adjusted.
                        ADC_CHANNEL_BATTERY;                            // Battery voltage as the next channel to sample.

                // Start the ADC of the voltage channel now
                ADCSRA |= (1<<ADSC);
            }

            break;


        case ADC_CHANNEL_BATTERY:

            // Remove flag
            adc_voltage_needed = 0;

            // Save voltage value to registers
            registers_write_word(REG_VOLTAGE_HI, REG_VOLTAGE_LO, new_value);

            break;
    }
}

