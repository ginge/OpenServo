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

#ifndef _OS_ADC_H_
#define _OS_ADC_H_ 1

// Initialize ADC conversion.
void adc_init(void);

// Declare externally so in-lines work.
extern volatile uint8_t adc_power_ready;
extern volatile uint16_t adc_power_value;
extern volatile uint8_t adc_position_ready;
extern volatile uint16_t adc_position_value;

// In-lines for fast access to power flags and values.

inline static uint16_t adc_get_power_value(void)
// Return the signed 16-bit ADC power value.
{
    // Clear the ready ADC value ready flag.
    adc_power_ready = 0;

    // Return the value.
    return adc_power_value;
}

inline static uint8_t adc_power_value_is_ready(void)
// Return the ADC power value ready flag.
{
    // Return the value ready flag.
    return adc_power_ready;
}

inline static void adc_power_value_clear_ready(void)
// Clear the ready ADC power value ready flag.
{
    adc_power_ready = 0;
}

// In-lines for fast access to position flags and values.

inline static uint16_t adc_get_position_value(void)
// Return the 16-bit ADC position value.
{
    // Clear the ready ADC value ready flag.
    adc_position_ready = 0;

    // Return the value.
    return adc_position_value;
}

inline static uint8_t adc_position_value_is_ready(void)
// Return the ADC position value ready flag.
{
    // Return the value ready flag.
    return adc_position_ready;
}

inline static void adc_position_value_clear_ready(void)
// Clear the ready ADC power value ready flag.
{
    adc_position_ready = 0;
}

#endif // _OS_ADC_H_
