/*
   Portions of this file derived from boot.h v1.20 from avr-libc project.  
   See the following link for details:

   http://www.nongnu.org/avr-libc/

   Original copyright notice included below:

   Copyright (c) 2002, 2003, 2004, 2005  Eric B. Weddington
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

   Copyright for original code not derived from boot.h:

   Copyright (c) 2005, Mike Thompson <mpthompson@gmail.com>
   All rights reserved.
*/

#undef __AVR_ATtiny2313__
#define __AVR_ATtiny45__

#include <inttypes.h>
#include <avr/io.h>
#include <avr/boot.h>

#include "bootloader.h"
#include "prog.h"

/* For some reason EEARH is undefined in io.h, but it's needed below. */
#ifndef EEARH
#define EEARH _SFR_IO8(0x1F)
#endif

// Programming state flags.
static uint8_t prog_update_flag;

// Programming buffer addresses.
static uint16_t prog_page_address;
static uint16_t prog_byte_address;

// Programming buffer.
static uint8_t prog_buffer[PROG_PAGE_SIZE];

BOOTLOADER_SECTION static void prog_flash_page_write(uint16_t address)
// Write the Flash data at the indicated address from the program buffer.
{
	uint16_t i;
	uint16_t dataword;

	// No need to disable interrupts.

	// No need to make sure EEPROM is not busy.

	// Erase the Flash page at the specified address.
    asm volatile
    (
        "movw r30, %2\n\t"
        "sts %0, %1\n\t"
        "spm\n\t"
        : "=m" (SPMCSR)
        : "r" ((uint8_t) ((1<<PGERS) | (1<<SPMEN))),
          "r" ((uint16_t) address)
        : "r30", "r31"
    );

	// Wait until SPM instruction is finished.
	while (SPMCSR & (1<<SPMEN));

	// Fill the temporary page buffer with the programming buffer.
	for (i = 0; i < PROG_PAGE_SIZE; i += 2)
	{
		// Prepare the little-endian data word.
		dataword = *(uint16_t *)(prog_buffer + i);

		// Write the word to the temporary page buffer.
	    asm volatile
		(
        	"movw  r0, %3\n\t"
        	"movw r30, %2\n\t"
        	"sts %0, %1\n\t"
        	"spm\n\t"
        	"clr  r1\n\t"
        	: "=m" (SPMCSR)
        	: "r" ((uint8_t) (1<<SPMEN)),
          	  "r" ((uint16_t) address + i),
          	  "r" ((uint16_t) dataword)
        	: "r0", "r30", "r31"
    	);
	}

	// Write the page into Flash.
    asm volatile
    (
        "movw r30, %2\n\t"
        "sts %0, %1\n\t"
        "spm\n\t"
        : "=m" (SPMCSR)
        : "r" ((uint8_t) ((1<<PGWRT) | (1<<SPMEN))),
          "r" ((uint16_t) address)
        : "r30", "r31"
    );

	// Wait until SPM instruction is finished.
	while (SPMCSR & (1<<SPMEN));

	// No need to enable interrupts.
}


BOOTLOADER_SECTION static void prog_flash_page_read(uint16_t address)
// Read the Flash data at the indicated address into the program buffer.
{
	uint8_t i;
	uint8_t param;
	uint16_t dataword;
	uint16_t dataaddr = address;

	// No need to disable interrupts.

	// Wait while the SPM instruction is busy.	
	boot_spm_busy_wait();

	// Set the address to read from Flash.
	dataaddr = address;

	// Fill the programming buffer.
	for (i = 0; i < PROG_PAGE_SIZE; i += 2)
	{
		// Set the parameter.
		param = 0x00;

		// Read the Flash word from memory.
		asm volatile
		(
			"movw r30, %3\n\t"
			"sbrc %2,0x00\n\t"
			"sts %0, %2\n\t"
			"lpm\n\t"            // R0 is now the LSB of the return value
			"mov %A1,r0\n\t"
			"inc r30\n\t"
			"lpm\n\t"            // R0 is now the MSB of the return value
			"mov %B1,r0\n\t"  
			: "=m" (SPMCSR),
			  "=r" ((uint16_t) dataword)
			: "r" ((uint8_t) param),
			  "r" ((uint16_t) dataaddr)
			: "r30", "r31", "r0"
		);

		// Write the dataword to the buffer.
		*(uint16_t *)(prog_buffer + i) = dataword;

		// Increment the data address.
		dataaddr += 2;
	}

	// No need to enable interrupts.
}


BOOTLOADER_SECTION static void prog_eeprom_page_write(uint16_t address)
// Read the EEPROM data at the indicated address into the program buffer.
{
	// Use assembly to write to the EEPROM.
	asm volatile (
		"mov r1,%6\n\t"
		"1:\n\t"
		"ld r0,z+\n\t"
		"2:\n\t"
		"sbic %0,%4\n\t"
		"rjmp 2b\n\t"					/* Make sure EEPROM is ready. */
		"out %1,r27\n\t"
		"out %2,r26\n\t"
		"out %3,r0\n\t"
		"adiw r26,1\n\t"				/* Increment x register. */
		"sbi %0,%5\n\t"
		"sbi %0,%4\n\t"
		"dec r1\n\t"
		"brne 1b\n\t"
		:
		: "I" (_SFR_IO_ADDR(EECR)), 
		  "I" (_SFR_IO_ADDR(EEARH)),
		  "I" (_SFR_IO_ADDR(EEARL)),
		  "I" (_SFR_IO_ADDR(EEDR)),
		  "I" (EEWE),
		  "I" (EEMWE),
		  "r"  ((uint8_t) PROG_PAGE_SIZE),
		  "x"  ((uint16_t) address),
		  "z"  ((uint16_t) prog_buffer)
		: "r0", "r1", "memory"
	);
}


