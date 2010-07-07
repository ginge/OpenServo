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
#define DEFAULT_PID_PGAIN               0x0600
#define DEFAULT_PID_DGAIN               0x0100
#define DEFAULT_PID_IGAIN               0x0000
#define DEFAULT_PID_DEADBAND            0x04

// Specify default mininimum and maximum seek positions.  The OpenServo will
// not attempt to seek beyond these positions.
#define DEFAULT_MIN_SEEK                0x0060
#define DEFAULT_MAX_SEEK                0x03A0

// Default pwm frequency divider.
#define DEFAULT_PWM_FREQ_DIVIDER        0x0040

// Default maximum PWM value as percentage
#define DEFAULT_PWM_MAX                  70

#define DEFAULT_PID_SPD_PGAIN           0x006F
#define DEFAULT_PID_SPD_DGAIN           0x0000
#define DEFAULT_PID_SPD_IGAIN           0x000F

#define DEFAULT_MAX_SPEED               0x03FF

#define DEFAULT_ANTI_WINDUP             0x03A0
#define DEFAULT_FEED_FORWARD            0x0033
#define DEFAULT_EMF_COLLAPSE_DELAY      0x03
#define DEFAULT_EMF_CHARGE_TIME         0x03

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

#elif (HARDWARE_TYPE == HARDWARE_TYPE_MG995_2007) // TowerPro MG995 (Nov 2007)

#define DEFAULT_PID_PGAIN               0x0460
#define DEFAULT_PID_IGAIN               0x0080
#define DEFAULT_PID_DGAIN               0x0140
#define DEFAULT_PID_DEADBAND            0x10
#define DEFAULT_MIN_SEEK                0x0028 // 40
#define DEFAULT_MAX_SEEK                0x03B6 // 950
#define DEFAULT_PWM_FREQ_DIVIDER        0x0060
#define DEFAULT_PWM_MAX                 0x60
#define DEFAULT_PID_SPD_PGAIN           0x0000
#define DEFAULT_PID_SPD_DGAIN           0x0000
#define DEFAULT_PID_SPD_IGAIN           0x0000
#define DEFAULT_MAX_SPEED               0x0000
#define DEFAULT_ANTI_WINDUP             0x0000
#define DEFAULT_FEED_FORWARD            0x0000
#define DEFAULT_EMF_COLLAPSE_DELAY      0x03
#define DEFAULT_EMF_CHARGE_TIME         0x03

#elif (HARDWARE_TYPE == HARDWARE_TYPE_MG996R_2009) // TowerPro MG996R (Nov 2009)
#define DEFAULT_PID_PGAIN               0x0460
#define DEFAULT_PID_IGAIN               0x0080
#define DEFAULT_PID_DGAIN               0x0150
#define DEFAULT_PID_DEADBAND            0x10
#define DEFAULT_MIN_SEEK                0x0028 // 40
#define DEFAULT_MAX_SEEK                0x03B6 // 950
#define DEFAULT_PWM_FREQ_DIVIDER        0x0060
#define DEFAULT_PWM_MAX                 0x60
#define DEFAULT_PID_SPD_PGAIN           0x0000
#define DEFAULT_PID_SPD_DGAIN           0x0000
#define DEFAULT_PID_SPD_IGAIN           0x0000
#define DEFAULT_MAX_SPEED               0x0000
#define DEFAULT_ANTI_WINDUP             0x0000
#define DEFAULT_FEED_FORWARD            0x0000
#define DEFAULT_EMF_COLLAPSE_DELAY      0x03
#define DEFAULT_EMF_CHARGE_TIME         0x03

#elif (HARDWARE_TYPE == HARDWARE_TYPE_HITEC_HS_645MG) // Hitec HS-645MG
#define DEFAULT_PID_PGAIN               0x0460
#define DEFAULT_PID_IGAIN               0x0080
#define DEFAULT_PID_DGAIN               0x0130
#define DEFAULT_PID_DEADBAND            0x04
#define DEFAULT_MIN_SEEK                0x0028 // 40
#define DEFAULT_MAX_SEEK                0x03B6 // 950
#define DEFAULT_PWM_FREQ_DIVIDER        0x0060
#define DEFAULT_PWM_MAX                 0x60
#define DEFAULT_PID_SPD_PGAIN           0x0000
#define DEFAULT_PID_SPD_DGAIN           0x0000
#define DEFAULT_PID_SPD_IGAIN           0x0000
#define DEFAULT_MAX_SPEED               0x0000
#define DEFAULT_ANTI_WINDUP             0x0000
#define DEFAULT_FEED_FORWARD            0x0000
#define DEFAULT_EMF_COLLAPSE_DELAY      0x03
#define DEFAULT_EMF_CHARGE_TIME         0x03

#endif
