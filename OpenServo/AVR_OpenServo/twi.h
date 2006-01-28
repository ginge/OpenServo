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
