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

#ifndef _OS_TWI_H_
#define _OS_TWI_H_ 1

#define TWI_CMD_RESET                   0x80        // Reset command
#define TWI_CMD_CHECKED_TXN             0x81        // Read/Write registers with simple checksum
#define TWI_CMD_PWM_ENABLE              0x82        // Enable PWM to motors
#define TWI_CMD_PWM_DISABLE             0x83        // Disable PWM to servo motors
#define TWI_CMD_WRITE_ENABLE            0x84        // Enable write of safe read/write registers
#define TWI_CMD_WRITE_DISABLE           0x85        // Disable write of safe read/write registers
#define TWI_CMD_REGISTERS_SAVE          0x86        // Save safe read/write registers fo EEPROM
#define TWI_CMD_REGISTERS_RESTORE       0x87        // Restore safe read/write registers from EEPROM
#define TWI_CMD_REGISTERS_DEFAULT       0x88        // Restore safe read/write registers to defaults
#define TWI_CMD_EEPROM_ERASE            0x89        // Erase the EEPROM.
#define TWI_CMD_VOLTAGE_READ            0x90        // Starts a ADC on the supply voltage channel
#define TWI_CMD_CURVE_MOTION_ENABLE     0x91        // Enable curve motion processing.
#define TWI_CMD_CURVE_MOTION_DISABLE    0x92        // Disable curve motion processing.
#define TWI_CMD_CURVE_MOTION_RESET      0x93        // Reset the curve motion buffer.
#define TWI_CMD_CURVE_MOTION_APPEND     0x94        // Append curve motion data.


#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega88__)|| defined(__AVR_ATmega168__)

//
// ATmega TWI State codes
//
// General TWI Master staus codes
#define TWI_START                  0x08  // START has been transmitted
#define TWI_REP_START              0x10  // Repeated START has been transmitted
#define TWI_ARB_LOST               0x38  // Arbitration lost

// TWI Master Transmitter staus codes
#define TWI_MTX_ADR_ACK            0x18  // SLA+W has been tramsmitted and ACK received
#define TWI_MTX_ADR_NACK           0x20  // SLA+W has been tramsmitted and NACK received
#define TWI_MTX_DATA_ACK           0x28  // Data byte has been tramsmitted and ACK received
#define TWI_MTX_DATA_NACK          0x30  // Data byte has been tramsmitted and NACK received

// TWI Master Receiver staus codes
#define TWI_MRX_ADR_ACK            0x40  // SLA+R has been tramsmitted and ACK received
#define TWI_MRX_ADR_NACK           0x48  // SLA+R has been tramsmitted and NACK received
#define TWI_MRX_DATA_ACK           0x50  // Data byte has been received and ACK tramsmitted
#define TWI_MRX_DATA_NACK          0x58  // Data byte has been received and NACK tramsmitted

// TWI Slave Transmitter staus codes
#define TWI_STX_ADR_ACK            0xA8  // Own SLA+R has been received; ACK has been returned
#define TWI_STX_ADR_ACK_M_ARB_LOST 0xB0  // Arbitration lost in SLA+R/W as Master; own SLA+R has been received; ACK has been returned
#define TWI_STX_DATA_ACK           0xB8  // Data byte in TWDR has been transmitted; ACK has been received
#define TWI_STX_DATA_NACK          0xC0  // Data byte in TWDR has been transmitted; NOT ACK has been received
#define TWI_STX_DATA_ACK_LAST_BYTE 0xC8  // Last data byte in TWDR has been transmitted (TWEA = "0"); ACK has been received

// TWI Slave Receiver staus codes
#define TWI_SRX_ADR_ACK            0x60  // Own SLA+W has been received ACK has been returned
#define TWI_SRX_ADR_ACK_M_ARB_LOST 0x68  // Arbitration lost in SLA+R/W as Master; own SLA+W has been received; ACK has been returned
#define TWI_SRX_GEN_ACK            0x70  // General call address has been received; ACK has been returned
#define TWI_SRX_GEN_ACK_M_ARB_LOST 0x78  // Arbitration lost in SLA+R/W as Master; General call address has been received; ACK has been returned
#define TWI_SRX_ADR_DATA_ACK       0x80  // Previously addressed with own SLA+W; data has been received; ACK has been returned
#define TWI_SRX_ADR_DATA_NACK      0x88  // Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
#define TWI_SRX_GEN_DATA_ACK       0x90  // Previously addressed with general call; data has been received; ACK has been returned
#define TWI_SRX_GEN_DATA_NACK      0x98  // Previously addressed with general call; data has been received; NOT ACK has been returned
#define TWI_SRX_STOP_RESTART       0xA0  // A STOP condition or repeated START condition has been received while still addressed as Slave

// TWI Miscellaneous status codes
#define TWI_NO_STATE               0xF8  // No relevant state information available; TWINT = "0"
#define TWI_BUS_ERROR              0x00  // Bus error due to an illegal START or STOP condition

#endif // __AVR_ATmega8__ || __AVR_ATmega88__ || __AVR_ATmega168__

void twi_slave_init(uint8_t);
uint8_t twi_receive_byte(void);
uint8_t twi_data_in_receive_buffer(void);

#endif // _OS_TWI_H_
