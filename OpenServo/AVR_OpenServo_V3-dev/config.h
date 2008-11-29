/*
    Copyright (c) 2008 Barry Carter <barry.carter@robotfuzz.com>
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

#ifndef _OS_CONFIG_H_
#define _OS_CONFIG_H_ 1

/*
 * This file contains overridable defaults for the OpenServo hardware
 * This also serves to load up the config files of the hardware available
 * in the config/ directory.
 */
#include "config/target_platform.h"

// By default the hardware type is unknown.  This value should be
// changed to reflect the hardware type that the code is actually
// being compiled for.
#ifndef HARDWARE_TYPE
#define HARDWARE_TYPE                   HARDWARE_TYPE_UNKNOWN
#endif

// By default the target platform is unknown, and is defined to be an
// OpenServo version 3. This should probably use dummy hardware?!?
#ifndef TARGET_HARDWARE
#define TARGET_HARDWARE                 TARGET_HARDWARE_OSV3
#endif

// Include the motor type definitions
#include "config/motor_hardware.h"
#include "config/servo_hardware.h"

/*
 * Load up any target specific options.
 */
#if (TARGET_HARDWARE == TARGET_HARDWARE_UNKNOWN)

#include "config/pindefs_custom.h"

#elif (TARGET_HARDWARE == TARGET_HARDWARE_OSV2)

#include "config/pindefs_OSv2.h"

#elif (TARGET_HARDWARE == TARGET_HARDWARE_OSV3)

#include "config/pindefs_OSv3.h"

#elif (TARGET_HARDWARE == TARGET_HARDWARE_RFOPENSTEP)

#include "config/pindefs_RobotFuzz_OpenStep.h"

#endif

/*
 * Provide defaults for the enabled modules to allow it to be overridden by a specific platform config file
 */


// Enable (1) or disable (0) checksum verification within the
// twi.c module.  When enabled the TWI_CMD_CHECKED_TXN command
// is enabled and basic checksum validation of reads and writes
// of registers can be made for more robust communication with
// the OpenServo.  The checksum code consumes about 280 bytes
// of Flash code space when enabled.
#ifndef TWI_CHECKED_ENABLED
#define TWI_CHECKED_ENABLED         1
#endif

// Enable (1) or disable (0) cubic Hermite curve following motion
// functions.  These functions allow the OpenServo to be controlled
// by keypoints along a cubic Hermite curve with each keypoint
// indicating servo position and velocity at a particular time.
#ifndef CURVE_MOTION_ENABLED
#define CURVE_MOTION_ENABLED        1
#endif

// Enable (1) or disable (0) some test motion code within the
// main.c module.  This test code can be enabled to test basic
// positioning of the OpenServo without a TWI master controlling
// the OpenServo.  It should normally be disabled.
#ifndef MAIN_MOTION_TEST_ENABLED
#define MAIN_MOTION_TEST_ENABLED    0
#endif

// Enable (1) or disable (0) standard servo pulse control signaling
// of the seek position.  As of 2/2/07 this feature is still under
// active development.  Please visit the OpenServo forums for the
// current status of this feature.
#ifndef PULSE_CONTROL_ENABLED
#define PULSE_CONTROL_ENABLED       0
#endif

// Enable (1) or disable (0) the swapping of PWM output A and B.
// This swapping must sometimes enabled depending on whether the
// positive lead to the motor is attached to MOSFET/PWM output A
// or MOSFET/PWM output B.  This option makes this easy to control
// within software.
#ifndef SWAP_PWM_DIRECTION_ENABLED
#define SWAP_PWM_DIRECTION_ENABLED  0
#endif

// Enable (1) or disable (0) the rolling subtype. The subtype will
// change string on each read
#ifndef ROLLING_SUBTYPE
#define ROLLING_SUBTYPE             1
#endif

// Enable (1) or disable (0) the back EMF speed measurement module
// Enabling this function introduces an off period of the PWM
// output which means you will need to recalibrate the PID and PWM
// module variables.
#ifndef BACKEMF_ENABLED
#define BACKEMF_ENABLED             1
#endif

// Enable (1) or disable (0) The Standard PWM module. The 
// "standard" PWM module is the same used in the Version 2 OpenServo
// and the most compatible
#ifndef PWM_STD_ENABLED
#define PWM_STD_ENABLED             0
#endif

// Enable (1) or disable (0) The Enhanced Version 3 style PWM 
// module control.
#ifndef PWM_ENH_ENABLED
//quick check to make sure it is not enabled is standard is enabled.
#if PWM_STD_ENABLED
#define PWM_ENH_ENABLED             0
#else
#define PWM_ENH_ENABLED             1
#endif
#endif

// Enable (1) or disable (0) The Stepper motor controller interface.
// This allows for a standard unipolar stepper motor to connect to 
// a suitable interface.
#ifndef STEP_ENABLED
#define STEP_ENABLED                0
#endif

// Enable (1) or disable (0) The board mounted thermometer.
#ifndef TEMPERATURE_ENABLED
#define TEMPERATURE_ENABLED         1
#endif

// Enable (1) or disable (0) the Alert based system.
#ifndef ALERT_ENABLED
#define ALERT_ENABLED               1
#endif

#ifndef ALERT_INTN_ENABLED
#define ALERT_INTN_ENABLED          0
#endif

/*
 * ADC Clock defines
 */
// Deault defines for the ADC subsystem. Default to 8MHZ internal xtal.
// The ADC clock prescaler of 64 is selected to yield a 125 KHz ADC clock
// from an 8 MHz system clock.
#ifndef ADPS
#define ADPS		((1<<ADPS2) | (1<<ADPS1) | (0<<ADPS0))
#endif

// The timer clock prescaler of 1024 is selected to yield a 7.8125 KHz ADC clock
// from a 8 MHz system clock.
#ifndef CSPS
#define CSPS		((1<<CS02) | (0<<CS01) | (1<<CS00))
#endif

// Define the compare register value to generate a timer interrupt and initiate
// an ADC sample every 9.987 milliseconds and yield a 100.1603 Hz sample rate.
#ifndef CRVALUE 
#define CRVALUE		78
#endif

// Define the frequency of the system heartbeat.
#ifndef HEARTBEAT_INTERVAL_VALUE 
#define HEARTBEAT_INTERVAL_VALUE  10
#endif

// Set a CPU speed. Assume 8mhz
#ifndef F_CPU 
#define F_CPU 8000000L
#endif

#endif // _OS_ADC_H_