BOOTLOADER_SECTION static void prog_eeprom_page_read(uint16_t address)
// Read the EEPROM data at the indicated address into the program buffer.
{
	// Use assembly to read from the EEPROM.
	asm volatile (
		"mov r1,%6\n\t"
		"1:\n\t"
		"sbic %0,%4\n\t"
		"rjmp 1b\n\t"					/* Make sure EEPROM is ready. */
		"out %1,r27\n\t"
		"out %2,r26\n\t"
		"sbi %0,%5\n\t"
		"adiw r26,1\n\t"				/* Increment x register. */
		"in r0,%3\n\t"
		"st z+,r0\n\t"
		"dec r1\n\t"
		"brne 1b\n\t"
		:
		: "I" (_SFR_IO_ADDR(EECR)), 
		  "I" (_SFR_IO_ADDR(EEARH)),
		  "I" (_SFR_IO_ADDR(EEARL)),
		  "I" (_SFR_IO_ADDR(EEDR)),
		  "I" (EEWE),
		  "I" (EERE),
		  "r" ((uint8_t) PROG_PAGE_SIZE),
		  "x" ((uint16_t) address),
		  "z" ((uint16_t) prog_buffer)
		: "r0", "r1", "memory"
	);
}


BOOTLOADER_SECTION void prog_init(void)
// Initialize programming.
{
	// Set the default program buffer address.
	prog_page_address = 0;
	prog_byte_address = 0;

	// Reset programming state flags.
	prog_update_flag = 0;
}


BOOTLOADER_SECTION void prog_buffer_set_address(uint16_t address)
// Set the address to be programmed.
{
	uint8_t i;

	// Split the address into a page address and byte address.
	prog_page_address = address & ~(PROG_PAGE_SIZE - 1);
	prog_byte_address = address & (PROG_PAGE_SIZE - 1);

	// Initialize the programming buffer.
	for (i = 0; i < PROG_PAGE_SIZE; ++i) prog_buffer[i] = 0xFF;

	// Which part of Flash/EEPROM are we reading.
	if (prog_page_address < PROG_FLASH_BOOTLOADER)
	{
#ifndef BOOTSTRAPPER
		// Read the Flash page into the programming buffer.
		prog_flash_page_read(prog_page_address - PROG_FLASH_START);
#else
		// Do nothing.  These pages are protected to prevent
		// overwriting of the bootstrapper application.
#endif
	}
	else if (prog_page_address < PROG_FLASH_END)
	{
#ifdef BOOTSTRAPPER
		// Read the Flash page into the programming buffer.
		prog_flash_page_read(prog_page_address - PROG_FLASH_START);
#else
		// Do nothing.  These pages are protected to prevent
		// overwriting of the bootloader application.
#endif
	}
	else if (prog_page_address < PROG_EEPROM_END)
	{
		// Read the EEPROM page into the programming buffer.
		prog_eeprom_page_read(prog_page_address - PROG_EEPROM_START);
	}

	// Reset the programming buffer update flag.
	prog_update_flag = 0;
}


BOOTLOADER_SECTION uint8_t prog_buffer_get_byte(void)
// Get the byte at the current address.
{
	uint8_t databyte;

	// Get the byte within the programming buffer.
	databyte = prog_buffer[prog_byte_address];

	// Increment the byte address within the page.
	++prog_byte_address;

	// Check the byte address for wrapping.
	if (prog_byte_address > (PROG_PAGE_SIZE - 1)) prog_byte_address = 0;

	return databyte;
}


BOOTLOADER_SECTION void prog_buffer_set_byte(uint8_t databyte)
// Set the byte at the current address.
{
	// Protect the bootloader by protecting the reset vector from being
	// overwritten. The reset vector is the first two byte of the first page.
	if ((prog_page_address != 0) || ((prog_byte_address != 0) && (prog_byte_address != 1)))
	{
		// Set the byte within the programming buffer.
		prog_buffer[prog_byte_address] = databyte;
	}

	// Increment the byte address within the page.
	++prog_byte_address;

	// Check the byte address for wrapping.
	if (prog_byte_address > (PROG_PAGE_SIZE - 1)) prog_byte_address = 0;

	// Set the programming update flag.  This indicates the programming 
	// buffer should be written to Flash.
	prog_update_flag = 1;
}


BOOTLOADER_SECTION void prog_buffer_update(void)
// If the programming buffer was updated it should now be written to Flash.
{
	// Was the programming buffer updated?
	if (prog_update_flag)
	{
		// Which part of Flash/EEPROM are we writing.
		if (prog_page_address < PROG_FLASH_BOOTLOADER)
		{
#ifndef BOOTSTRAPPER
			// Write the programming buffer to Flash.
			prog_flash_page_write(prog_page_address - PROG_FLASH_START);
#else
			// Do nothing.  These pages are protected to prevent
			// overwriting of the bootstrapper application.
#endif
		}
		else if (prog_page_address < PROG_FLASH_END)
		{
#ifdef BOOTSTRAPPER
			// Write the programming buffer to Flash.
			prog_flash_page_write(prog_page_address - PROG_FLASH_START);
#else
			// Do nothing.  These pages are protected to prevent
			// overwriting of the bootloader application.
#endif
		}
		else if (prog_page_address < PROG_EEPROM_END)
		{
			// Read the EEPROM page into the programming buffer.
			prog_eeprom_page_write(prog_page_address - PROG_EEPROM_START);
		}

		// Reset the programming buffer update flag.
		prog_update_flag = 0;
	}
}


