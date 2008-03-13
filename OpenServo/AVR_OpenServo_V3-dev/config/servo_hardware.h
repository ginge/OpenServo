/*
    Copyright (c) 2008 Barry Carter <barry.carter@robotfuzz.com>

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
 * This file allows you to tune the configuration for your servo. If
 * you do change this file, please make a note and tell us in the forums
 * so we may include it in the next revision
 */

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
