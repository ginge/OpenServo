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

#ifndef _PROG_H_
#define _PROG_H_ 1

// These constants define the page configuration based on the Attiny25/45/85 MPU.
// The the Flash and EEPROM pages that can be written are represented as a
// contiguous address space with the Flash spanning the lower pages and the
// EEPROM spanning the upper pages.  It is up to the end user to keep track of
// which type and size of the page being programmed.  Page and address maps
// are shown below.
//
//  ATtiny25 Flash/EEPROM Map
//
//  Page     Address    Type
//  --------------------------
//   00     0000-001F   Flash
//   ..
//   1F     03E0-03FF   Flash
//   20     0400-041F   Flash (R/O - Bootloader)
//   ..
//   3F     07E0-07FF   Flash (R/O - Bootloader)
//   40     0800-081F   EEPROM
//   ..
//   43     0860-087F   EEPROM
//

#ifdef __AVR_ATtiny25__

// Flash/EEPROM Page Information
#define FLASH_PAGE_COUNT        (64)
#define EEPROM_PAGE_COUNT       (4)
#define PROG_PAGE_SIZE          (32)

// Flash/EEPROM Address Information
#define PROG_FLASH_START        (0x0000)
#define PROG_FLASH_BOOTLOADER   (0x0400)
#define PROG_FLASH_END          (0x0800)
#define PROG_EEPROM_START       (0x0800)
#define PROG_EEPROM_END         (0x0880)

#endif

//  ATtiny45 Flash/EEPROM Map
//
//  Page     Address    Type
//  --------------------------
//   00     0000-003F   Flash
//   ..
//   2F     0BC0-0BFF   Flash
//   30     0C00-0C3F   Flash (R/O - Bootloader)
//   ..
//   3F     0FC0-0FFF   Flash (R/O - Bootloader)
//   40     1000-103F   EEPROM
//   ..
//   43     10C0-10FF   EEPROM

#ifdef __AVR_ATtiny45__

// Flash/EEPROM Page Information
#define FLASH_PAGE_COUNT        (64)
#define EEPROM_PAGE_COUNT       (4)
#define PROG_PAGE_SIZE          (64)

// Flash/EEPROM Address Information
#define PROG_FLASH_START        (0x0000)
#define PROG_FLASH_BOOTLOADER   (0x0C00)
#define PROG_FLASH_END          (0x1000)
#define PROG_EEPROM_START       (0x1000)
#define PROG_EEPROM_END         (0x1100)

#endif

//  ATtiny85 Flash/EEPROM Map
//
//  Page     Address    Type
//  --------------------------
//   00     0000-003F   Flash
//   ..
//   6F     1BC0-1BFF   Flash
//   70     1C00-1C3F   Flash (R/O - Bootloader)
//   ..
//   7F     1FC0-1FFF   Flash (R/O - Bootloader)
//   80     2000-203F   EEPROM
//   ..
//   87     21C0-21FF   EEPROM

#if defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny84__)

// Flash/EEPROM Page Information
#define FLASH_PAGE_COUNT        (128)
#define EEPROM_PAGE_COUNT       (8)
#define PROG_PAGE_SIZE          (64)

// Flash/EEPROM Address Information
#define PROG_FLASH_START        (0x0000)
#define PROG_FLASH_BOOTLOADER   (0x1C00)
#define PROG_FLASH_END          (0x2000)
#define PROG_EEPROM_START       (0x2000)
#define PROG_EEPROM_END         (0x2200)

#endif

extern void prog_init(void);
extern void prog_buffer_set_address(uint16_t address);
extern uint8_t prog_buffer_get_byte(void);
extern void prog_buffer_set_byte(uint8_t databyte);
extern void prog_buffer_update(void);

#endif // _PROG_H_
