/*
    Copyright (c) 2007 Michael P. Thompson <mpthompson@gmail.com>

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

// The the Flash and EEPROM pages that can be written are represented as a
// contiguous address space with the Flash spanning the lower pages and the
// EEPROM spanning the upper pages.  It is up to the end user to keep track of
// which type and size of the page being programmed.  Page and address maps
// are shown below.

//  ATmega8 Flash/EEPROM Map
//  ========================
//
//  Page size = 64 bytes
//
//  Page     Address    Type
//  ---------------------------------------------
//   00     0000-003F   Flash
//   ...
//   6F     1BC0-1BFF   Flash
//   70     1C00-1C3F   Flash (bootloader)
//   ...
//   7F     1FC0-1FFF   Flash (bootloader)
//   80     2000-203F   EEPROM
//   ...
//   87     21C0-21FF   EEPROM

#ifdef __AVR_ATmega8__

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

//  ATmega168 Flash/EEPROM Map
//  ==========================
//
//  Page size = 128 bytes
//
//  Page     Address    Type
//  ---------------------------------------------
//   00     0000-007F   Flash
//   ...
//   77     3B80-3BFF   Flash
//   78     3C00-3C7F   Flash (bootloader)
//   ...
//   7F     3FC0-3FFF   Flash (bootloader)
//   80     4000-407F   EEPROM
//   ...
//   83     4180-41FF   EEPROM

#ifdef __AVR_ATmega168__

// Flash/EEPROM Page Information
#define FLASH_PAGE_COUNT        (128)
#define EEPROM_PAGE_COUNT       (4)
#define PROG_PAGE_SIZE          (128)

// Flash/EEPROM Address Information
#define PROG_FLASH_START        (0x0000)
#define PROG_FLASH_BOOTLOADER   (0x3C00)
#define PROG_FLASH_END          (0x4000)
#define PROG_EEPROM_START       (0x4000)
#define PROG_EEPROM_END         (0x4200)

#endif

extern void prog_init(void);
extern void prog_buffer_set_address(uint16_t address);
extern uint8_t prog_buffer_get_byte(void);
extern void prog_buffer_set_byte(uint8_t databyte);
extern void prog_buffer_update(void);

#endif // _PROG_H_
