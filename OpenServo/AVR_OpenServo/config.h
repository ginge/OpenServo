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

// Enable (1) or disable (0) the Luenberg state estimator 
// algorithm for determining servo speed.  It is a realtime 
// simulation of the servo behavior with a internal controller 
// which keeps track of the difference between the simulated 
// position and the measured one.  This must be enabled when
// the MOTION_REGULATOR_ENABLED flag is enabled.  
//
// NOTE: The estimator code is still under development.  Please
// contact Stefan Engelke for more information regarding how
// this code should be used.
#define ESTIMATOR_ENABLED           0

// Enable (1) or disable (0) the PID algorithm for motion 
// control in the motion.c module.  This setting cannot be
// set when the other MOTION_XXX_ENABLED flags are set.
#define MOTION_PID_ENABLED          0

// Enable (1) or disable (0) the IPD algorithm for motion 
// control in the motion.c module.  This setting cannot be
// set when the other MOTION_XXX_ENABLED flags are set.
#define MOTION_IPD_ENABLED          1

// Enable (1) or disable (0) the state regulator algorithm
// for motion control in the motion.c module.  This setting
// cannot be set when the other MOTION_XXX_ENABLED flags are
// set.
//
// NOTE: The state regulator code is still under development.  
// Please contact Stefan Engelke for more information regarding
// how this code should be used.
#define MOTION_REGULATOR_ENABLED    0

// Enable (1) or disable (0) some test motion code within the
// main.c module.  This test code can be enabled to test basic
// positioning of the OpenServo without a TWI master controlling
// the OpenServo.  It should normally be disabled.
#define MAIN_MOTION_TEST_ENABLED    0

// Perform some sanity check of settings here.
#if MOTION_PID_ENABLED && (MOTION_IPD_ENABLED || MOTION_REGULATOR_ENABLED)
#  error "Conflicting configuration settings for MOTION_PID_ENABLED."
#endif
#if MOTION_IPD_ENABLED && (MOTION_PID_ENABLED || MOTION_REGULATOR_ENABLED)
#  error "Conflicting configuration settings for MOTION_IPD_ENABLED"
#endif
#if MOTION_REGULATOR_ENABLED && (MOTION_PID_ENABLED || MOTION_IPD_ENABLED)
#  error "Conflicting configuration settings for MOTION_REGULATOR_ENABLED"
#endif
#if MOTION_REGULATOR_ENABLED && !ESTIMATOR_ENABLED
#  error "Configuration settings for MOTION_REGULATOR_ENABLED requires ESTIMATOR_ENABLED"
#endif

#endif // _OS_ADC_H_
