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

#undef __AVR_ATtiny2313__
#define __AVR_ATtiny45__

#include <inttypes.h>
#include <avr/io.h>

#include "bootloader.h"
#include "prog.h"
#include "twi.h"

// Device dependant defines
#if defined(__AVR_ATtiny25__) | defined(__AVR_ATtiny45__) | defined(__AVR_ATtiny85__)
    #define DDR_USI             DDRB
    #define DD_SDA              DDB0
    #define DD_SCL              DDB2
    #define PORT_USI            PORTB
    #define P_SDA               PB0
    #define P_SCL               PB2
    #define PIN_USI             PINB
    #define PIN_SDA             PINB0
    #define PIN_SCL             PINB2
#endif

// TWI write states.
#define TWI_WRITE_ADDR_HI_BYTE              (0x00)
#define TWI_WRITE_ADDR_LO_BYTE              (0x01)
#define TWI_WRITE_DATA_BYTE                 (0x02)

// TWI overflow states.
#define TWI_OVERFLOW_STATE_NONE             (0x00)
#define TWI_OVERFLOW_STATE_ACK_PR_RX        (0x01)
#define TWI_OVERFLOW_STATE_DATA_RX          (0x02)
#define TWI_OVERFLOW_STATE_ACK_PR_TX        (0x03)
#define TWI_OVERFLOW_STATE_PR_ACK_TX        (0x04)
#define TWI_OVERFLOW_STATE_DATA_TX          (0x05)

// TWI state values.
static uint8_t twi_write_state;
static uint8_t twi_overflow_state;

// TWI flag values.
static uint8_t twi_address_update;

// TWI data values.
static uint16_t twi_address;

