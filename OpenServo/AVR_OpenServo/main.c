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

// The following is needed until WINAVR supports the ATtinyX5 MCUs.
#undef __AVR_ATtiny2313__
#define __AVR_ATtiny45__

#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "config.h"
#include "adc.h"
#include "eeprom.h"
#include "estimator.h"
#include "ipd.h"
#include "pid.h"
#include "regulator.h"
#include "power.h"
#include "pwm.h"
#include "timer.h"
#include "twi.h"
#include "watchdog.h"
#include "registers.h"

#if defined(__AVR_ATtiny45__)
    // Do nothing.
#else
#  error "Don't know what kind of MCU you are compiling for"
#endif

void handle_twi_command(void)
{
    uint8_t command;

    // Get the command from the receive buffer.
    command = twi_receive_byte();

    switch (command)
    {
        case TWI_CMD_RESET:

            // Reset the servo.
            watchdog_hard_reset();

            break;

        case TWI_CMD_PWM_ENABLE:

            // Enable PWM to the servo motor.
            pwm_enable();

            break;

        case TWI_CMD_PWM_DISABLE:

            // Disable PWM to the servo motor.
            pwm_disable();

            break;

        case TWI_CMD_WRITE_ENABLE:

            // Enable write to read/write protected registers.
            registers_write_enable();

            break;

        case TWI_CMD_WRITE_DISABLE:

            // Disable write to read/write protected registers.
            registers_write_disable();

            break;

        case TWI_CMD_REGISTERS_SAVE:

            // Save register values into EEPROM.
            eeprom_save_registers();

            break;

        case TWI_CMD_REGISTERS_RESTORE:

            // Restore register values into EEPROM.
            eeprom_restore_registers();

            break;

        case TWI_CMD_REGISTERS_DEFAULT:

            // Restore register values to factory defaults.
            registers_defaults();
            break;

        default:

            // Ignore unknown command.
            break;
    }
}


int main (void)
{
    // Initialize the watchdog module.
    watchdog_init();

    // First, initialize registers that control servo operation.
    registers_init();

    // Initialize the PWM module.
    pwm_init();

    // Initialize the ADC module.
    adc_init();

#if ESTIMATOR_ENABLED
    // Initialize the state estimator module.
    estimator_init();
#endif

#if PID_MOTION_ENABLED
    // Initialize the PID algorithm module.
    pid_init();
#endif

#if IPD_MOTION_ENABLED
    // Initialize the IPD algorithm module.
    ipd_init();
#endif

#if REGULATOR_MOTION_ENABLED
    // Initialize the regulator algorithm module.
    regulator_init();
#endif

    // Initialize the power module.
    power_init();

    // Initialize the TWI slave module.
    twi_slave_init(registers_read_byte(REG_TWI_ADDRESS));

    // Finally initialize the timer.
    timer_set(0);

    // Enable interrupts.
    sei();

    // XXX Enable PWM and writing.  I do this for now to make development and
    // XXX tuning a bit easier.  Constantly manually setting these values to 
    // XXX turn the servo on and write the gain values get's to be a pain.
    pwm_enable();
    registers_write_enable();

    // Wait until initial position value is ready.
    while (!adc_position_value_is_ready());

    // Store the initial position as the initial command position.  If PWM is 
    // enabled this will have the servo hold the position it is currently at.
    registers_write_word(REG_SEEK_HI, REG_SEEK_LO, adc_get_position_value());

    // This is the main processing loop for the servo.  It basically looks
    // for new position, power or TWI commands to be processed.
    for (;;)
    {
        // Is position value ready?
        if (adc_position_value_is_ready())
        {
            int16_t pwm;
            int16_t position;

            // Get the new position value.
            position = (int16_t) adc_get_position_value();

#if ESTIMATOR_ENABLED
            // Estimate velocity.
            estimate_velocity(position);
#endif

#if PID_MOTION_ENABLED
            // Call the PID algorithm module to get a new PWM value.
            pwm = pid_position_to_pwm(position);
#endif

#if IPD_MOTION_ENABLED
            // Call the IPD algorithm module to get a new PWM value.
            pwm = ipd_position_to_pwm(position);
#endif

#if REGULATOR_MOTION_ENABLED
            // Call the state regulator algorithm module to get a new PWM value.
            pwm = regulator_position_to_pwm(position);
#endif

            // Update the servo movement as indicated by the PWM value.
            // Sanity checks are performed against the position value.
            pwm_update(position, pwm);
        }

        // Is a power value ready?
        if (adc_power_value_is_ready())
        {
            // Get the new power value.
            uint16_t power = adc_get_power_value();

            // Update the power value for reporting.
            power_update(power);
        }

        // Was a command recieved?
        if (twi_data_in_receive_buffer())
        {
            // Handle any TWI command.
            handle_twi_command();
        }

#if MAIN_MOTION_TEST_ENABLED
        // This code is in place for having the servo drive itself between 
        // two positions to aid in the servo tuning process.  This code 
        // should normally be disabled.
        {
            // Get the timer.  This timer is defined by the position/power sample
            // frequency and has a cycle time of approximately 512 microseconds.
            uint16_t timer = timer_get();

            // Reset the timer if greater than 16000.
            if (timer > 16000) timer_set(0);

            // Look for specific events.
            if (timer == 0)
            {
                registers_write_word(REG_SEEK_HI, REG_SEEK_LO, 0x0100);
            }
            else if (timer == 8000)
            {
                registers_write_word(REG_SEEK_HI, REG_SEEK_LO, 0x0300);
            }
        }
#endif
    }

    return 0;
}

