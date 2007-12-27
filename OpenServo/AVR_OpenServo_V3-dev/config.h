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
#define DEFAULT_PID_PGAIN               0x0800
#define DEFAULT_PID_DGAIN               0x0C00
#define DEFAULT_PID_IGAIN               0x0000
#define DEFAULT_PID_DEADBAND            0x01

// Specify default mininimum and maximum seek positions.  The OpenServo will
// not attempt to seek beyond these positions.
#define DEFAULT_MIN_SEEK                0x0060
#define DEFAULT_MAX_SEEK                0x03A0

// Default pwm frequency divider.
#define DEFAULT_PWM_FREQ_DIVIDER        0x0040

// Default maximum PWM value as percentage
#define DEFAULT_PWM_MAX                  70

#elif (HARDWARE_TYPE == HARDWARE_TYPE_FUTABA_S3003)

// Futaba S3003 hardware default PID gains.
#define DEFAULT_PID_PGAIN               0x0600
#define DEFAULT_PID_DGAIN               0x0C00
#define DEFAULT_PID_IGAIN               0x0000
#define DEFAULT_PID_DEADBAND            0x01

// Futaba S3003 hardware default mininimum and maximum seek positions.
#define DEFAULT_MIN_SEEK                0x0060
#define DEFAULT_MAX_SEEK                0x03A0

// Futaba S3003 hardware default pwm frequency divider.
#define DEFAULT_PWM_FREQ_DIVIDER        0x0040

// Default maximum PWM value as percentage
#define DEFAULT_PWM_MAX                  100

#elif (HARDWARE_TYPE == HARDWARE_TYPE_HITEC_HS_311)

// Hitec HS-311 hardware default PID gains.
#define DEFAULT_PID_PGAIN               0x0600
#define DEFAULT_PID_DGAIN               0x0C00
#define DEFAULT_PID_IGAIN               0x0000
#define DEFAULT_PID_DEADBAND            0x01

// Hitec HS-311 hardware default mininimum and maximum seek positions.
#define DEFAULT_MIN_SEEK                0x0060
#define DEFAULT_MAX_SEEK                0x03A0

// Hitec HS-311 hardware default pwm frequency divider.
#define DEFAULT_PWM_FREQ_DIVIDER        0x0040

// Default maximum PWM value as percentage
#define DEFAULT_PWM_MAX                  100

#elif (HARDWARE_TYPE == HARDWARE_TYPE_HITEC_HS_475HB)

// Hitec HS-475HB hardware default PID gains.
#define DEFAULT_PID_PGAIN               0x0600
#define DEFAULT_PID_DGAIN               0x0C00
#define DEFAULT_PID_IGAIN               0x0000
#define DEFAULT_PID_DEADBAND            0x01

// Hitec HS-475HB hardware default mininimum and maximum seek positions.
#define DEFAULT_MIN_SEEK                0x0060
#define DEFAULT_MAX_SEEK                0x03A0

// Hitec HS-475HB hardware default pwm frequency divider.
#define DEFAULT_PWM_FREQ_DIVIDER        0x0040

// Default maximum PWM value as percentage
#define DEFAULT_PWM_MAX                  100

#endif

#endif // _OS_ADC_H_
