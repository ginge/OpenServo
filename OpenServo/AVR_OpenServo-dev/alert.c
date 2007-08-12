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

#include "openservo.h"
#include "config.h"
#include "eeprom.h"
#include "estimator.h"
#include "ipd.h"
#include "pid.h"
#include "pwm.h"
#include "regulator.h"
#include "registers.h"
#include "banks.h"
#include "alert.h"

static uint16_t throttle;

void alert_init(void)
// Function to initialize alerts.
{
    // reset the alerts register to 0, or no errors
    banks_write_byte(0, ALERT_STATUS, 0x00);
    throttle = 0;
}

void alert_defaults(void)
// Reset safe read/write registers to defaults.
{
    // put your default bank data here

}

void alert_check(void)
// Check the alert registers for min/max overflows and set the status register accordingly
{
    uint8_t voltage;
    uint8_t current;
    uint8_t max_voltage;
    uint8_t min_voltage;
    uint8_t max_current;

    throttle = 0;

    // Get the current voltage and power
    voltage = registers_read_word(REG_VOLTAGE_HI,REG_VOLTAGE_LO);
    current = registers_read_word(REG_POWER_HI,REG_POWER_LO);

    // Get the set limits for voltage and power
    max_voltage = banks_read_word(BANK_1, ALERT_VOLT_MAX_LIMIT_HI, ALERT_VOLT_MAX_LIMIT_LO);
    min_voltage = banks_read_word(BANK_1, ALERT_VOLT_MIN_LIMIT_HI, ALERT_VOLT_MIN_LIMIT_LO);

    max_current = banks_read_word(BANK_1, ALERT_CURR_MAX_LIMIT_HI, ALERT_CURR_MAX_LIMIT_LO);

    // Check the voltage is not below or above the set voltage. Ignore if 0
    // NOTE: This would be a good place to alter the pwm of the motor to output the same voltage
    if (voltage > max_voltage && max_voltage >0)
    {
        banks_write_byte(BANK_0, ALERT_STATUS, alert_setbit(ALERT_STATUS, ALERT_OVERVOLT));
    }
    else if (voltage < min_voltage && min_voltage >0)
    {
        banks_write_byte(BANK_0, ALERT_STATUS, alert_setbit(ALERT_STATUS, ALERT_UNDERVOLT));
    }

    // Check the curent is not over the maximum set current. Ignore if 0
    // NOTE: This would be a good place to throttle the current if we want to
    if (current > max_current && max_current >0)
    {
        banks_write_byte(BANK_0, ALERT_STATUS, alert_setbit(ALERT_STATUS, ALERT_OVERCURR));
        throttle = max_current - current;
    }

}

uint16_t alert_pwm_throttle(uint16_t pwm)
// Allow an alert to modify the PWM value
// This function runs in the ADC context, so don't tie up for any longer than 2ms
{
    // Do something here if you want to throttle PWM somehow
    if (pwm>0)
        pwm -= (throttle * 5);
    else
        pwm += (throttle * 5);

    return pwm;
}

// UTILITY FUNCTIONS

uint8_t alert_setbit(uint8_t reg, uint8_t bit)
// set a bit in the register
{
    reg = reg^bit;

    return reg;
}
