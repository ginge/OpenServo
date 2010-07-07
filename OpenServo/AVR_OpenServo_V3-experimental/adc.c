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


// Globals used to maintain ADC state and values.
volatile uint8_t adc_power_ready;
volatile uint16_t adc_power_value;
volatile uint8_t adc_position_ready;
volatile uint16_t adc_position_value;
volatile uint16_t adc_position_value_supersample;
volatile uint32_t adc_position_value_supersample_wrk;
volatile uint16_t adc_position_samplecount;
#if BACKEMF_ENABLED
volatile uint8_t adc_backemf_ready;
volatile uint16_t adc_backemf_value;
#endif
#if TEMPERATURE_ENABLED
volatile uint8_t adc_temperature_ready;
volatile uint16_t adc_temperature_value;
#endif

void adc_init(void)
// Initialize ADC conversion for reading current monitoring and position.
{
    // Initialize flags and values.
    adc_power_ready = 0;
    adc_power_value = 0;
    adc_position_ready = 0;
    adc_position_value = 0; 
    adc_position_samplecount = 0;
    adc_position_value_supersample = 0;
    adc_position_value_supersample_wrk=0;

#if BACKEMF_ENABLED
    adc_backemf_ready = 0;
    adc_backemf_value = 0;
#endif
#if TEMPERATURE_ENABLED
    adc_temperature_ready = 0;
    adc_temperature_value = 0;
#endif

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
}

void adc_start(uint8_t channel)
// Initialise the adc sample on the first channel
{
   if(channel==ADC_CHANNEL_POSITION)
   {
      adc_position_samplecount=ADC_POSITION_SUPERSAMPLINGCOUNTS; // Maximum bandwidth: ~475Hz (as configured elsewhere at time of writing)
      adc_position_value_supersample_wrk=0;
   }

   // Set the ADC multiplexer selection register.
   ADMUX = (0<<REFS1) | (1<<REFS0) |        // Select AVCC as voltage reference.
      (0<<ADLAR) |                     // Keep high bits right adjusted.
      channel;                         // Position as the first channel to sample.

   ADCSRA |= (1<<ADSC);                     // Start the ADC

// An alternative method: but note the processor will be sleeping a lot...
// SMCR=1;
// asm("sleep");

}

ISR(SIG_ADC)
//
// ADC interrupt handler.
//
// This acts as a self triggering mechanism sampling the configured channels in a cascade, expcept
// for back EMF, which is handled elsewhere. After each channel completes the next configured channel
// is initiated. Once the cascade is complete, the next pass must be started from the firmware logic
// by calling the adc_start function with the argument ADC_FIRST_CHANNEL.
//
// Note that the position pot is "super-sampled" (this is not strictly "over sampling" as the sampling
// is not "continuous" nor does it seem at the moment that the OpenServo hardware provides sufficient
// evenly distributed low level noise for "over sampling" to be truly effective, both of these could
// be changed, given time). Given the way that the OpenServo firmware is currently written, only 16
// samples are taken, the next sampling level of 64 samples runs too close to the timing of the "event
// loop" in main.c, and would therefore probably be unreliable.
//
// The super-sampling of the position pot does appear to improve accuracy at lower velocities and should
// be helping with any transient noise issues. The step size of the position pot, in relation to the
// physical position of the servo is quite "large".
//
{
    // Read the 10-bit ADC value.
    uint16_t new_value = ADCW;
    uint8_t next_channel=0xFF;
    // Which channel is being read?
    switch (ADMUX & 0x0f)
    {


//
// Position channel
//
// Accumulate the new sample and decrement sampling counter. Once the sampling counter reaches
// zero, store the position value, set the event flag to indicate that it is ready and cascade
// to the next item to be sampled.
//
       case ADC_CHANNEL_POSITION:
           adc_position_value_supersample_wrk+=new_value;
           adc_position_samplecount--;
           if(adc_position_samplecount>0)
           {
              // Start another position pot sample
              next_channel=ADC_CHANNEL_POSITION;
           } else
           {
            // Save the new position value.
//               adc_position_value = new_value;
              adc_position_value_supersample=(int16_t)((adc_position_value_supersample_wrk+((1<<ADC_POSITION_SUPERSAMPLINGSHIFT)>>1))>>ADC_POSITION_SUPERSAMPLINGSHIFT);
              adc_position_value=(int16_t)((adc_position_value_supersample)>>ADC_POSITION_SUPERSAMPLINGBITS);

            // Flag the position value as ready.
              adc_position_ready = 1;

            // Start the ADC of the power channel now
              next_channel=ADC_CHANNEL_CURRENT;
           }
           break;

        case ADC_CHANNEL_CURRENT:

            // Save the new power value.
            adc_power_value = new_value;

            // Flag the power value as ready.
            adc_power_ready = 1;

#if TEMPERATURE_ENABLED
            next_channel = ADC_CHANNEL_TEMPERATURE;                  // Temperature as the next channel to sample.
#elif BATTERY_ENABLED
            next_channel = ADC_CHANNEL_BATTERY;                      // Sample the battery channel next
#endif
            break;

#if TEMPERATURE_ENABLED
        case ADC_CHANNEL_TEMPERATURE:

            // Save the temperature value
            adc_temperature_value = new_value;

            // Fag the temperature value as ready
            adc_temperature_ready = 1;
#if BATTERY_ENABLED
            next_channel = ADC_CHANNEL_BATTERY;                      // Battery voltage as the next channel to sample.
#endif
            break;
#endif
#if BATTERY_ENABLED
        case ADC_CHANNEL_BATTERY:

            // Save voltage value to registers
            registers_write_word(REG_VOLTAGE_HI, new_value);

            break;
#endif
#if BACKEMF_ENABLED
        case ADC_CHANNEL_BACKEMF:

            adc_backemf_ready = 1;

            adc_backemf_value = new_value;

            break;
#endif
    }

//
// If a next sampling channel has been set, start it. We do not call adc_start to keep the stack
// usage down (also not that adc_start resets the position pot sampling counter too).
//
    if(next_channel!=0x0FF)
    {
       // Set the ADC multiplexer selection register.
       ADMUX = (0<<REFS1) | (1<<REFS0) |   // Select AVCC as voltage reference.
          (0<<ADLAR) |                     // Keep high bits right adjusted.
          next_channel;                    // Next channel to sample.
       ADCSRA |= (1<<ADSC);                // Start the ADC
    }
}
