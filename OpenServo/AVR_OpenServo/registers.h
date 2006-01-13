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

#ifndef _OS_REGISTERS_H_
#define _OS_REGISTERS_H_ 1

//
// Defines servo control registers.
//

// TWI read/only status registers.  Writing
// values to these registers has no effect.

#define PWM_ENABLE              0x00
#define WRITE_ENABLE            0x01
#define TIMER_HI                0x02
#define TIMER_LO                0x03
#define POSITION_HI             0x04
#define POSITION_LO             0x05
#define POWER_HI                0x06
#define POWER_LO                0x07

#define PWM_CW                  0x08
#define PWM_CCW                 0x09
#define RESERVED_0A             0x0A
#define RESERVED_0B             0x0B
#define RESERVED_0C             0x0C
#define RESERVED_0D             0x0D
#define RESERVED_0E             0x0E
#define RESERVED_0F             0x0F


// TWI read/write registers.  Writing these
// registers controls operation of the servo.

#define SEEK_HI                 0x10
#define SEEK_LO                 0x11
#define MAXIMUM_PWM             0x12
#define RESERVED_13             0x13
#define RESERVED_14             0x14
#define RESERVED_15             0x15
#define RESERVED_16             0x16
#define RESERVED_17             0x17

#define RESERVED_18             0x18
#define RESERVED_19             0x19
#define RESERVED_1A             0x1A
#define RESERVED_1B             0x1B
#define RESERVED_1C             0x1C
#define RESERVED_1D             0x1D
#define RESERVED_1E             0x1E
#define RESERVED_1F             0x1F


// TWI safe read/write registers.  These registers
// may only be written to when write enabled.

#define TWI_ADDRESS             0x20
#define RESERVED_21             0x21
#define PID_PGAIN_HI            0x22
#define PID_PGAIN_LO            0x23
#define PID_DGAIN_HI            0x24
#define PID_DGAIN_LO            0x25
#define PID_IGAIN_HI            0x26
#define PID_IGAIN_LO            0x27

#define MIN_SEEK_HI             0x28
#define MIN_SEEK_LO             0x29
#define MAX_SEEK_HI             0x2a
#define MAX_SEEK_LO             0x2b
#define REVERSE_SEEK            0x2c
#define RESERVED_2D             0x2d
#define RESERVED_2E             0x2e
#define RESERVED_2F             0x2f

#define RESERVED_30             0x30
#define RESERVED_31             0x31
#define RESERVED_32             0x32
#define RESERVED_33             0x33
#define RESERVED_34             0x34
#define RESERVED_35             0x35
#define RESERVED_36             0x36
#define RESERVED_37             0x37

#define RESERVED_38             0x38
#define RESERVED_39             0x39
#define RESERVED_3A             0x3A
#define RESERVED_3B             0x3B
#define RESERVED_3C             0x3C
#define RESERVED_3D             0x3D
#define RESERVED_3E             0x3E
#define RESERVED_3F             0x3F

// Set the register ranges.
#define MIN_RO_REGISTER         0x00
#define MAX_RO_REGISTER         0x0F
#define MIN_RW_REGISTER         0x10
#define MAX_RW_REGISTER         0x1F
#define MIN_SW_REGISTER         0x20
#define MAX_SW_REGISTER         0x3F
#define MAX_REGISTER            MAX_SW_REGISTER

// Global register array.
extern uint8_t registers[MAX_REGISTER + 1];

// Register functions.

void registers_init(void);
void registers_defaults(void);

// Register in-line functions.

// Read a single byte from the registers.
inline static uint8_t registers_read_byte(uint8_t address)
{
    return registers[address];
}


// Write a single byte to the registers.
inline static void registers_write_byte(uint8_t address, uint8_t value)
{
    registers[address] = value;
}


// Read a 16-bit word from the registers.
// Interrupts are disabled during the read.
inline static uint16_t registers_read_word(uint8_t address_hi, uint8_t address_lo)
{
    uint16_t value;

    asm volatile (
        "in __tmp_reg__,__SREG__\n\t"
        "cli\n\t"
        "mov %A0,%2\n\t"
        "mov %B0,%1\n\t"
        "out __SREG__,__tmp_reg__\n\t"
        : "=r" ((uint16_t) (value)) \
        : "r" ((uint8_t) (registers[address_hi])),
          "r" ((uint8_t) (registers[address_lo]))
    );

    return value;
}


// Write a 16-bit word to the registers.
// Interrupts are disabled during the write.
inline static void registers_write_word(uint8_t address_hi, uint8_t address_lo, uint16_t value)
{
    asm volatile (
        "in __tmp_reg__,__SREG__\n\t"
        "cli\n\t"
        "mov %0,%B2\n\t"
        "mov %1,%A2\n\t"
        "out __SREG__,__tmp_reg__\n\t"
        : "=&r" ((uint8_t) (registers[address_hi])),
          "=r" ((uint8_t) (registers[address_lo]))
        : "r" ((uint16_t) (value))
    );
}

#endif // _OS_REGISTERS_H_

