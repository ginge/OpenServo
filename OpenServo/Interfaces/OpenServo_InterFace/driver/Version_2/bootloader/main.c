// ======================================================================
// USBtiny bootloader
//
// Copyright (C) 2007 Barry Carter
//
//   A very minimal bootloader to update the OSIF over USB using the
//   software USB library "usbtiny".
//   There are four main functions, Initialise Application, 
//   Read page, Write page and Get the page size from the device.
//
//
//
// usbtiny Copyright (C) 2006 Dick Streefland
//
// This is free software, licensed under the terms of the GNU General
// Public License V2 as published by the Free Software Foundation.
// ======================================================================

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/boot.h>
#include <avr/delay.h>
#include "usb.h"

#define USBTINYBL_FUNC_APP_INIT 1
#define USBTINYBL_FUNC_READ     2
#define USBTINYBL_FUNC_WRITE    3
#define USBTINYBL_FUNC_GET_PAGE 4
#define USBTINYBL_FUNC_GET_VER  5

#define BOOTLOADER_ADDRESS 0x1800

// Firmware version
#define VERSION_MAJOR  0
#define VERSION_MINOR  9

#define STATUS_IDLE  0
#define STATUS_WRITE 1
#define STATUS_READ  2

// Set the initial mode as IDLE
static byte_t status = STATUS_IDLE;
static unsigned int page_address;
static unsigned int page_counter;

// A pointer to the main application
void (*init_app)(void) = 0x0000;

// Start the application in firmware
void init_application() 
{
    cli();
    boot_rww_enable();   // Enable access to the flash memory
    GICR = (1 << IVCE);  // enable change of interrupt vectors
    GICR = (0 << IVSEL); // move interrupts to application flash section 
    init_app();
}

// Handle setup request
extern	byte_t	usb_setup ( byte_t data[8] )
{
    byte_t len = 0;

    // Load the application
    if (data[1] == USBTINYBL_FUNC_APP_INIT) 
    {
        init_application();
    }
    // Write a page
    else if (data[1] == USBTINYBL_FUNC_WRITE) 
    {

        status = STATUS_WRITE;

        page_address = (data[3] << 8) | data[2]; // Set the page address
        // ignore a page write over the bootloader section
        if (page_address >= BOOTLOADER_ADDRESS)
        {
            status = STATUS_IDLE;
            return 0xff;
        }
        page_counter = 0;

        eeprom_busy_wait();
        cli();
        boot_page_erase(page_address);          // Erase the page
        sei();
        boot_spm_busy_wait();                   // Wait until page is erased

        len = 0xff;                             // Request the data though usb_out
    }
    // Read a page
    else if (data[1] == USBTINYBL_FUNC_READ) 
    {

        status = STATUS_READ;

        page_address = (data[3] << 8) | data[2]; // Set the page address
        page_counter = 0;
        boot_rww_enable();
        len = 0xff;                             // Request the data though usb_out
    }
    else if (data[1] == USBTINYBL_FUNC_GET_VER)
    {
        data[0] = VERSION_MAJOR;
        data[1] = VERSION_MINOR;
        len = 2;
    }
    // Load page size
    else if (data[1] == USBTINYBL_FUNC_GET_PAGE) 
    {

      data[0] = SPM_PAGESIZE >> 8;
      data[1] = SPM_PAGESIZE & 0xff;
      len = 2;
    }

    return len;
}

// Handle USB out
extern	void	usb_out ( byte_t* data, byte_t len )
{

    byte_t i;

    // Make sure we are in write mode
    if (status != STATUS_WRITE)
        return;

    for (i = 0; i < len; i+=2) 
    {
        cli();
        boot_page_fill(page_address + page_counter, data[i] | (data[i + 1] << 8)); //Write the page to flash
        sei();
        page_counter += 2;     //Increment the page counter

        // Check to see if this the end of the page
        if (page_counter >= SPM_PAGESIZE) 
        {
            // Flash the page
            cli();
            boot_page_write(page_address);
            sei();
            boot_spm_busy_wait();
            // Set the status to IDLE
            status = STATUS_IDLE;
            return;
        }
    }
    return;
}

extern byte_t usb_in ( byte_t* data, byte_t len )
{
    byte_t i;

    // Make sure we are in write mode
    if (status != STATUS_READ)
        return 0;

    for(i=0; i < len; i++)
    {
        // Don't return the bootloader section on a read return 0xff (blank)
        if (page_address >= BOOTLOADER_ADDRESS)
        {
            data[i] = 0xFF;
            continue;
        }
        data[i] = pgm_read_byte( page_address + page_counter);

        page_counter++;
    }

    if (page_counter >= SPM_PAGESIZE)
        status = STATUS_IDLE;

    return i;
}

int main(void)
{
    // Initialise hardware
    //Set all ports as inputs apart from the USB
    DDRB  = 0;
    PORTB = 0;
    PORTB |= _BV(PB4); // Enable the PB4 pullup.
    DDRB  = _BV(PB3);  // This is where our jumper pin detection will happen. Make output
    PORTC = 0;         
    DDRC  = 0;
    PORTD = 0;
    DDRD  = 0x02;


    // Load the main application if jumper is not set
    // Change this pin to relocate the jumper elsewhere
    // Take PB3 low
    PORTB &= ~(1 << PB3);

    // If PB3 and PB4 are connected, then pulling PB3 low will pull PB4 low
    if (PINB & (1 << PB4))
    {
       init_application();
    }

    /*
    // We need to pull USB + - low for at least 200ms to force
    // a renumeration of init
    DDRC = (_BV(PC0) | _BV(PC1));
    PORTC &= ~(_BV(PC0) | _BV(PC1));

    _delay_ms(400);
*/
    GICR = (1 << IVCE);  // Enable change of interrupt vectors
    GICR = (1 << IVSEL); // Move interrupts to boot flash section

    usb_init();
    sei();
    for(;;){
        usb_poll();

    }
    return 0;
}

