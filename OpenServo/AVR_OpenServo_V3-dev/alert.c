/*
    Copyright (c) 2007 Barry Carter <Barry.Carter@gmail.com>

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

/*
    Alert functionality to monitor system status and fill an alert
    register with the alarms on the system. Sets bits in the ALERT_STATUS
    register that should be read and decoded by the host.

    NOTE: it is up to the host to reset the alert flags in the register
    manually after any pending operations are completed.

    Also note that these registers are not saved to eeprom over a reboot, 
    and are zero'ed on init. When set to 0 they are ineffective and ignored.
*/

#include <inttypes.h>
#include <string.h>
#include <avr/io.h>
#include "openservo.h"
#include "config.h"
#include "eeprom.h"
#include "pid.h"
#include "pwm.h"
#include "step.h"
#include "registers.h"
#include "banks.h"
#include "alert.h"

#if ALERT_ENABLED

static uint16_t throttle;

void alert_init(void)
// Function to initialize alerts.
{
    // reset the alerts register to 0, or no errors
    banks_write_byte(ALERT_CONFIG_BANK, ALERT_STATUS, 0x00);

    throttle = 0;

#if ALERT_INTN_ENABLED
    // Set the interrupt port(s) as an output
    ALERT_INTN_DDR |= ALERT_INTN_DDR_CONF;

    // Set it (them) low
    ALERT_INTN_PORT &= ~(1<<ALERT_INTN_PIN);

    // Set all alert interrupts on INTn enabled
    banks_write_byte(ALERT_CONFIG_BANK, REG_ALERT_CAUSES_INT, 0xFF);
    
#endif
}

void alert_defaults(void)
// Reset safe read/write registers to defaults.
{
    // put your default bank data here
    banks_write_word(ALERT_CONFIG_BANK, ALERT_VOLT_MAX_LIMIT_HI, ALERT_VOLT_MAX_LIMIT_LO, 0);
    banks_write_word(ALERT_CONFIG_BANK, ALERT_VOLT_MIN_LIMIT_HI, ALERT_VOLT_MIN_LIMIT_LO, 0);

    banks_write_word(ALERT_CONFIG_BANK, ALERT_TEMP_MAX_LIMIT_HI, ALERT_TEMP_MAX_LIMIT_LO, 0);

    banks_write_word(ALERT_CONFIG_BANK, ALERT_CURR_MAX_LIMIT_HI, ALERT_CURR_MAX_LIMIT_LO, 0);

    // Set all alerts enabled
    banks_write_byte(ALERT_CONFIG_BANK, REG_ALERT_ENABLE, 0xFF);

#if ALERT_INTN_ENABLED
    // Set all alert interrupts on INTn enabled
    banks_write_byte(ALERT_CONFIG_BANK, REG_ALERT_CAUSES_INT, 0xFF);

#endif
}

