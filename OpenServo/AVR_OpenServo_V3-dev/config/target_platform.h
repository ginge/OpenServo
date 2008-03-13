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

#ifndef _OS_TARGET_PLATORM_H_
#define _OS_TARGET_PLATORM_H_

/*
 * The hardware you will be using the OpenServo on is defined below
 * You can change the hardware to suit your application here.
 * To add your own hardware, add a new hardware device here, and 
 * define the template file to load in ../config.h
 *
 */

// The known OpenServo compatible hardware is defined below. Choose
// your correct platform hardware here.
#define TARGET_HARDWARE_UNKNOWN     0
#define TARGET_HARDWARE_OSV2        1
#define TARGET_HARDWARE_OSV3        2
#define TARGET_HARDWARE_RFOPENSTEP  8

// The known OpenServo servo hardware types are listed below.
// Choose one of these when using the hobby servo replacements 
// boards.
#define HARDWARE_TYPE_UNKNOWN           0
#define HARDWARE_TYPE_FUTABA_S3003      1
#define HARDWARE_TYPE_HITEC_HS_311      2
#define HARDWARE_TYPE_HITEC_HS_475HB    3


// Default to an OpenServo Version3 which is that latest santioned 
// hardware revision. If you want to compile for a different hardware
// target, such as the OpenServo Version 2, or the OpenStep modules
// you change this definition.
#define TARGET_HARDWARE                 TARGET_HARDWARE_OSV3

// By default the hardware type is unknown.  This value should be
// changed to reflect the hardware type that the code is actually
// being compiled for. You can add new hardware and edit parameters
// by using the servo_hardware.h and motor_hardware.h files, which 
// define the type of motor you are using.
#define HARDWARE_TYPE                   HARDWARE_TYPE_UNKNOWN

#endif
