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

#define TWI_CMD_RESET               0x80        // Reset command
#define TWI_CMD_CHECKED_TXN         0x81        // Read/Write registers with simple checksum
#define TWI_CMD_PWM_ENABLE          0x82        // Enable PWM to motors
#define TWI_CMD_PWM_DISABLE         0x83        // Disable PWM to servo motors
#define TWI_CMD_WRITE_ENABLE        0x84        // Enable write of safe read/write registers
#define TWI_CMD_WRITE_DISABLE       0x85        // Disable write of safe read/write registers
#define TWI_CMD_REGISTERS_SAVE      0x86        // Save safe read/write registers fo EEPROM
#define TWI_CMD_REGISTERS_RESTORE   0x87        // Restore safe read/write registers from EEPROM
#define TWI_CMD_REGISTERS_DEFAULT   0x88        // Restore safe read/write registers to defaults

void twi_slave_init(uint8_t);
void twi_transmit_byte(uint8_t);
uint8_t twi_receive_byte(void);
uint8_t twi_data_in_receive_buffer(void);


#endif // _OS_TWI_H_
