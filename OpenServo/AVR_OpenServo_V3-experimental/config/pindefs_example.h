/*
    Copyright (c) 2008 Barry Carter <mpthompson@gmail.com>

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
#ifndef _OS_PINDEFS_H_
#define _OS_PINDEFS_H_ 1

/******************************************************************************
 *                                                                            *
 *   This file is provided as a hardware template to allow you to connect the *
 *   OpenServo Core to hardware of your choosing. You can enable and disable  *
 *   modules to fit the features you want and need and the clock frequency    *
 *   you specify.                                                             *
 *                                                                            *
 *****************************************************************************/



/*
 * This section is to override any configuration options needed in your
 * hardware. Every feature available can be enabled or disabled here.
 * You can remove any options you don't want to use. Presented below
 * are the default options for an OpenServo Version 3 hardware.
 */

// You can swap these to change the type of hardware output and switch
// between the stepper controller and the standard H-Bridge.
// Enable (1) or disable (0) checksum verification within the
// twi.c module.  When enabled the TWI_CMD_CHECKED_TXN command
// is enabled and basic checksum validation of reads and writes
// of registers can be made for more robust communication with
// the OpenServo.  The checksum code consumes about 280 bytes
// of Flash code space when enabled.
#define TWI_CHECKED_ENABLED         1

// Enable (1) or disable (0) cubic Hermite curve following motion
// functions.  These functions allow the OpenServo to be controlled
// by keypoints along a cubic Hermite curve with each keypoint
// indicating servo position and velocity at a particular time.
#define CURVE_MOTION_ENABLED        1

// Enable (1) or disable (0) some test motion code within the
// main.c module.  This test code can be enabled to test basic
// positioning of the OpenServo without a TWI master controlling
// the OpenServo.  It should normally be disabled.
#define MAIN_MOTION_TEST_ENABLED    0

// Enable (1) or disable (0) standard servo pulse control signaling
// of the seek position.  As of 2/2/07 this feature is still under
// active development.  Please visit the OpenServo forums for the
// current status of this feature.
#define PULSE_CONTROL_ENABLED       0

// Enable (1) or disable (0) the swapping of PWM output A and B.
// This swapping must sometimes enabled depending on whether the
// positive lead to the motor is attached to MOSFET/PWM output A
// or MOSFET/PWM output B.  This option makes this easy to control
// within software.
#define SWAP_PWM_DIRECTION_ENABLED  0

// Enable (1) or disable (0) the rolling subtype. The subtype will
// change string on each read
#define ROLLING_SUBTYPE             1

// Enable (1) or disable (0) the back EMF speed measurement module
// Enabling this function introduces an off period of the PWM
// output which means you will need to recalibrate the PID and PWM
// module variables.
#define BACKEMF_ENABLED             1

// Enable (1) or disable (0) The Standard PWM module. The 
// "standard" PWM module is the same used in the Version 2 OpenServo
// and the most compatible
#define PWM_STD_ENABLED             0

// Enable (1) or disable (0) The Enhanced Version 3 style PWM 
// module control.
#define PWM_ENH_ENABLED             1

// Enable (1) or disable (0) The Stepper motor controller interface.
// This allows for a standard unipolar stepper motor to connect to 
// a suitable interface.
#define STEP_ENABLED                0

// Enable (1) or disable (0) The temperature sensing thermometer.
#define TEMPERATURE_ENABLED         1

/*
 * Config for the PWM control output pins
 * These include the bridge enable and PWM sample enable pins.
 * This only works with the PM_ENH_ENABLE option (above)
 */
//  PB1/OC1A - PWM_A    - PWM pulse output direction A
//  PB2/OC1B - PWM_B    - PWM pulse output direction B
//  PD2      - EN_A     - PWM enable direction A
//  PD3      - EN_B     - PWM enable direction B
//  PD4      - SMPLn_B  - BEMF sample enable B
//  PD7      - SMPLn_A  - BEMF sample enable A
//
#define PWM_CTL_PORT      PORTD
#define PWM_CTL_DDR       DDRD
#define PWM_CRL_DDR_CONF  ((1<<DDD2) | (1<<DDD3) | (1<<DDD4) | (1<<DDD7))
#define PWM_PIN_SMPLA     PD7
#define PWM_PIN_SMPLB     PD4
#define PWM_PIN_ENA       PD2
#define PWM_PIN_ENB       PD3

/*
 * Configs for the PWM output pins. These are the pins that the hardware
 * PWM is on the target AVR device. This will mostly be the same as below
 * unless you are not using an AtMega168
 */
#define PWM_DUTY_PORT     PORTB
#define PWM_DUTY_DDR      DDRB
#define PWM_DUTY_DDR_CONF ((1<<DDB1) | (1<<DDB2))
#define PWM_DUTY_PWMA     PB1
#define PWM_DUTY_PWMB     PB2

/*
 * ADC control pins, and pin assignments. Select the correct pin for
 * the feature you want to implement. Some options are not available
 * is the module is disabled, such as BackEMF and Temperature modules.
 */
// Defines for the power and position channels.
// The channel the connection is on should be defined here
//  ADC0 (PC0) - Motor Current
//  ADC1 (PC1) - Battery Voltage
//  ADC2 (PC2) - Position
//  ADC3 (PC3) - Temperature
//  ADC7 (PC7) - Back EMF
#define ADC_CHANNEL_CURRENT         0
#define ADC_CHANNEL_BATTERY         1
#define ADC_CHANNEL_POSITION        2
#define ADC_CHANNEL_TEMPERATURE     3
#define ADC_CHANNEL_BACKEMF         7
#define ADC_PORT                    PORTC
#define ADC_IO_PINS                 ((1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3))
#define ADC_DIDR                    DIDR0
#define ADC_DIGITAL                 (1<<ADC0D) | (1<<ADC1D) | (1<<ADC2D) | (1<<ADC3D)

/*
 * RC PWM input pin assignments and options. This feature allows for RC
 * PWM input to control the OpenServo. You can use this feature on any AVR
 * pin that support pin interrupts such as PCINT3 (pin interrupt 3)
 */
#define RCPULSE_PORT                PORTB
#define RCPULSE_PIN                 PINB
#define RCPULSE_DDR                 DDRB
#define RCPULSE_DDR_DIR             DDB3
#define RCPULSE_INPUT_PIN           PB3
#define RCPULSE INPUT_PIN_I         PINB3
#define RCPULSE_INT_PIN             PCINT3

/*
 * System Clock defines. These allow control over the heartbeat speed of
 * the system. It is implemented using the 16 bit timer, so you can choose
 * a clock value to suit the CPU speed and hardware.
 * You can calculate the  CRVALUE and prescaler using this formula
 * Target Timer Count = (Input Frequency / Prescale) / Target Frequency)
 */
// The ADC clock prescaler of 128 is selected to yield a 156.25 KHz ADC clock
// from an 20 MHz system clock.
#define ADPS		((1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0))

// The timer clock prescaler of 1024 is selected to yield a 19.531 KHz ADC clock
// from a 20 MHz system clock.
#define CSPS		((1<<CS02) | (0<<CS01) | (1<<CS00))

// Define the compare register value to generate a timer interrupt and initiate
// an ADC sample every 9.984 milliseconds and yield a 100.1603 Hz sample rate.
#define CRVALUE		195


// Define the frequency of the system heartbeat in ms. This is an approximation
// used by the motion.c module for the time step intervals.
#define HEARTBEAT_INTERVAL_VALUE  10

// Set a CPU speed for the various delay functions.
#define F_CPU 20000000L

#endif
