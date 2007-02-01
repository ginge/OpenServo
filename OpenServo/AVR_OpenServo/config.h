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

#ifndef _OS_CONFIG_H_
#define _OS_CONFIG_H_ 1

// This file is intended to consolidate configuration settings
// for the OpenServo software into a single location. Enabling 
// certain combinations of settings may result in a servo that
// doesn't function properly or be too large to fit within the
// 3K Flash limit for applications on the ATtiny45 MCU in the
// OpenServo. Where such limitations are known, comments should
// the placed within this file.

// Enable (1) or disable (0) checksum verification within the 
// twi.c module.  When enabled the TWI_CMD_CHECKED_TXN command
// is enabled and basic checksum validation of reads and writes
// of registers can be made for more robust communication with
// the OpenServo.  The checksum code consumes about 280 bytes
// of Flash code space when enabled.
#define TWI_CHECKED_ENABLED         1

// Enable (1) or disable (0) the PID algorithm for motion 
// control in the motion.c module.  This setting cannot be
// set when the other XXX_MOTION_ENABLED flags are set.
//
// NOTE: This is the motion control algorithm most people should
// use until the other algorithms are further developed.
#define PID_MOTION_ENABLED          1

// Enable (1) or disable (0) the IPD algorithm for motion 
// control in the motion.c module.  This setting cannot be
// set when the other XXX_MOTION_ENABLED flags are set.
//
// NOTE: The IPD algorithm is still under development and is
// currently unstable.  Please contact Mike Thompson in the 
// OpenServo forums for more information before enabling this
// feature.
#define IPD_MOTION_ENABLED          0

// Enable (1) or disable (0) the state regulator algorithm
// for motion control in the motion.c module.  This setting
// cannot be set when the other XXX_MOTION_ENABLED flags are
// set.
//
// NOTE: The state regulator code is still under development.  
// Please contact Stefan Engelke in the OpenServo forums for 
// more information before enabling this feature.
#define REGULATOR_MOTION_ENABLED    0

// Enable (1) or disable (0) the Luenberg state estimator 
// algorithm for determining servo speed.  It is a realtime 
// simulation of the servo behavior with a internal controller 
// which keeps track of the difference between the simulated 
// position and the measured one.  This must be enabled when
// the MOTION_REGULATOR_ENABLED flag is enabled.  
#define ESTIMATOR_ENABLED           (REGULATOR_MOTION_ENABLED)

// Enable (1) or disable (0) fixed point utility functions in 
// the math.c module.  Currently these routines are only used
// by the state estimator and state regulator.
#define FIXED_MATH_ENABLED          (ESTIMATOR_ENABLED || REGULATOR_ENABLED)

// Enable (1) or disable (0) cubic Hermite curve following motion
// functions.  These functions allow the OpenServo to be controlled
// by keypoints along a cubic Hermite curve with each keypoint
// indicating servo position and velocity at a particular time.
// As of Jan. 27th, 2007 this feature is not yet fully implemented
// and should remain disabled.
#define CURVE_MOTION_ENABLED        1

// Enable (1) or disable (0) some test motion code within the
// main.c module.  This test code can be enabled to test basic
// positioning of the OpenServo without a TWI master controlling
// the OpenServo.  It should normally be disabled.
#define MAIN_MOTION_TEST_ENABLED    0

// Enable (1) or disable (0) the swapping of PWM output A and B.
// This swapping must sometimes enabled depending on whether the
// positive lead to the motor is attached to MOSFET/PWM output A
// or MOSFET/PWM output B.  This option makes this easy to control
// within software.
#define SWAP_PWM_DIRECTION_ENABLED  0

// Perform some sanity check of settings here.
#if PID_MOTION_ENABLED && (IPD_MOTION_ENABLED || REGULATOR_MOTION_ENABLED)
#  error "Conflicting configuration settings for PID_MOTION_ENABLED"
#endif
#if IPD_MOTION_ENABLED && (PID_MOTION_ENABLED || REGULATOR_MOTION_ENABLED)
#  error "Conflicting configuration settings for MOTION_IPD_ENABLED"
#endif
#if REGULATOR_MOTION_ENABLED && (PID_MOTION_ENABLED || IPD_MOTION_ENABLED)
#  error "Conflicting configuration settings for REGULATOR_MOTION_ENABLED"
#endif
#if REGULATOR_MOTION_ENABLED && !ESTIMATOR_ENABLED
#  error "Configuration settings for REGULATOR_MOTION_ENABLED requires ESTIMATOR_ENABLED."
#endif

// The known OpenServo hardware types are listed below.
#define HARDWARE_TYPE_UNKNOWN           0
#define HARDWARE_TYPE_FUTABA_S3003      1
#define HARDWARE_TYPE_HITEC_HS_311      2
#define HARDWARE_TYPE_HITEC_HS_475HB    3

// By default the hardware type is unknown.  This value should be 
// changed to reflect the hardware type that the code is actually 
// being compiled for.
#define HARDWARE_TYPE                   HARDWARE_TYPE_UNKNOWN

// Set configuration values based on HARDWARE_TYPE.
#if (HARDWARE_TYPE == HARDWARE_TYPE_UNKNOWN)

// By default the PID gains are set to zero because they are the 
// safest default when implementing an OpenServo on a new hardware.  
// These defaults should be overriden by specifying the HARDWARE_TYPE
// above for actual known servo hardware.
#define DEFAULT_PID_PGAIN               0x0000
#define DEFAULT_PID_DGAIN               0x0000
#define DEFAULT_PID_IGAIN               0x0000

// Specify default mininimum and maximum seek positions.  The OpenServo will
// not attempt to seek beyond these positions.
#define DEFAULT_MIN_SEEK                0x0060
#define DEFAULT_MAX_SEEK                0x03A0

#elif (HARDWARE_TYPE == HARDWARE_TYPE_FUTABA_S3003)

// Futaba S3003 hardware default PID gains.
#define DEFAULT_PID_PGAIN               0x0600
#define DEFAULT_PID_DGAIN               0x0800
#define DEFAULT_PID_IGAIN               0x0000

// Futaba S3003 hardware default mininimum and maximum seek positions.
#define DEFAULT_MIN_SEEK                0x0060
#define DEFAULT_MAX_SEEK                0x03A0

#elif (HARDWARE_TYPE == HARDWARE_TYPE_HITEC_HS_311)

// Hitec HS-311 hardware default PID gains.
#define DEFAULT_PID_PGAIN               0x0600
#define DEFAULT_PID_DGAIN               0x0800
#define DEFAULT_PID_IGAIN               0x0000

// Hitec HS-311 hardware default mininimum and maximum seek positions.
#define DEFAULT_MIN_SEEK                0x0060
#define DEFAULT_MAX_SEEK                0x03A0

#elif (HARDWARE_TYPE == HARDWARE_TYPE_HITEC_HS_475HB)

// Hitec HS-475HB hardware default PID gains.
#define DEFAULT_PID_PGAIN               0x0600
#define DEFAULT_PID_DGAIN               0x0800
#define DEFAULT_PID_IGAIN               0x0000

// Hitec HS-475HB hardware default mininimum and maximum seek positions.
#define DEFAULT_MIN_SEEK                0x0060
#define DEFAULT_MAX_SEEK                0x03A0

#endif

#endif // _OS_ADC_H_
