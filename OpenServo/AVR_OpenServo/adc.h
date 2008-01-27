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

#ifndef _OS_ADC_H_
#define _OS_ADC_H_ 1

// Initialize ADC conversion.
void adc_init(void);

// Declare externally so in-lines work.
extern volatile uint8_t adc_power_ready;
extern volatile uint16_t adc_power_value;
extern volatile uint8_t adc_position_ready;
extern volatile uint16_t adc_position_value;
extern volatile uint8_t adc_voltage_needed;


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

inline static void adc_read_voltage(void) 
// Set a flag to start a adc on supply voltage channel.
{
    adc_voltage_needed = 1;
}

#endif // _OS_ADC_H_