void alert_check(void)
// Check the alert registers for min/max overflows and set the status register accordingly
{
    uint16_t voltage;
    uint16_t current;
    uint16_t temperature;
    uint16_t max_voltage;
    uint16_t min_voltage;
    uint16_t max_current;
    uint16_t max_temperature;
    uint16_t cur_position;
    uint16_t seek_position;

    // Save cycles by returning here if we are disabled.
    if ((banks_read_byte(ALERT_CONFIG_BANK, REG_ALERT_ENABLE) == 0)
        && (ALERT_INTN_PORT & ALERT_INTN_PIN) == 0)
        return;

    // Get the current voltage and power
    voltage     = registers_read_word(REG_VOLTAGE_HI,REG_VOLTAGE_LO);
    current     = registers_read_word(REG_POWER_HI,REG_POWER_LO);
    temperature = registers_read_word(REG_TEMPERATURE_HI,REG_TEMPERATURE_LO);

    // Get the set limits for voltage and power
    max_voltage = banks_read_word(ALERT_CONFIG_BANK, ALERT_VOLT_MAX_LIMIT_HI, ALERT_VOLT_MAX_LIMIT_LO);
    min_voltage = banks_read_word(ALERT_CONFIG_BANK, ALERT_VOLT_MIN_LIMIT_HI, ALERT_VOLT_MIN_LIMIT_LO);

    max_temperature = banks_read_word(ALERT_CONFIG_BANK, ALERT_TEMP_MAX_LIMIT_HI, ALERT_TEMP_MAX_LIMIT_LO);

    max_current = banks_read_word(ALERT_CONFIG_BANK, ALERT_CURR_MAX_LIMIT_HI, ALERT_CURR_MAX_LIMIT_LO);

    cur_position = registers_read_word(REG_POSITION_HI, REG_POSITION_LO);
    seek_position = registers_read_word(REG_SEEK_POSITION_HI, REG_SEEK_POSITION_LO);

    // Check the voltage is not below or above the set voltage. Ignore if 0
    // NOTE: This would be a good place to alter the pwm of the motor to output the same voltage
    if (alert_is_enabled(ALERT_OVERVOLT))
    {
        if (voltage > max_voltage && max_voltage >0)
        {
            alert_setbit(ALERT_OVERVOLT);
        }
        else if (voltage < min_voltage && min_voltage >0)
        {
            alert_setbit(ALERT_UNDERVOLT);
        }
    }
    
    // Check the curent is not over the maximum set current. Ignore if 0
    // NOTE: This would be a good place to throttle the current if we want to
    if (current > max_current
        && max_current >0
        && alert_is_enabled(ALERT_OVERCURR))
    {
        alert_setbit(ALERT_OVERCURR);
        throttle = current - max_current ;
    }

    // Check the curent is not over the maximum set current. Ignore if 0
    // NOTE: This would be a good place to throttle the current if we want to
    if (temperature > max_temperature
        && max_temperature >0
        && alert_is_enabled(ALERT_OVERTEMP))
    {
        alert_setbit(ALERT_OVERTEMP);
#if PWM_ENABLED
        // Turn off pwm?
        pwm_disable();
#endif
#if STEP_ENABLED
        step_disable();
#endif
    }

    // Check to see if the position has reached the destination
    if (((cur_position > seek_position + 2) ||
       (cur_position < seek_position - 2)) &&
       (alert_is_enabled(ALERT_POSITION_REACHED)))
    {
        alert_setbit(ALERT_POSITION_REACHED);
        alert_int_high(ALERT_POSITION_REACHED);
    }
    else
        alert_clearbit(ALERT_POSITION_REACHED);

    // check if we can lower the interrupt pins
    alert_int_low();
    
    // If we are throttling on an alert, then decrement the counter.
    // This is so that the throttle is not permanent, and will gradually
    // fade away until the throttle is off.
    if(throttle >0) throttle--;
}

uint16_t alert_pwm_throttle(uint16_t pwm)
// Allow an alert to modify the PWM value
// This function runs in the ADC context, so don't tie up for any longer than 2ms
{
    // Do something here if you want to throttle PWM somehow

    if (pwm>0)
        pwm -= (throttle*2);
    else
        pwm += (throttle*2);

    return pwm;
}

// UTILITY FUNCTIONS

void alert_int_high(uint8_t bit)
{
    // Raise the INTn line on v3
#if ALERT_INTN_ENABLED
    ALERT_INTN_PORT |= (1<<ALERT_INTN_PIN);
#endif

}

void alert_int_low(void)
{
#if ALERT_INTN_ENABLED
    uint8_t status;
    status = banks_read_byte(ALERT_BANK, ALERT_STATUS);

    // Check to see if there are any existing interrupts pending
    // TODO discuss if we actually want it to pull low anyway as this will be pulled
    // high again on the next checkup
    if ((status & banks_read_byte(ALERT_CONFIG_BANK, REG_ALERT_CAUSES_INT)) > 0)
       return;

    // Lower the interrupt pin
    ALERT_INTN_PORT &= ~(1<<ALERT_INTN_PIN);
#endif
}

uint8_t alert_is_enabled(uint8_t bit)
{
    return ((banks_read_byte(ALERT_CONFIG_BANK, REG_ALERT_ENABLE) & bit) > 0 ? 1 : 0);
}

void alert_clearbit(uint8_t bit)
{
    uint8_t reg;
    reg = banks_read_byte(ALERT_BANK, ALERT_STATUS);
    reg &= ~(1<<bit);
    banks_write_byte(ALERT_BANK, ALERT_STATUS, reg);
}

void alert_setbit(uint8_t bit)
// set a bit in the register
{
    uint8_t reg;
    reg = banks_read_byte(ALERT_BANK, ALERT_STATUS);
    reg |= (1<<bit);
    banks_write_byte(ALERT_BANK, ALERT_STATUS, reg);
}

#endif
