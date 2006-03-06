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

#include <inttypes.h>
#include <avr/io.h>

#include "bootloader.h"
#include "prog.h"
#include "twi.h"

// TWI write states.
#define TWI_WRITE_ADDR_HI_BYTE              (0x00)
#define TWI_WRITE_ADDR_LO_BYTE              (0x01)
#define TWI_WRITE_DATA_BYTE                 (0x02)

// TWI state information.
static uint8_t twi_write_state;
static uint16_t twi_address;

BOOTLOADER_SECTION void twi_init(void)
// Initialise TWI hardware for slave mode.
{
    // Initialize the TWI state information.
    twi_write_state = TWI_WRITE_ADDR_HI_BYTE;
    twi_address = 0;

    // Set own TWI slave address.
    TWAR = TWI_SLAVE_ADDRESS << 1;

    // Default content = SDA released.
    TWDR = 0xFF;

    // Initialize the TWI interrupt to wait for a new event.
    TWCR = (1<<TWEN) |                                  // Keep the TWI interface enabled.
           (1<<TWIE) |                                  // Keep the TWI interrupt enabled.
           (0<<TWSTA) |                                 // Don't generate start condition.
           (0<<TWSTO) |                                 // Don't generate stop condition.
           (1<<TWINT) |                                 // Clear the TWI interrupt.
           (1<<TWEA) |                                  // Acknowledge the data.
           (0<<TWWC);                                   //
}


BOOTLOADER_SECTION void twi_deinit(void)
// De-initialise TWI hardware.
{
    // Reset the TWI registers.
    TWAR = 0;
    TWDR = 0;
    TWCR = 0;
}


BOOTLOADER_SECTION void twi_check_conditions(void)
// Checks for TWI interrupt conditions.
{
    // Check for TWI interrupt condition.
    if (TWCR & (1<<TWINT))
    {
        // Handle the TWI interrupt condition.
        twi_handle_interrupt_condition();
    }

}


