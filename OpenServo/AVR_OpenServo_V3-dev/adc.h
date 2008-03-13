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

// Initialize ADC conversion registers.
void adc_init(void);

// Start ADC conversion
void adc_start(uint8_t channel);

// Declare externally so in-lines work.
extern volatile uint8_t adc_power_ready;
extern volatile uint16_t adc_power_value;
extern volatile uint8_t adc_position_ready;
extern volatile uint16_t adc_position_value;
extern volatile uint8_t adc_heartbeat_ready;
#if BACKEMF_ENABLED
extern volatile uint8_t adc_backemf_ready;
extern volatile uint16_t adc_backemf_value;
#endif
#if TEMPERATURE_ENABLED
extern volatile uint8_t adc_temperature_ready;
extern volatile uint16_t adc_temperature_value;
#endif

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

inline static uint8_t adc_heartbeat_is_ready(void)
// Return the ADC position value ready flag.
{
    // Return the value ready flag.
    return adc_heartbeat_ready;
}

inline static void adc_heartbeat_value_clear_ready(void)
// Clear the ready ADC power value ready flag.
{
    adc_heartbeat_ready = 0;
}

#if BACKEMF_ENABLED
inline static uint16_t adc_get_backemf_value(void)
// Return the signed 16-bit ADC power value.
{
    // Clear the ready ADC value ready flag.
    adc_backemf_ready = 0;

    // Return the value.
    return adc_backemf_value;
}

inline static uint8_t adc_backemf_value_is_ready(void)
// Return the ADC power value ready flag.
{
    // Return the value ready flag.
    return adc_backemf_ready;
}

inline static void adc_backemf_value_clear_ready(void)
// Clear the ready ADC power value ready flag.
{
    adc_backemf_ready = 0;
}
#endif

#if TEMPERATURE_ENABLED
inline static uint16_t adc_get_temperature_value(void)
// Return the 16-bit ADC temperature value.
{
    // Clear the ready ADC value ready flag.
    adc_temperature_ready = 0;

    // Return the value.
    return adc_temperature_value;
}

inline static uint8_t adc_temperature_value_is_ready(void)
// Return the ADC temperature value ready flag.
{
    // Return the value ready flag.
    return adc_temperature_ready;
}

inline static void adc_temperature_value_clear_ready(void)
// Clear the ready ADC temperature value ready flag.
{
    adc_temperature_ready = 0;
}
#endif

#endif // _OS_ADC_H_
