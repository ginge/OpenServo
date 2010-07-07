/*
    Copyright (c) 2006 Michael P. Thompson <mpthompson@gmail.com>
    Copyright (c) 2009 barry Carter <barry.carter@gmail.com>

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
#include <avr/interrupt.h>
#include <avr/io.h>

#include "openservo.h"
#include "config.h"
#include "adc.h"
#include "eeprom.h"
#include "motion.h"
#include "pid.h"
#include "power.h"
#include "step.h"
#include "pwm.h"
#include "seek.h"
#include "pulsectl.h"
#include "timer.h"
#include "twi.h"
#include "watchdog.h"
#include "registers.h"
#include "backemf.h"
#include "alert.h"
#include "banks.h"
#include "heartbeat.h"

static void config_pin_defaults(void)
// Configure pins to their default states to conform to recommendation that all
// AVR MCU pins have a defined level.  We do this by configuring unused pins
// as inputs and enabling the internal pull-ups.
{
    // Configure unused port B pins as inputs and enable internal pull-up resistor.
    DDRB = (0<<DDB7) | (0<<DDB6) | (0<<DDB5) | (0<<DDB4) |
           (0<<DDB3) | (1<<DDB2) | (1<<DDB1) | (0<<DDB0);
    PORTB = (1<<PORTB7) | (1<<PORTB6) | (1<<PORTB5) | (1<<PORTB4) |
            (1<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (1<<PORTB0);

    // Configure unused port C pins as inputs and enable internal pull-up resistor.
    DDRC = (0<<DDC6) | (0<<DDC5) | (0<<DDC4) |
           (0<<DDC3) | (0<<DDC2) | (0<<DDC1) | (0<<DDC0);
    PORTC = (1<<PORTC6) | (1<<PORTC5) | (1<<PORTC4) |
            (1<<PORTC3) | (1<<PORTC2) | (1<<PORTC1) | (1<<PORTC0);

    // Configure port D pins as inputs and enable internal pull-up resistor.
    DDRD = (1<<DDD7) | (0<<DDD6) | (0<<DDD5) | (1<<DDD4) |
           (1<<DDD3) | (1<<DDD2) | (0<<DDD1) | (0<<DDD0);
    PORTD = (1<<PORTD7) | (1<<PORTD6) | (1<<PORTD5) | (1<<PORTD4) |
            (0<<PORTD3) | (0<<PORTD2) | (0<<PORTD1) | (0<<PORTD0);
}


static void handle_twi_command(void)
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
#if PWM_STD_ENABLED || PWM_ENH_ENABLED
            // Enable PWM to the servo motor.
//            pid_reset();
            pwm_enable();
#endif
#if STEP_ENABLED
            // Enable Stepper motor control.
            step_enable();
#endif

            break;

        case TWI_CMD_PWM_DISABLE:
#if PWM_STD_ENABLED || PWM_ENH_ENABLED
            // Disable PWM to the servo motor.
            pwm_disable();
#endif

#if STEP_ENABLED
            // Enable Stepper motor control.
            step_disable();
#endif

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

        case TWI_CMD_EEPROM_ERASE:

            // Erase the EEPROM.
            eeprom_erase();

            break;

#if CURVE_MOTION_ENABLED
        case TWI_CMD_CURVE_MOTION_ENABLE:

            // Enable curve motion handling.
            motion_enable();

            break;

        case TWI_CMD_CURVE_MOTION_DISABLE:

            // Disable curve motion handling.
            motion_disable();

            break;

        case TWI_CMD_CURVE_MOTION_RESET:

            // Reset the motion to the current position.
            motion_reset(adc_get_position_value());

            break;

        case TWI_CMD_CURVE_MOTION_APPEND:

            // Append motion curve data stored in the registers.
            motion_append();

            break;
#endif
        case TWI_CMD_GCALL_ENABLE:

            // Enable the general call functionality
            general_call_enable();
            break;

        case TWI_CMD_GCALL_DISABLE:

            // Disable General call functionaility
            general_call_disable();

            break;

        case TWI_CMD_GCALL_START_MOVE:

            // start the general call movement
            general_call_start_move();

            break;

        case TWI_CMD_GCALL_START_WAIT:

            // dont move unless we get the start command or the group command
            general_call_start_wait();

            break;

        case TWI_CMD_PWM_BRAKE_ENABLE:
#if PWM_ENH_ENABLED
            pwm_brake_enable();
#endif

            break;

        case TWI_CMD_PWM_BRAKE_DISABLE:
#if PWM_ENH_ENABLED
            pwm_brake_disable();
#endif

            break;

        default:

            // Ignore unknown command.
            break;
    }
}


int main (void)
{
    // Configure pins to the default states.
    config_pin_defaults();

    // Initialize the watchdog module.
    watchdog_init();

    // First, initialize registers that control servo operation.
    registers_init();

#if PWM_STD_ENABLED || PWM_ENH_ENABLED
    // Initialize the PWM module.
    pwm_init();
#endif

#if STEP_ENABLED
    // Initialise the stepper motor
    step_init();
#endif
    
    // Initialize the ADC module.
    adc_init();

    // Initialise the Heartbeart
    heartbeat_init();

    // Initialize the PID algorithm module.
    pid_init();

#if CURVE_MOTION_ENABLED
    // Initialize curve motion module.
    motion_init();
#endif

    // Initialize the power module.
    power_init();

#if PULSE_CONTROL_ENABLED
    pulse_control_init();
#endif

#if BACKEMF_ENABLED
    // Initialise the back emf module
    backemf_init();
#endif

#if ALERT_ENABLED
    //initialise the alert registers
    alert_init();
#endif

    // Initialize the TWI slave module.
    twi_slave_init(banks_read_byte(POS_PID_BANK, REG_TWI_ADDRESS));

    // Finally initialize the timer.
    timer_set(0);

    // Enable interrupts.
    sei();

    // Trigger the adc sampling hardware
    adc_start(ADC_CHANNEL_POSITION);

    // Wait until initial position value is ready.
    while (!adc_position_value_is_ready());

#if CURVE_MOTION_ENABLED
    // Reset the curve motion with the current position of the servo.
    motion_reset(adc_get_position_value());
#endif

    // Set the initial seek position and velocity.
    registers_write_word(REG_SEEK_POSITION_HI, adc_get_position_value());
    registers_write_word(REG_SEEK_VELOCITY_HI, 0);

    // XXX Enable PWM and writing.  I do this for now to make development and
    // XXX tuning a bit easier.  Constantly manually setting these values to
    // XXX turn the servo on and write the gain values get's to be a pain.
#if PWM_STD_ENABLED || PWM_ENH_ENABLED
    pwm_disable();
#endif
#if STEP_ENABLED
    step_enable();
#endif

    registers_write_enable();

    // This is the main processing loop for the servo.  It basically looks
    // for new position, power or TWI commands to be processed.
    for (;;)
    {
        static uint8_t emf_motor_is_coasting = 0;

        // Is the system heartbeat ready?
        if (heartbeat_is_ready())
        {
            static int16_t last_seek_position;
            static int16_t wait_seek_position;
            static int16_t new_seek_position;

            // Clear the heartbeat flag
            heartbeat_value_clear_ready();

#if PULSE_CONTROL_ENABLED
            // Give pulse control a chance to update the seek position.
            pulse_control_update();
#endif

#if CURVE_MOTION_ENABLED
            // Give the motion curve a chance to update the seek position and velocity.
            motion_next(10);
#endif

            // General call support
            // Check to see if we have the wait flag enabled. If so save the new position, and write in the
            // old position until we get the move command
            if (general_call_enabled()) 
            {
                //we need to wait for the go command before moving
                if (general_call_wait())
                {
                    // store the new position, but let the servo lock to the last seek position
                    wait_seek_position = (int16_t) registers_read_word(REG_SEEK_POSITION_HI);
                    if (wait_seek_position != last_seek_position) // do we have a new position?
                    {
                        new_seek_position = wait_seek_position;
                        registers_write_word(REG_SEEK_POSITION_HI, last_seek_position);
                    }
                }
                last_seek_position = registers_read_word(REG_SEEK_POSITION_HI);

                //check to make sure that we can start the move.
                if (general_call_start() || 
                    ( registers_read_byte(REG_GENERAL_CALL_GROUP_START) == banks_read_byte(CONFIG_BANK, REG_GENERAL_CALL_GROUP)))
                {
                    // write the new position with the previously saved position
                    registers_write_word(REG_SEEK_POSITION_HI, new_seek_position);  
                    general_call_start_wait_reset();  // reset the wait flag
                    general_call_start_reset();  // reset the start flag
                }
            }

#if BACKEMF_ENABLED
            // Quick and dirty check to see if pwm is active. This is done to make sure the motor doesn't
            // whine in the audible range while idling.
            uint8_t pwm_a = registers_read_byte(REG_PWM_DIRA);
            uint8_t pwm_b = registers_read_byte(REG_PWM_DIRB);
            if (pwm_a || pwm_b)
            {
                // Disable PWM
                backemf_coast_motor();
                emf_motor_is_coasting = 1;
            }
            else
            {
                // reset the back EMF value to 0
                banks_write_word(INFORMATION_BANK, REG_BACKEMF_HI, 0);
                emf_motor_is_coasting = 0;
            }
#endif

#if ADC_ENABLED
            // Trigger the adc sampling hardware. This triggers the position and temperature sample
            adc_start(ADC_FIRST_CHANNEL);
#endif

        }
    
    
        // Wait for the samples to complete
#if TEMPERATURE_ENABLED
        if (adc_temperature_value_is_ready())
        {
            // Save temperature value to registers
            registers_write_word(REG_TEMPERATURE_HI, (uint16_t)adc_get_temperature_value());
        }
#endif
#if CURRENT_ENABLED
        if (adc_power_value_is_ready())
        {

            // Get the new power value.
            uint16_t power = adc_get_power_value();

            // Update the power value for reporting.
            power_update(power);
        }
#endif
#if ADC_POSITION_ENABLED
        if (adc_position_value_is_ready())
        {
            int16_t position;
            // Get the new position value from the ADC module.
            position = (int16_t) adc_get_position_value();
#else
        if (position_value_is_ready())
        {
            int16_t position;
            // Get the position value from an external module.
            position = (int16_t) get_position_value();
#endif
            int16_t pwm;
            uint8_t braking;
#if BACKEMF_ENABLED
            if (emf_motor_is_coasting == 1)
            {
                uint8_t pwm_a = registers_read_byte(REG_PWM_DIRA);
                uint8_t pwm_b = registers_read_byte(REG_PWM_DIRB);

                // Quick and dirty check to see if pwm is active
                if (pwm_a || pwm_b)
                {
                    // Get the backemf sample.
                    backemf_get_sample();

                    // Turn the motor back on
                    backemf_restore_motor();
		    emf_motor_is_coasting = 0;
                }
            }
#endif

            // Call the PID algorithm module to get a new PWM value.
            pwm = pid_position_to_pwm(position,&braking);

#if ALERT_ENABLED
            // Update the alert status registers and do any throttling
            alert_check();
#endif

            // Allow any alerts to modify the PWM value.
            pwm = alert_pwm_throttle(pwm);

#if PWM_STD_ENABLED || PWM_ENH_ENABLED
            // Update the servo movement as indicated by the PWM value.
            // Sanity checks are performed against the position value.
            pwm_update(position, pwm, braking);
#endif

#if STEP_ENABLED
            // Update the stepper motor as indicated by the PWM value.
            // Sanity checks are performed against the position value.
            step_update(position, pwm);
#endif
        }
    
        // Was a command recieved?
        if (twi_data_in_receive_buffer())
        {
            // Handle any TWI command.
            handle_twi_command();
        }

        // Update the bank register operations
        banks_update_registers();

#if MAIN_MOTION_TEST_ENABLED
        // This code is in place for having the servo drive itself between
        // two positions to aid in the servo tuning process.  This code
        // should normally be disabled in config.h.
#if CURVE_MOTION_ENABLED
        if (motion_time_left() == 0)
        {
            registers_write_word(REG_CURVE_DELTA_HI, 2000);
            registers_write_word(REG_CURVE_POSITION_HI, 0x0100);
            motion_append();
            registers_write_word(REG_CURVE_DELTA_HI, 1000);
            registers_write_word(REG_CURVE_POSITION_HI, 0x0300);
            motion_append();
            registers_write_word(REG_CURVE_DELTA_HI, 2000);
            registers_write_word(REG_CURVE_POSITION_HI, 0x0300);
            motion_append();
            registers_write_word(REG_CURVE_DELTA_HI, 1000);
            registers_write_word(REG_CURVE_POSITION_HI, 0x0100);
            motion_append();
        }
#else
        {
            // Get the timer.
            uint16_t timer = timer_get();

            // Reset the timer if greater than 800.
            if (timer > 800) timer_set(0);

            // Look for specific events.
            if (timer == 0)
            {
                registers_write_word(REG_SEEK_POSITION_HI, 0x0100);
            }
            else if (timer == 400)
            {
                registers_write_word(REG_SEEK_POSITION_HI, 0x0300);
            }
        }
#endif
#endif
    }

    return 0;
}

