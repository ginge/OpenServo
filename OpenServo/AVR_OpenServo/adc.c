/*
   Copyright (c) 2005, Mike Thompson <mpthompson@gmail.com>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

// The following is needed until WINAVR supports the ATtinyX5 MCUs.
#undef __AVR_ATtiny2313__
#define __AVR_ATtiny45__

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>

#include "adc.h"
#include "timer.h"

// The 10-bit Analog to Digital Converter (ADC) on the ATtinyX5 chip is used
// to provide power and position feedback from the servo circuitry.  Power
// to the servo motors is read from ADC0 (PB5) and potentiometer position is
// read from ADC3 (PB4) input channels.

// Defines for the power and position channels.
#define ADC_CHANNEL_POWER       0
#define ADC_CHANNEL_POSITION    1

// Globals used to maintain ADC state and values.
volatile uint8_t adc_channel;
volatile uint8_t adc_power_ready;
volatile uint16_t adc_power_value;
volatile uint8_t adc_position_ready;
volatile uint16_t adc_position_value;

void adc_init(void)
// Initialize ADC conversion for reading current monitoring on ADC0 (PB5)
// and position on ADC3 (PB4).  The ADC clock prescaler of 64 is selected
// to yield a 125 KHz ADC clock. The timer/clock 0 is programmed to trigger
// ADC samples every 512 microseconds.
{
    // Read from position first.
    adc_channel = ADC_CHANNEL_POSITION;

    // Initialize flags and values.
    adc_power_ready = 0;
    adc_power_value = 0;
    adc_position_ready = 0;
    adc_position_value = 0;

    //
    // Initialize ADC registers to yield a 125KHz clock.
    //

    // Make sure port PB4 (ADC3) and PB5 (ADC0) are set as input.
    PORTB &= ~((1<<PB4) | (1<<PB5));

    // Disable digital input for ADC3 and ADC0 to reduce power consumption.
    DIDR0 |= (1<<ADC3D) | (1<<ADC0D);

    // Set the ADC multiplexer selection register.
    ADMUX = (0<<REFS2) | (0<<REFS1) | (0<<REFS0) |          // Select VCC as voltage reference.
            (0<<MUX3) | (0<<MUX2) | (1<<MUX1) | (1<<MUX0) | // Select ADC3 (PB4), no gain.
            (0<<ADLAR);                                     // Keep high bits right adjusted.

    // Set the ADC control and status register B.
    ADCSRB = (0<<BIN) |                                     // Gain working in unipolar mode.
             (0<<IPR) |                                     // No input polarity reversal.
             (0<<ADTS2) | (1<<ADTS1) | (1<<ADTS0);          // Auto trigger to free running.

    // Set the ADC control and status register A.
    ADCSRA = (1<<ADEN) |                                    // Enable ADC.
             (0<<ADSC) |                                    // Don's start yet, will be auto triggered.
             (1<<ADATE) |                                   // Start auto triggering.
             (1<<ADIE) |                                    // Activate ADC conversion complete interrupt.
             (1<<ADPS2) | (1<<ADPS1) | (0<<ADPS0);          // Prescale to divide input clock by 64.

    //
    // Timer/Counter 0 settings.  This timer is used to trigger a ADC sample alternating
    // between the power and position channel every 512 microseconds.
    //

    // Set timer/counter 0 control register A.
    TCCR0A = (0<<COM0A1) | (0<<COM0A0) |                    // Disconnect OCOA.
             (0<<COM0B1) | (0<<COM0B0) |                    // Disconnect OCOB.
             (1<<WGM01) | (0<<WGM00);                       // Mode 2 - clear timer on compare match.

    // Set timer/counter 1 control register B.
    // Note: The clock is 8 MHz / 256 = 31.250 KHz or 32 microseconds per cycle.
    TCCR0B = (0<<FOC0A) | (0<<FOC0B) |                      // No force output compare A or B.
             (0<<WGM02) |                                   // Mode 2 - clear timer on compare match.
             (1<<CS02) | (0<<CS01) | (0<<CS00);             // Prescale divide by 256.

    // Set the timer/counter interrupt masks.
    TIMSK = (1<<OCIE0A) |                                   // Interrupt on compare match A.
            (0<<OCIE0B) |                                   // No interrupt on compare match B.
            (0<<TOIE0);                                     // No interrupt on overflow.

    // Set the compare match A value. Note: With a prescale of divide by
    // 256 and a compare match value of 8 an ADC sample is triggered every
    // 256 microseconds. Software below alternates between ADC0 and ADC3,
    // yielding a position sample every 512 microseconds with a power sample
    // interleaved every 512 microseconds.
    OCR0A = 0x08;
}


SIGNAL(SIG_OUTPUT_COMPARE0A)
// Handles timer/counter 0 compare match A.
{
    // Increment the timer.
    timer_increment();
}


SIGNAL(SIG_ADC)
// Handles ADC interrupt.
{
    uint16_t new_value;

    // Read the 10-bit ADC values by reading ADCH first and then ADCL.
    new_value = ADCL;
    new_value |= (ADCH << 8);

    // Make sure the value is never larger than 0x3ff.  This should never
    // happen, but better save than sorry because many other modules
    // assume this value is between 0x000 and 0x3ff.
    new_value &= 0x3ff;

    // Is the position or power channel being read?
    if (adc_channel == ADC_CHANNEL_POSITION)
    {
        // Save the new position value.
        adc_position_value = new_value;

        // Flag the position value as ready.
        adc_position_ready = 1;

        // Switch to power for the next reading.
        adc_channel = ADC_CHANNEL_POWER;

        // Set the ADC multiplexer selection register.
        ADMUX = (0<<REFS2) | (0<<REFS1) | (0<<REFS0) |          // Select VCC as voltage reference.
                (0<<MUX3) | (0<<MUX2) | (0<<MUX1) | (0<<MUX0) | // Select ADC0 (PB5), no gain.
                (0<<ADLAR);                                     // Keep high bits right adjusted.
    }
    else
    {
        // Save the new power value.
        adc_power_value = new_value;

        // Flag the power value as ready.
        adc_power_ready = 1;

        // Switch to position for the next reading.
        adc_channel = ADC_CHANNEL_POSITION;

        // Set the ADC multiplexer selection register.
        ADMUX = (0<<REFS2) | (0<<REFS1) | (0<<REFS0) |          // Select VCC as voltage reference.
                (0<<MUX3) | (0<<MUX2) | (1<<MUX1) | (1<<MUX0) | // Select ADC3 (PB4), no gain.
                (0<<ADLAR);                                     // Keep high bits right adjusted.

    }
}

