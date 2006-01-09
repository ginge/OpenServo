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

// The following is needed until WINAVR supports the ATtinyX5 MCUs.
#undef __AVR_ATtiny2313__
#define __AVR_ATtiny45__

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>

#include "twi.h"
#include "registers.h"

//////////////////////////////////////////////////////////////////
///////////////// Driver Buffer Definitions //////////////////////
//////////////////////////////////////////////////////////////////
// 1,2,4,8,16,32,64,128 or 256 bytes are allowed buffer sizes

#define TWI_RX_BUFFER_SIZE (16)
#define TWI_RX_BUFFER_MASK (TWI_RX_BUFFER_SIZE - 1)

#if (TWI_RX_BUFFER_SIZE & TWI_RX_BUFFER_MASK)
        #error TWI RX buffer size is not a power of 2
#endif

//////////////////////////////////////////////////////////////////

#define TWI_SLAVE_CHECK_ADDRESS                (0x00)
#define TWI_SLAVE_SEND_DATA                    (0x01)
#define TWI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA (0x02)
#define TWI_SLAVE_CHECK_REPLY_FROM_SEND_DATA   (0x03)
#define TWI_SLAVE_REQUEST_DATA                 (0x04)
#define TWI_SLAVE_GET_DATA_AND_SEND_ACK        (0x05)

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


// Read registers.  This function is meant to safely read
// the registers for the TWI master.
inline static uint8_t twi_registers_read(uint8_t address)
{
    // Prevent overflow by wrapping.
    address &= 0x7F;

    // Avoid reading unimplemented registers.
    if (address > 0x3F) return 0;

    // Read the register.
    return registers_read_byte(address);
}


// Write non-write protected registers.  This function
// is meant to safely write registers for the TWI master.
inline static void twi_registers_write(uint8_t address, uint8_t data)
{
    // Prevent overflow by wrapping.
    address &= 0x7F;

    // Avoid writing unimplemented registers.
    if (address > 0x3F) return;

    // Don't write to read/only registers.
    if (address <= MAX_RO_REGISTER) return;

    // Don't write to safe read/write registers unless enabled.
    if (!registers_read_byte(WRITE_ENABLE) && (address > MAX_RW_REGISTER)) return;

    // Write the data to the address address.
    registers_write_byte(address, data);
}


// Locals.
static uint8_t twi_slave_address;
static volatile uint8_t twi_overflow_state;
static volatile uint8_t twi_command_mode;
static volatile uint8_t twi_address;

static uint8_t twi_rxbuf[TWI_RX_BUFFER_SIZE];
static volatile uint8_t twi_rxhead;
static volatile uint8_t twi_rxtail;