BOOTLOADER_SECTION void twi_init(void)
// Initialise USI for TWI slave mode.
{
    // Initialize the TWI states.
    twi_write_state = TWI_WRITE_ADDR_HI_BYTE;
    twi_overflow_state = TWI_OVERFLOW_STATE_NONE;

    // Initialize the TWI flags.
    twi_address_update = 0;

    // Initialize the TWI data.
    twi_address = 0;

    // Set the interrupt enable, wire mode and clock settings.  Note: At this
    // time the wire mode must not be set to hold the SCL line low when the 
    // counter overflows. Otherwise, this TWI slave will interfere with other
    // TWI slaves.
    USICR = (0<<USISIE) | (0<<USIOIE) |                 // Disable start condition and overflow interrupt.
            (1<<USIWM1) | (0<<USIWM0) |                 // Set USI to two-wire mode without clock stretching.
            (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
            (0<<USITC);                                 // No toggle of clock pin.

    // Clear the interrupt flags and reset the counter.
    USISR = (1<<USISIF) | (1<<USIOIF) | (1<<USIPF) |        // Clear interrupt flags.
            (0x0<<USICNT0);                                 // USI to sample 8 bits or 16 edge toggles.

    // Configure SDA.
    DDR_USI &= ~(1<<DD_SDA);
    PORT_USI &= ~(1<<P_SDA);

    // Configure SCL.
    DDR_USI |= (1<<DD_SCL);
    PORT_USI |= (1<<P_SCL);

    // Start condition interrupt enable.
    USICR |= (1<<USISIE);
}


BOOTLOADER_SECTION void twi_deinit(void)
// De-initialise USI.
{
    // Reset SCL and SDA.
    PORT_USI &= ~((1<<P_SCL) | (1<<P_SDA));
    DDR_USI &= ~((1<<DD_SCL) | (1<<DD_SDA));

    // Clear the USI registers.
    USIDR = 0x00;
    USICR = 0x00;
    USISR = 0xF0;
}


BOOTLOADER_SECTION void twi_check_conditions(void)
// Checks for TWI start condition.
{
    // Check for TWI start condition.
    if (USISR & (1<<USISIF))
    {
        // Handle the TWI start condition.
        twi_handle_start_condition();
    }

    // Check for TWI overflow condition.
    if (USISR & (1<<USIOIF))
    {
        // Handle the TWI overflow condition.
        twi_handle_overflow_condition();

        // Should we update the twi address?
        if (twi_address_update)
        {
            // Mark the bootloader as active.
            bootloader_active = 1;

            // Check for the special address to exit the bootloader.
            if (twi_address != 0xffff)
            {
                // Set the twi address.  This will load the corresponding page from
                // flash into the programming buffer for reading and writing.
                prog_buffer_set_address(twi_address);
            }

            // Reset the flag.
            twi_address_update = 0;
        }
    }

    // Check for TWI stop condition.
    if (USISR & (1<<USIPF))
    {
        // Clear the stop condition flag.
        USISR = (1<<USIPF);

        // Check for the special address to exit the bootloader.
        if (twi_address == 0xffff)
        {
            // Set the flag to have the bootloader eixt.
            bootloader_exit = 1;
        }
        else
        {
            // Update the programming buffer if needed.
            prog_buffer_update();
        }
    }
}


BOOTLOADER_SECTION void twi_handle_start_condition(void)
// Handle the TWI start condition.  This is called when the TWI master initiates
// communication with a TWI slave by asserting the TWI start condition.
{
    // Wait until the "Start Condition" is complete when SCL goes low. If we fail to wait
    // for SCL to go low we may miscount the number of clocks pulses for the data because
    // the transition of SCL could be mistaken as one of the data clock pulses.
    while ((PIN_USI & (1<<PIN_SCL)));

    // Reset the overflow state.
    twi_overflow_state = TWI_OVERFLOW_STATE_NONE;

    // Clear the interrupt flags and reset the counter.
    USISR = (1<<USISIF) | (1<<USIOIF) | (1<<USIPF) |    // Clear interrupt flags.
            (0x00<<USICNT0);                            // USI to sample 8 bits or 16 edge toggles.

    // Update the interrupt enable, wire mode and clock settings.
    USICR = (1<<USISIE) | (1<<USIOIE) |                 // Enable Overflow and Start Condition Interrupt.
            (1<<USIWM1) | (1<<USIWM0) |                 // Maintain USI in two-wire mode with clock stretching.
            (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
            (0<<USITC);                                 // No toggle of clock pin.
}


BOOTLOADER_SECTION void twi_handle_overflow_condition(void)
// Handle the TWI overflow condition.  This is called when the TWI 4-bit counter
// overflows indicating the TWI master has clocked in/out a databyte or a single
// ack/nack byte following a databyte transfer.
{
    // Buffer the USI data.
    uint8_t usi_data = USIDR;

    // Handle the interrupt based on the overflow state.
    switch (twi_overflow_state)
    {
        // Handle the first byte transmitted from master -- the slave address.
        case TWI_OVERFLOW_STATE_NONE:

            // Are we receiving our address?
            if ((usi_data >> 1) == TWI_SLAVE_ADDRESS)
            {
                // Yes. Are we to send or receive data?
                twi_overflow_state = (usi_data & 0x01) ? TWI_OVERFLOW_STATE_ACK_PR_TX : TWI_OVERFLOW_STATE_ACK_PR_RX;

                // Reset the write state.
                twi_write_state = TWI_WRITE_ADDR_HI_BYTE;

                // Set SDA for output.
                PORT_USI |= (1<<P_SDA);
                DDR_USI |= (1<<DD_SDA);

                // Load data for ACK.
                USIDR = 0;

                // Reload counter for ACK -- two clock transitions.
                USISR = 0x0E;
            }
            else
            {
                // No. Reset USI to detect start condition.  Update the interrupt enable, 
                // wire mode and clock settings.  Note: At this time the wire mode must
                // not be set to hold the SCL line low when the counter overflows.  
                // Otherwise, this TWI slave will interfere with other TWI slaves.
                USICR = (1<<USISIE) | (0<<USIOIE) |                 // Enable Start Condition Interrupt. Disable overflow.
                        (1<<USIWM1) | (0<<USIWM0) |                 // Maintain USI in two-wire mode without clock stretching.
                        (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
                        (0<<USITC);                                 // No toggle of clock pin.
            }

            break;

        // Ack sent to master so prepare to receive more data.
        case TWI_OVERFLOW_STATE_ACK_PR_RX:

            // Update our state.
            twi_overflow_state = TWI_OVERFLOW_STATE_DATA_RX;

            // Set SDA for input
            DDR_USI &= ~(1<<DD_SDA);
            PORT_USI &= ~(1<<P_SDA);

            break;

        // Data received from master so prepare to send ACK.
        case TWI_OVERFLOW_STATE_DATA_RX:

            // Update our state.
            twi_overflow_state = TWI_OVERFLOW_STATE_ACK_PR_RX;

            // Check the TWI write state to determine what type of byte we received.
            if (twi_write_state == TWI_WRITE_ADDR_HI_BYTE)
            {
                // Set the twi address high byte.
                twi_address = usi_data;

                // Set the next state.
                twi_write_state = TWI_WRITE_ADDR_LO_BYTE;
            }
            else if (twi_write_state == TWI_WRITE_ADDR_LO_BYTE)
            {
                // Set the address low byte.
                twi_address = (twi_address << 8) | usi_data;

                // Set the programming address.
                twi_address_update = 1;

                // Set the next state.
                twi_write_state = TWI_WRITE_DATA_BYTE;
            }
            else
            {
                // Write the data to the buffer.
                prog_buffer_set_byte(usi_data);
            }

            // Set SDA for output.
            PORT_USI |= (1<<P_SDA);
            DDR_USI |= (1<<DD_SDA);

            // Load data for ACK.
            USIDR = 0;

            // Reload counter for ACK -- two clock transitions.
            USISR = 0x0E;

            break;

        // ACK received from master.  Reset USI state if NACK received.
        case TWI_OVERFLOW_STATE_PR_ACK_TX:

            // Check the lowest bit for NACK?  If set, the master does not want more data.
            if (usi_data & 0x01)
            {
                // Update our state.
                twi_overflow_state = TWI_OVERFLOW_STATE_NONE;

                // Reset USI to detect start condition. Update the interrupt enable,
                // wire mode and clock settings. Note: At this time the wire mode must
                // not be set to hold the SCL line low when the counter overflows.  
                // Otherwise, this TWI slave will interfere with other TWI slaves.
                USICR = (1<<USISIE) | (0<<USIOIE) |                 // Enable Start Condition Interrupt. Disable overflow.
                        (1<<USIWM1) | (0<<USIWM0) |                 // Maintain USI in two-wire mode without clock stretching.
                        (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
                        (0<<USITC);                                 // No toggle of clock pin.


                // Clear the overflow interrupt flag and release the hold on SCL.
                USISR |= (1<<USIOIF);

                return;
            }

        // Handle sending a byte of data.
        case TWI_OVERFLOW_STATE_ACK_PR_TX:

            // Update our state.
            twi_overflow_state = TWI_OVERFLOW_STATE_DATA_TX;

            // Set SDA for output.
            PORT_USI |= (1<<P_SDA);
            DDR_USI |= (1<<DD_SDA);

            // Get the data to send.
            USIDR = prog_buffer_get_byte();

            break;

        // Data sent to to master so prepare to receive ack.
        case TWI_OVERFLOW_STATE_DATA_TX:

            // Update our state.
            twi_overflow_state = TWI_OVERFLOW_STATE_PR_ACK_TX;

            // Set SDA for input.
            DDR_USI &= ~(1<<DD_SDA);
            PORT_USI &= ~(1<<P_SDA);

            // Reload counter for ACK -- two clock transitions.
            USISR = 0x0E;

            break;

    }

    // Clear the overflow interrupt flag and release the hold on SCL.
    USISR |= (1<<USIOIF);
}

