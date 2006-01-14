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
    #define DDRTWI              DDRB
    #define DDSDA               DDB0
    #define DDSCL               DDB2
    #define PORTTWI             PORTB
    #define PORTSDA             PB0
    #define PORTSCL             PB2
    #define PINTWI              PINB
    #define PINSDA              PINB0
    #define PINSCL              PINB2
#endif

// TWI write states.
#define TWI_WRITE_ADDR_HI_BYTE                  (0x00)
#define TWI_WRITE_ADDR_LO_BYTE                  (0x01)
#define TWI_WRITE_DATA_BYTE                     (0x02)

// TWI slave states.
#define TWI_SLAVE_CHECK_ADDRESS                 (0x00)
#define TWI_SLAVE_SEND_DATA                     (0x01)
#define TWI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA  (0x02)
#define TWI_SLAVE_CHECK_REPLY_FROM_SEND_DATA    (0x03)
#define TWI_SLAVE_REQUEST_DATA                  (0x04)
#define TWI_SLAVE_GET_DATA_AND_SEND_ACK         (0x05)

// TWI state values.
static uint8_t twi_write_state;
static uint8_t twi_slave_state;

// TWI flag values.
static uint8_t twi_address_update;

// TWI data values.
static uint16_t twi_address;

BOOTLOADER_SECTION void twi_init(void)
// Initialise USI for TWI slave mode.
{
    // Initialize the TWI state.
    twi_write_state = TWI_WRITE_ADDR_HI_BYTE;
    twi_slave_state = TWI_SLAVE_CHECK_ADDRESS;

    // Initialize the TWI flags.
    twi_address_update = 0;

    // Initialize the TWI data.
    twi_address = 0;

    // Set the SCL and SDA to default to high.
    PORTTWI |= (1<<PORTSCL) | (1<<PORTSDA);

    // Set SCL as output, SDA as input.
    DDRTWI |= (1<<DDSCL);
    DDRTWI &= ~(1<<DDSDA);

    // Set the interrupt enable, wire mode and clock settings.  Note: At this
    // time the wire mode must not be set to hold the SCL line low when the 
    // counter overflows. Otherwise, this TWI slave will interfere with other
    // TWI slaves.
    USICR = (1<<USISIE) | (0<<USIOIE) |                 // Enable Start Condition interrupt. Disable overflow.
            (1<<USIWM1) | (0<<USIWM0) |                 // Maintain USI in two-wire mode without clock stretching.
            (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
            (0<<USITC);                                 // No toggle of clock pin.

    // Clear the interrupt flags and reset the counter.
    USISR = (1<<USISIF) | (1<<USIOIF) | (1<<USIPF) |        // Clear interrupt flags.
            (0x0<<USICNT0);                                 // USI to sample 8 bits or 16 edge toggles.
}


BOOTLOADER_SECTION void twi_deinit(void)
// De-initialise USI.
{
    // Reset SCL and SDA.
    PORTTWI &= ~((1<<PORTSCL) | (1<<PORTSDA));
    DDRTWI &= ~((1<<DDSDA) | (1<<DDSDA));

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
    while ((PINTWI & (1<<PINSCL)) & ~(PINTWI & (1<<PINSDA)));

    // Make sure SDA didn't return to high indicating a stop condition.
    if (~(PINTWI & (1<<PINSDA)))
    {
        // Update the slave state.
        twi_slave_state = TWI_SLAVE_CHECK_ADDRESS;

        // Set SDA as input
        DDRTWI &= ~(1<<PORTSDA);

        // Update the interrupt enable, wire mode and clock settings.
        USICR = (1<<USISIE) | (1<<USIOIE) |                 // Enable Overflow and Start Condition Interrupt.
                (1<<USIWM1) | (1<<USIWM0) |                 // Maintain USI in two-wire mode with clock stretching.
                (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
                (0<<USITC);                                 // No toggle of clock pin.
    }

    // Clear the interrupt flags and reset the counter.
    USISR = (1<<USISIF) | (1<<USIOIF) | (1<<USIPF) |        // Clear interrupt flags.
            (0x00<<USICNT0);                                // USI to sample 8 bits or 16 edge toggles.
}


BOOTLOADER_SECTION void twi_handle_overflow_condition(void)
// Handle the TWI overflow condition.  This is called when the TWI 4-bit counter
// overflows indicating the TWI master has clocked in/out a databyte or a single
// ack/nack byte following a databyte transfer.
{
    // Handling is determined by slave state.
    switch (twi_slave_state)
    {
        // ---------- Address mode ----------
        // Check address and send ACK (and next TWI_SLAVE_SEND_DATA) if OK, else reset USI.
        case TWI_SLAVE_CHECK_ADDRESS:

            // USIDR now contains the TWI address in the upper seven bits
            // and the data direction in the lowest bit.  We check here for
            // an address we are insterested in.
            if ((USIDR>>1) == TWI_SLAVE_ADDRESS)
            {
                // Check the lowest bit.
                if (USIDR & 0x01)
                {
                    // We are to send data to the master.
                    twi_slave_state = TWI_SLAVE_SEND_DATA;
                }
                else
                {
                    // We are to recieve data from the master.
                    twi_slave_state = TWI_SLAVE_REQUEST_DATA;
                }

                // Reset the write state.
                twi_write_state = TWI_WRITE_ADDR_HI_BYTE;

                // Set SDA as output.
                DDRTWI |= (1<<PORTSDA);

                // Have the USI send an ACK.
                USIDR = 0;

                // Update the interrupt enable, wire mode and clock settings.
                USICR = (1<<USISIE) | (1<<USIOIE) |                 // Enable Start Condition and overflow interrupt.
                        (1<<USIWM1) | (1<<USIWM0) |                 // Maintain USI in two-wire mode with clock stretching.
                        (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
                        (0<<USITC);                                 // No toggle of clock pin.

                // Clear the interrupt flags and reset the counter for one bit.
                USISR = (1<<USISIF) | (1<<USIOIF) | (0<<USIPF) |    // Clear start and overflow conditions flags.
                        (0x0E<<USICNT0);                            // USI to sample 1 bit or two edge toggles.
            }
            else
            {
                // Reset USI to detect start condition.  Update the interrupt enable, 
                // wire mode and clock settings.  Note: At this time the wire mode must
                // not be set to hold the SCL line low when the counter overflows.  
                // Otherwise, this TWI slave will interfere with other TWI slaves.
                USICR = (1<<USISIE) | (0<<USIOIE) |                 // Enable Start Condition Interrupt. Disable overflow.
                        (1<<USIWM1) | (0<<USIWM0) |                 // Maintain USI in two-wire mode without clock stretching.
                        (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
                        (0<<USITC);                                 // No toggle of clock pin.

                // Clear the interrupt flags and reset the counter.
                USISR = (0<<USISIF) | (1<<USIOIF) | (1<<USIPF) |    // Clear overflow and stop condition flags.
                        (0x00<<USICNT0);                            // USI to sample 8 bits or 16 edge toggles.

            }
            break;

        // ----- Master read data mode ------

        // Check reply and goto TWI_SLAVE_SEND_DATA if OK, else reset USI.
        case TWI_SLAVE_CHECK_REPLY_FROM_SEND_DATA:

            // Check the lowest bit for NACK?  If set, the master does not want more data.
            if (USIDR & 0x01)
            {
                // Reset USI to detect start condition.  Update the interrupt enable,
                // wire mode and clock settings. Note: At this time the wire mode must
                // not be set to hold the SCL line low when the counter overflows.  
                // Otherwise, this TWI slave will interfere with other TWI slaves.
                USICR = (1<<USISIE) | (0<<USIOIE) |                 // Enable Start Condition Interrupt. Disable overflow.
                        (1<<USIWM1) | (0<<USIWM0) |                 // Maintain USI in two-wire mode without clock stretching.
                        (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
                        (0<<USITC);                                 // No toggle of clock pin.

                // Clear the interrupt flags and reset the counter.
                USISR = (0<<USISIF) | (1<<USIOIF) | (0<<USIPF) |    // Clear overflow condition flag.
                        (0x00<<USICNT0);                            // USI to sample 8 bits or 16 edge toggles.

                return;
            }

            // From here we just drop straight into TWI_SLAVE_SEND_DATA if the master sent an ACK

        // Copy data from buffer to USIDR and set USI to shift byte. Next TWI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA
        case TWI_SLAVE_SEND_DATA:

            // Update our slave state.
            twi_slave_state = TWI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA;

            // Set SDA as output.
            DDRTWI |= (1<<PORTSDA);

            // Get the data to send.
            USIDR = prog_buffer_get_byte();

            // Update the interrupt enable, wire mode and clock settings.
            USICR = (1<<USISIE) | (1<<USIOIE) |                 // Enable Start Condition and overflow interrupt.
                    (1<<USIWM1) | (1<<USIWM0) |                 // Maintain USI in two-wire mode with clock stretching.
                    (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
                    (0<<USITC);                                 // No toggle of clock pin.

            // Clear the interrupt flags and reset the counter.
            USISR = (0<<USISIF) | (1<<USIOIF) | (0<<USIPF) |    // Clear overflow condition flag.
                    (0x00<<USICNT0);                            // USI to sample 8 bits or 16 edge toggles.

            break;

        // ----- Master write data mode ------
        // Set USI to sample reply from master. Next TWI_SLAVE_CHECK_REPLY_FROM_SEND_DATA
        case TWI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA:

            // Update our slave state.
            twi_slave_state = TWI_SLAVE_CHECK_REPLY_FROM_SEND_DATA;

            // Set SDA as input
            DDRTWI &= ~(1<<PORTSDA);

            // Update the interrupt enable, wire mode and clock settings.
            USICR = (1<<USISIE) | (1<<USIOIE) |                 // Enable Start Condition and overflow interrupt.
                    (1<<USIWM1) | (1<<USIWM0) |                 // Maintain USI in two-wire mode with clock stretching.
                    (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
                    (0<<USITC);                                 // No toggle of clock pin.

            // Clear the interrupt flags and reset the counter for one bit.
            USISR = (0<<USISIF) | (1<<USIOIF) | (0<<USIPF) |    // Clear overflow condition flag.
                    (0x0E<<USICNT0);                            // USI to sample 1 bit or two edge toggles.

            break;

        // Set USI to sample data from master. Next TWI_SLAVE_GET_DATA_AND_SEND_ACK.
        case TWI_SLAVE_REQUEST_DATA:

            // Update our slave state.
            twi_slave_state = TWI_SLAVE_GET_DATA_AND_SEND_ACK;

            // Set SDA as input
            DDRTWI &= ~(1<<PORTSDA);

            // Update the interrupt enable, wire mode and clock settings.
            USICR = (1<<USISIE) | (1<<USIOIE) |                 // Enable Start Condition and overflow interrupt.
                    (1<<USIWM1) | (1<<USIWM0) |                 // Maintain USI in two-wire mode with clock stretching.
                    (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
                    (0<<USITC);                                 // No toggle of clock pin.

            // Clear the interrupt flags and reset the counter.
            USISR = (0<<USISIF) | (1<<USIOIF) | (0<<USIPF) |    // Clear overflow condition flag.
                    (0x00<<USICNT0);                            // USI to sample 8 bits or 16 edge toggles.

            break;

        // Copy data from USIDR and send ACK. Next TWI_SLAVE_REQUEST_DATA
        case TWI_SLAVE_GET_DATA_AND_SEND_ACK:

            // Update our slave state.
            twi_slave_state = TWI_SLAVE_REQUEST_DATA;

            // Check the TWI write state to determine what type of byte we received.
            if (twi_write_state == TWI_WRITE_ADDR_HI_BYTE)
            {
                // Set the twi address high byte.
                twi_address = USIDR;

                // Set the next state.
                twi_write_state = TWI_WRITE_ADDR_LO_BYTE;
            }
            else if (twi_write_state == TWI_WRITE_ADDR_LO_BYTE)
            {
                // Set the address low byte.
                twi_address = (twi_address << 8) | USIDR;

                // Set the programming address.
                twi_address_update = 1;

                // Set the next state.
                twi_write_state = TWI_WRITE_DATA_BYTE;
            }
            else
            {
                // Write the data to the buffer.
                prog_buffer_set_byte(USIDR);
            }

            // Set SDA as output.
            DDRTWI |= (1<<PORTSDA);

            // Have the USI send an ACK.
            USIDR = 0;

            // Update the interrupt enable, wire mode and clock settings.
            USICR = (1<<USISIE) | (1<<USIOIE) |                 // Enable Start Condition and overflow interrupt.
                    (1<<USIWM1) | (1<<USIWM0) |                 // Maintain USI in two-wire mode with clock stretching.
                    (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
                    (0<<USITC);                                 // No toggle of clock pin.

            // Clear the interrupt flags and reset the counter for one bit.
            USISR = (1<<USISIF) | (1<<USIOIF) | (0<<USIPF) |    // Clear start and overflow condition flag.
                    (0x0E<<USICNT0);                            // USI to sample 1 bit or two edge toggles.

            break;
    }
}