void
twi_slave_init(uint8_t slave_address)
// Initialise USI for TWI slave mode.
{
    // Flush the buffers.
    twi_rxtail = 0;
    twi_rxhead = 0;

    // Set the slave address.
    twi_slave_address = slave_address;

    // Set the SCL AND SDA to default to high.
    PORTTWI |= (1<<PORTSCL) | (1<<PORTSDA);

    // Set SCL as output, SDA as input.
    DDRTWI |= (1<<DDSCL);
    DDRTWI &= ~(1<<DDSDA);

    // Set the interrupt enable, wire mode and clock settings.
    USICR = (1<<USISIE) | (0<<USIOIE) |                 // Enable Start Condition interrupt. Disable overflow.
            (1<<USIWM1) | (1<<USIWM0) |                 // Maintain USI in Two-wire mode.
            (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
            (0<<USITC);                                 // No toggle of clock pin.

    // Clear the interrupt flags and reset the counter.
    USISR = (1<<USISIF) | (1<<USIOIF) | (1<<USIPF) |        // Clear interrupt flags.
            (0x0<<USICNT0);                                 // USI to sample 8 bits or 16 edge toggles.
}


uint8_t twi_receive_byte(void)
// Returns a byte from the receive buffer. Waits if buffer is empty.
{
    // Wait for data in the buffer.
    while (twi_rxhead == twi_rxtail);

    // Calculate buffer index.
    twi_rxtail = (twi_rxtail + 1 ) & TWI_RX_BUFFER_MASK;

    // Return data from the buffer.
    return twi_rxbuf[twi_rxtail];
}


uint8_t twi_data_in_receive_buffer(void)
// Check if there is data in the receive buffer.
{
    // Return 0 (FALSE) if the receive buffer is empty.
    return (twi_rxhead != twi_rxtail);
}


SIGNAL(SIG_USI_START)
// Detects the USI_TWI Start Condition and intialises the USI
// for reception of the "TWI Address" packet.
{
    // Wait until the "Start Condition" is complete when SCL goes low. If we fail to wait
    // for SCL to go low we may miscount the number of clocks pulses for the data because
    // the transition of SCL could be mistaken as one of the data clock pulses.
    while ((PINTWI & (1<<PINSCL)) & ~(PINTWI & (1<<PINSDA)));

    // Make sure SDA didn't return to high indicating a stop condition.
    if (~(PINTWI & (1<<PINSDA)))
    {
        // Update our state.
        twi_overflow_state = TWI_SLAVE_CHECK_ADDRESS;

        // Set SDA as input
        DDRTWI &= ~(1<<PORTSDA);

        // Update the interrupt enable, wire mode and clock settings.
        USICR = (1<<USISIE) | (1<<USIOIE) |                 // Enable Overflow and Start Condition Interrupt.
                (1<<USIWM1) | (1<<USIWM0) |                 // Maintain USI in Two-wire mode.
                (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
                (0<<USITC);                                 // No toggle of clock pin.
    }

    // Clear the interrupt flags and reset the counter.
    USISR = (1<<USISIF) | (1<<USIOIF) | (1<<USIPF) |        // Clear interrupt flags.
            (0x00<<USICNT0);                                // USI to sample 8 bits or 16 edge toggles.
}


SIGNAL(SIG_USI_OVERFLOW)
// Handles all the comunication. Disabled only when waiting for new Start Condition.
{
    switch (twi_overflow_state)
    {
        // ---------- Address mode ----------
        // Check address and send ACK (and next TWI_SLAVE_SEND_DATA) if OK, else reset USI.
        case TWI_SLAVE_CHECK_ADDRESS:

            // USIDR now contains the TWI address in the upper seven bits
            // and the data direction in the lowest bit.  We check here for
            // an address we are insterested in.
            if ((USIDR>>1) == (twi_slave_address & 0x7f))
            {
                // Check the lowest bit.
                if (USIDR & 0x01)
                {
                    // We are to send data to the master.
                    twi_overflow_state = TWI_SLAVE_SEND_DATA;
                }
                else
                {
                    // We are to recieve data from the master.
                    twi_overflow_state = TWI_SLAVE_REQUEST_DATA;
                }

                // Reset the command mode flag to assume command mode.
                twi_command_mode = 1;

                // Set SDA as output.
                DDRTWI |= (1<<PORTSDA);

                // Have the USI send an ACK.
                USIDR = 0;

                // Update the interrupt enable, wire mode and clock settings.
                USICR = (1<<USISIE) | (1<<USIOIE) |                 // Enable Start Condition and overflow interrupt.
                        (1<<USIWM1) | (1<<USIWM0) |                 // Maintain USI in Two-wire mode.
                        (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
                        (0<<USITC);                                 // No toggle of clock pin.

                // Clear the interrupt flags and reset the counter for one bit.
                USISR = (1<<USISIF) | (1<<USIOIF) | (1<<USIPF) |        // Clear interrupt flags.
                        (0x0E<<USICNT0);                                // USI to sample 1 bit or two edge toggles.
            }
            else
            {
                // Reset USI to detect start condition.
                // Update the interrupt enable, wire mode and clock settings.
                USICR = (1<<USISIE) | (0<<USIOIE) |                 // Enable Start Condition Interrupt. Disable overflow.
                        (1<<USIWM1) | (1<<USIWM0) |                 // Maintain USI in Two-wire mode.
                        (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
                        (0<<USITC);                                 // No toggle of clock pin.

                // Clear the interrupt flags and reset the counter.
                USISR = (0<<USISIF) | (1<<USIOIF) | (1<<USIPF) |    // Clear interrupt flags except start condition.
                        (0x00<<USICNT0);                            // USI to sample 8 bits or 16 edge toggles.

            }
            break;

        // ----- Master read data mode ------

        // Check reply and goto TWI_SLAVE_SEND_DATA if OK, else reset USI.
        case TWI_SLAVE_CHECK_REPLY_FROM_SEND_DATA:

            // Check the lowest bit for NACK?  If set, the master does not want more data.
            if (USIDR & 0x01)
            {
                // Reset USI to detect start condition.
                // Update the interrupt enable, wire mode and clock settings.
                USICR = (1<<USISIE) | (0<<USIOIE) |                 // Enable Start Condition Interrupt. Disable overflow.
                        (1<<USIWM1) | (1<<USIWM0) |                 // Maintain USI in Two-wire mode.
                        (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
                        (0<<USITC);                                 // No toggle of clock pin.

                // Clear the interrupt flags and reset the counter.
                USISR = (0<<USISIF) | (1<<USIOIF) | (1<<USIPF) |    // Clear interrupt flags except start condition.
                        (0x00<<USICNT0);                            // USI to sample 8 bits or 16 edge toggles.

                return;
            }

            // From here we just drop straight into TWI_SLAVE_SEND_DATA if the master sent an ACK

        // Copy data from buffer to USIDR and set USI to shift byte. Next TWI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA
        case TWI_SLAVE_SEND_DATA:

            // Update our state.
            twi_overflow_state = TWI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA;

            // Set SDA as output.
            DDRTWI |= (1<<PORTSDA);

            // Read the data from the addressed register.
            USIDR = twi_registers_read(twi_address);

            // Increment to the next address.
            ++twi_address;

            // Update the interrupt enable, wire mode and clock settings.
            USICR = (1<<USISIE) | (1<<USIOIE) |                 // Enable Start Condition and overflow interrupt.
                    (1<<USIWM1) | (1<<USIWM0) |                 // Maintain USI in Two-wire mode.
                    (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
                    (0<<USITC);                                 // No toggle of clock pin.

            // Clear the interrupt flags and reset the counter.
            USISR = (0<<USISIF) | (1<<USIOIF) | (1<<USIPF) |    // Clear interrupt flags except start condition.
                    (0x00<<USICNT0);                            // USI to sample 8 bits or 16 edge toggles.

            break;

        // ----- Master write data mode ------
        // Set USI to sample reply from master. Next TWI_SLAVE_CHECK_REPLY_FROM_SEND_DATA
        case TWI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA:

            // Update our state.
            twi_overflow_state = TWI_SLAVE_CHECK_REPLY_FROM_SEND_DATA;

            // Set SDA as input
            DDRTWI &= ~(1<<PORTSDA);

            // Update the interrupt enable, wire mode and clock settings.
            USICR = (1<<USISIE) | (1<<USIOIE) |                 // Enable Start Condition and overflow interrupt.
                    (1<<USIWM1) | (1<<USIWM0) |                 // Maintain USI in Two-wire mode.
                    (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
                    (0<<USITC);                                 // No toggle of clock pin.

            // Clear the interrupt flags and reset the counter for one bit.
            USISR = (0<<USISIF) | (1<<USIOIF) | (1<<USIPF) |    // Clear interrupt flags except start condition.
                    (0x0E<<USICNT0);                            // USI to sample 1 bit or two edge toggles.

            break;

        // Set USI to sample data from master. Next TWI_SLAVE_GET_DATA_AND_SEND_ACK.
        case TWI_SLAVE_REQUEST_DATA:

            // Update our state.
            twi_overflow_state = TWI_SLAVE_GET_DATA_AND_SEND_ACK;

            // Set SDA as input
            DDRTWI &= ~(1<<PORTSDA);

            // Update the interrupt enable, wire mode and clock settings.
            USICR = (1<<USISIE) | (1<<USIOIE) |                 // Enable Start Condition and overflow interrupt.
                    (1<<USIWM1) | (1<<USIWM0) |                 // Maintain USI in Two-wire mode.
                    (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
                    (0<<USITC);                                 // No toggle of clock pin.

            // Clear the interrupt flags and reset the counter.
            USISR = (0<<USISIF) | (1<<USIOIF) | (1<<USIPF) |    // Clear interrupt flags except start condition.
                    (0x00<<USICNT0);                            // USI to sample 8 bits or 16 edge toggles.

            break;

        // Copy data from USIDR and send ACK. Next TWI_SLAVE_REQUEST_DATA
        case TWI_SLAVE_GET_DATA_AND_SEND_ACK:

            // Update our state.
            twi_overflow_state = TWI_SLAVE_REQUEST_DATA;

            // Is this an address or command?
            if (twi_command_mode)
            {
                // Are we switching to addressed mode?
                if (USIDR < TWI_CMD_RESET)
                {
                    // Yes. Capture the register address.
                    twi_address = USIDR;

                    // Switch out of command mode.
                    twi_command_mode = 0;
                }
                else
                {
                    // Put the command into the receive buffer to be handled asynchronously.
                    twi_rxhead = (twi_rxhead + 1) & TWI_RX_BUFFER_MASK;
                    twi_rxbuf[twi_rxhead] = USIDR;
                }
            }
            else
            {
                // Write the data to the addressed register.
                twi_registers_write(twi_address, USIDR);

                // Increment to the next address.
                ++twi_address;
            }

            // Set SDA as output.
            DDRTWI |= (1<<PORTSDA);

            // Have the USI send an ACK.
            USIDR = 0;

            // Update the interrupt enable, wire mode and clock settings.
            USICR = (1<<USISIE) | (1<<USIOIE) |                 // Enable Start Condition and overflow interrupt.
                    (1<<USIWM1) | (1<<USIWM0) |                 // Maintain USI in Two-wire mode.
                    (1<<USICS1) | (0<<USICS0) | (0<<USICLK) |   // Shift Register Clock Source = External, positive edge
                    (0<<USITC);                                 // No toggle of clock pin.

            // Clear the interrupt flags and reset the counter for one bit.
            USISR = (1<<USISIF) | (1<<USIOIF) | (1<<USIPF) |        // Clear interrupt flags.
                    (0x0E<<USICNT0);                                // USI to sample 1 bit or two edge toggles.

            break;
    }
}