BOOTLOADER_SECTION void twi_handle_interrupt_condition(void)
{
    switch (TWSR)
    {
        // Own SLA+R has been received; ACK has been returned.
        case TWI_STX_ADR_ACK:
        // Data byte in TWDR has been transmitted; ACK has been received.
        case TWI_STX_DATA_ACK:

            // Get the data to send.
            TWDR = prog_buffer_get_byte();

            // Data byte will be transmitted and ACK should be received.
            TWCR = (1<<TWEN) |                              // Keep the TWI interface enabled.
                   (1<<TWIE) |                              // Keep the TWI interrupt enabled.
                   (0<<TWSTA) |                             // Don't generate start condition.
                   (0<<TWSTO) |                             // Don't generate stop condition.
                   (1<<TWINT) |                             // Clear the TWI interrupt.
                   (1<<TWEA) |                              // Acknowledge the data.
                   (0<<TWWC);                               //
            break;

        // Data byte in TWDR has been transmitted; NOT ACK has been received.
        case TWI_STX_DATA_NACK:
        // Last data byte in TWDR has been transmitted (TWEA = "0"); ACK has been received.
        case TWI_STX_DATA_ACK_LAST_BYTE:

            // Switched to the not addressed slave mode; own SLA will be recognized.
            TWCR = (1<<TWEN) |                              // Keep the TWI interface enabled.
                   (1<<TWIE) |                              // Keep the TWI interrupt enabled.
                   (0<<TWSTA) |                             // Don't generate start condition.
                   (0<<TWSTO) |                             // Don't generate stop condition.
                   (1<<TWINT) |                             // Clear the TWI interrupt.
                   (1<<TWEA) |                              // Acknowledge the data.
                   (0<<TWWC);                               //
            break;

        // Own SLA+W has been received; ACK has been returned.
        case TWI_SRX_ADR_ACK:

            // Reset the write state.
            twi_write_state = TWI_WRITE_ADDR_HI_BYTE;

            // Data byte will be received and ACK will be returned.
            TWCR = (1<<TWEN) |                              // Keep the TWI interface enabled.
                   (1<<TWIE) |                              // Keep the TWI interrupt enabled.
                   (0<<TWSTA) |                             // Don't generate start condition.
                   (0<<TWSTO) |                             // Don't generate stop condition.
                   (1<<TWINT) |                             // Clear the TWI interrupt.
                   (1<<TWEA) |                              // Acknowledge the data.
                   (0<<TWWC);                               //

            break;

        // Previously addressed with own SLA+W; data has been received; ACK has been returned.
        case TWI_SRX_ADR_DATA_ACK:

            // Check the TWI write state to determine what type of byte we received.
            if (twi_write_state == TWI_WRITE_ADDR_HI_BYTE)
            {
                // Set the twi address high byte.
                twi_address = TWDR;

                // Set the next state.
                twi_write_state = TWI_WRITE_ADDR_LO_BYTE;

                // Data byte will be received and ACK will be returned.
                TWCR = (1<<TWEN) |                          // Keep the TWI interface enabled.
                       (1<<TWIE) |                          // Keep the TWI interrupt enabled.
                       (0<<TWSTA) |                         // Don't generate start condition.
                       (0<<TWSTO) |                         // Don't generate stop condition.
                       (1<<TWINT) |                         // Clear the TWI interrupt.
                       (1<<TWEA) |                          // Acknowledge the data.
                       (0<<TWWC);                           //
            }
            else if (twi_write_state == TWI_WRITE_ADDR_LO_BYTE)
            {
                // Set the address low byte.
                twi_address = (twi_address << 8) | TWDR;

                // Mark the bootloader as active.
                bootloader_active = 1;

                // Set the next state.
                twi_write_state = TWI_WRITE_DATA_BYTE;

                // Set the twi address.  This will load the corresponding page from
                // flash into the programming buffer for reading and writing.
                prog_buffer_set_address(twi_address);

                // Data byte will be received and ACK will be returned.
                TWCR = (1<<TWEN) |                          // Keep the TWI interface enabled.
                       (1<<TWIE) |                          // Keep the TWI interrupt enabled.
                       (0<<TWSTA) |                         // Don't generate start condition.
                       (0<<TWSTO) |                         // Don't generate stop condition.
                       (1<<TWINT) |                         // Clear the TWI interrupt.
                       (1<<TWEA) |                          // Acknowledge the data.
                       (0<<TWWC);                           //
            }
            else
            {
                // Write the data to the buffer.
                prog_buffer_set_byte(TWDR);

                // Data byte will be received and ACK will be returned.
                TWCR = (1<<TWEN) |                          // Keep the TWI interface enabled.
                       (1<<TWIE) |                          // Keep the TWI interrupt enabled.
                       (0<<TWSTA) |                         // Don't generate start condition.
                       (0<<TWSTO) |                         // Don't generate stop condition.
                       (1<<TWINT) |                         // Clear the TWI interrupt.
                       (1<<TWEA) |                          // Acknowledge the data.
                       (0<<TWWC);                           //
            }

            break;

        // A STOP condition or repeated START condition has been received while still addressed as Slave.
        case TWI_SRX_STOP_RESTART:

             // Switch to the not addressed slave mode; own SLA will be recognized.
             TWCR = (1<<TWEN) |                              // Keep the TWI interface enabled.
                    (1<<TWIE) |                              // Keep the TWI interrupt enabled.
                    (0<<TWSTA) |                             // Don't generate start condition.
                    (0<<TWSTO) |                             // Don't generate stop condition.
                    (1<<TWINT) |                             // Clear the TWI interrupt.
                    (1<<TWEA) |                              // Acknowledge the data.
                    (0<<TWWC);                               //

            // Update the programming buffer if needed.
            prog_buffer_update();

            break;

        // Bus error due to an illegal START or STOP condition.
        case TWI_BUS_ERROR:

            // Only the internal hardware is affected, no STOP condition is sent on the bus.
            // In all cases, the bus is released and TWSTO is cleared.
            TWCR = (1<<TWEN) |                              // Keep the TWI interface enabled.
                   (1<<TWIE) |                              // Keep the TWI interrupt enabled.
                   (0<<TWSTA) |                             // Don't generate start condition.
                   (1<<TWSTO) |                             // Don't generate stop condition.
                   (1<<TWINT) |                             // Clear the TWI interrupt.
                   (1<<TWEA) |                              // Acknowledge the data.
                   (0<<TWWC);                               //
            break;

        // No relevant state information available; TWINT="0".
        case TWI_NO_STATE:

            // No action required.
            break;

#if 0
        // Previously addressed with own SLA+W; data has been received; NOT ACK has been returned.
        case TWI_SRX_ADR_DATA_NACK:
#endif
        default:

            // Reset the TWI interrupt to wait for a new event.
            TWCR = (1<<TWEN) |                                  // Keep the TWI interface enabled.
                   (1<<TWIE) |                                  // Keep the TWI interrupt enabled.
                   (0<<TWSTA) |                                 // Don't generate start condition.
                   (0<<TWSTO) |                                 // Don't generate stop condition.
                   (1<<TWINT) |                                 // Clear the TWI interrupt.
                   (1<<TWEA) |                                  // Acknowledge the data.
                   (0<<TWWC);                                   //
            break;
    }
}

