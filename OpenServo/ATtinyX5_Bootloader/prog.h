/*
   Copyright (c) 2005, Mike Thompson <mpthompson@gmail.com>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
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

#ifdef __AVR_ATtiny85__

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
