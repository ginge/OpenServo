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

#ifndef _OS_OPENSERVO_H_
#define _OS_OPENSERVO_H_ 1

// Define the device type and subtype.  These values are used so that
// external TWI clients can query the type of device the servo is.
#define OPENSERVO_DEVICE_TYPE           1
#define OPENSERVO_DEVICE_SUBTYPE        1

// Define the software major and minor version.  These values are used
// so that external TWI clients can query the version of the software
// running on the servo.
#define SOFTWARE_VERSION_MAJOR          3
#define SOFTWARE_VERSION_MINOR          1

// The default TWI address. Change this if you want to change the TWI address of the servo
#define REG_DEFAULT_TWI_ADDR        0x10

#endif // _OS_OPENSERVO_H_
