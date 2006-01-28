/*
   Copyright (c) 2006, Mike Thompson <mpthompson@gmail.com>
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

// Set define to 1 to enable checksum handling, 0 to disable checksum handling.
#define TWI_CHECKED_ENABLED                 1

//////////////////////////////////////////////////////////////////
///////////////// Driver Buffer Definitions //////////////////////
//////////////////////////////////////////////////////////////////
// 1,2,4,8,16,32,64,128 or 256 bytes are allowed buffer sizes

#define TWI_RX_BUFFER_SIZE (4)
#define TWI_RX_BUFFER_MASK (TWI_RX_BUFFER_SIZE - 1)

#if (TWI_RX_BUFFER_SIZE & TWI_RX_BUFFER_MASK)
        #error TWI RX buffer size is not a power of 2
#endif

#define TWI_CHK_WRITE_BUFFER_SIZE (16)
#define TWI_CHK_WRITE_BUFFER_MASK (TWI_CHK_WRITE_BUFFER_SIZE - 1)

#if (TWI_CHK_WRITE_BUFFER_SIZE & TWI_CHK_WRITE_BUFFER_MASK)
        #error TWI CHK WRITE buffer size is not a power of 2
#endif

//////////////////////////////////////////////////////////////////

// TWI acknowledgment values.
#define TWI_ACK                             (0x00)
#define TWI_NAK                             (0x01)

// Overflow state values.
#define TWI_OVERFLOW_STATE_NONE             (0x00)
#define TWI_OVERFLOW_STATE_ACK_PR_RX        (0x01)
#define TWI_OVERFLOW_STATE_DATA_RX          (0x02)
#define TWI_OVERFLOW_STATE_ACK_PR_TX        (0x03)
#define TWI_OVERFLOW_STATE_PR_ACK_TX        (0x04)
#define TWI_OVERFLOW_STATE_DATA_TX          (0x05)

// Data state values.
#define TWI_DATA_STATE_COMMAND              (0x00)
#define TWI_DATA_STATE_DATA                 (0x01)
#if TWI_CHECKED_ENABLED
#define TWI_DATA_STATE_CHECKED_COUNTING     (0x02)
#define TWI_DATA_STATE_CHECKED_ADDRESS      (0x03)
#define TWI_DATA_STATE_CHECKED_DATA         (0x04)
#endif

// Device dependant defines
#if defined(__AVR_ATtiny25__) | defined(__AVR_ATtiny45__) | defined(__AVR_ATtiny85__)
    #define DDR_USI                         DDRB
    #define DD_SDA                          DDB0
    #define DD_SCL                          DDB2
    #define PORT_USI                        PORTB
    #define P_SDA                           PB0
    #define P_SCL                           PB2
    #define PIN_USI                         PINB
    #define PIN_SDA                         PINB0
    #define PIN_SCL                         PINB2
#endif

// Locals.
static uint8_t twi_slave_address;
static volatile uint8_t twi_address;
static volatile uint8_t twi_data_state;
static volatile uint8_t twi_overflow_state;

static volatile uint8_t twi_rxhead;
static volatile uint8_t twi_rxtail;
static uint8_t twi_rxbuf[TWI_RX_BUFFER_SIZE];

#if TWI_CHECKED_ENABLED
static uint8_t twi_chk_count;            // current byte in transaction
static uint8_t twi_chk_count_target;     // How many bytes are we reading/writing
static uint8_t twi_chk_sum;              // Accumulator for checksum
static uint8_t twi_chk_write_buffer[TWI_CHK_WRITE_BUFFER_SIZE];
#endif

static uint8_t twi_registers_read(uint8_t address)
// Read registers.  This function is meant to safely read
// the registers for the TWI master.
{
    // Prevent overflow by wrapping.
    address &= 0x7F;

    // Avoid reading unimplemented registers.
    if (address > MAX_REGISTER) return 0;

    // Read the register.
    return registers_read_byte(address);
}


static void twi_registers_write(uint8_t address, uint8_t data)
// Write non-write protected registers.  This function
// is meant to safely write registers for the TWI master.
{
    // Prevent overflow by wrapping.
    address &= 0x7F;

    // Avoid writing unimplemented registers.
    if (address > MAX_REGISTER) return;

    // Don't write to read/only registers.
    if (address <= MAX_RO_REGISTER) return;

    // Don't write to safe read/write registers unless enabled.
    if ((address > MAX_RW_REGISTER) && registers_is_write_disabled()) return;

    // Write the data to the address address.
    registers_write_byte(address, data);
}


#if TWI_CHECKED_ENABLED
static void twi_write_buffer(void)
// Write the recieve buffer to memory.
{
    // Loop over the data within the write buffer.
    for (twi_chk_count = 0; twi_chk_count < twi_chk_count_target; twi_chk_count++)
    {
        // Write the data to the addressed register.
        twi_registers_write(twi_address, twi_chk_write_buffer[twi_chk_count & TWI_CHK_WRITE_BUFFER_MASK]);

        // Increment to the next address.
        ++twi_address;
    }
}
#endif


static uint8_t twi_read_data()
// Handle checked/non-checked read of data.
{
    // By default read the data to be returned.
    uint8_t usi_data = twi_registers_read(twi_address);

#if TWI_CHECKED_ENABLED
    // Are we handling checked data?
    if (twi_data_state == TWI_DATA_STATE_CHECKED_DATA)
    {
        // Have we reached the end of the read?
        if (twi_chk_count < twi_chk_count_target)
        {
            // Add the data to the check sum.
            twi_chk_sum += usi_data;

            // Increment the check sum data count.
            ++twi_chk_count;

            // Increment the address.
            ++twi_address;
        }
        else
        {
            // Replace the data with the checksum.
            usi_data = twi_chk_sum;
        }
    }
    else
    {
        // Increment the address.
        ++twi_address;
    }
#else
    // Increment the address.
    ++twi_address;
#endif

    return usi_data;
}


static uint8_t twi_write_data(uint8_t usi_data)
// Handle checked/non-checked write of data or command.
{
    // By default, return ACK from write.
    uint8_t ack = TWI_ACK;

    // Handle the write depending on the write state.
    switch (twi_data_state)
    {
        case TWI_DATA_STATE_COMMAND:

            // This is a byte.
            if (usi_data < TWI_CMD_RESET)
            {
                // Capture the address.
                twi_address = usi_data;

                // Update the write state.
                twi_data_state = TWI_DATA_STATE_DATA;
            }
#if TWI_CHECKED_ENABLED
            else if (usi_data == TWI_CMD_CHECKED_TXN)
            {
                // Update the write state.
                twi_data_state = TWI_DATA_STATE_CHECKED_COUNTING;
            }
#endif
            else
            {
                // Handle the command asynchronously.
                twi_rxhead = (twi_rxhead + 1) & TWI_RX_BUFFER_MASK;
                twi_rxbuf[twi_rxhead] = usi_data;
            }

            break;

        case TWI_DATA_STATE_DATA:

            // Write the data to the addressed register.
            twi_registers_write(twi_address, usi_data);

            // Increment to the next address.
            ++twi_address;

            break;

#if TWI_CHECKED_ENABLED
        case TWI_DATA_STATE_CHECKED_COUNTING:

            // Read in the count (Make sure it's less than the max count).
            twi_chk_count_target = usi_data & TWI_CHK_WRITE_BUFFER_MASK;

            // Clear the checksum and count.
            twi_chk_sum = twi_chk_count = 0;

            // Update the write state.
            twi_data_state = TWI_DATA_STATE_CHECKED_ADDRESS;

            break;

        case TWI_DATA_STATE_CHECKED_ADDRESS:

            // Capture the address.
            twi_address = usi_data;

            // Update the write state.
            twi_data_state = TWI_DATA_STATE_CHECKED_DATA;

            break;

        case TWI_DATA_STATE_CHECKED_DATA:

            // Have we reached the end of the write?
            if (twi_chk_count < twi_chk_count_target)
            {
                // No. Write the data to the checksum buffer
                twi_chk_write_buffer[twi_chk_count & TWI_CHK_WRITE_BUFFER_MASK] = usi_data;

                // Add the data to the checksum.
                twi_chk_sum += usi_data;

                // Increment the check sum data count.
                ++twi_chk_count;
            }
            else
            {
                // Verify the checksum
                if (usi_data == twi_chk_sum)
                {
                    // Write the checksum buffer to addressed registers.
                    twi_write_buffer();
                }
                else
                {
                    // Checksum failed so return NACK.
                    ack = TWI_NAK;
                }
            }

            break;
#endif
    }

    return ack;
}


void
twi_slave_init(uint8_t slave_address)
// Initialise USI for TWI slave mode.
{
    // Flush the buffers.
    twi_rxtail = 0;
    twi_rxhead = 0;

    // Set the slave address.
    twi_slave_address = slave_address & 0x7f;

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


SIGNAL(SIG_USI_OVERFLOW)
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
            if ((usi_data >> 1) == twi_slave_address)
            {
                // Are we transmitting or receiving data?
                if (usi_data & 0x01)
                {
                    // We are to transmit data.  Reset the overflow 
                    // state, but preserve the data state from the last write.
                    twi_overflow_state = TWI_OVERFLOW_STATE_ACK_PR_TX;
                }
                else
                {
                    // We are receiving data.  Set data and overflow state.
                    twi_data_state = TWI_DATA_STATE_COMMAND;
                    twi_overflow_state = TWI_OVERFLOW_STATE_ACK_PR_RX;
                }

                // Set SDA for output.
                PORT_USI |= (1<<P_SDA);
                DDR_USI |= (1<<DD_SDA);

                // Load data for ACK.
                USIDR = TWI_ACK;

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

            // Write the data and return ack/nack.
            USIDR = twi_write_data(usi_data);

            // Set SDA for output.
            PORT_USI |= (1<<P_SDA);
            DDR_USI |= (1<<DD_SDA);

            // Reload counter for ACK -- two clock transitions.
            USISR = 0x0E;

            break;

        // ACK received from master.  Reset USI state if NACK received.
        case TWI_OVERFLOW_STATE_PR_ACK_TX:

            // Check the lowest bit for NACK?  If set, the master does not want more data.
            if (usi_data & 0x01)
            {
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

            // Read the checked/non-checked data.
            USIDR = twi_read_data();

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


