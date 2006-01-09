/*
   Portions of this file derived from gcrt1.S v1.8 from avr-libc project.
   See the following link for details:

   http://www.nongnu.org/avr-libc/

   Original copyright notice included below:

   Copyright (c) 2002, Marek Michalkiewicz <marekm@amelek.gda.pl>
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

   Copyright for original code not derived from gcrt1.S:

   Copyright (c) 2005, Mike Thompson <mpthompson@gmail.com>
   All rights reserved.
*/

#if (__GNUC__ < 3) || (__GNUC__ == 3 && __GNUC_MINOR__ < 3)
#error "GCC version >= 3.3 required"
#endif

/* Hack to get around GCC limitation regarding ATtiny45 for now. */
#undef __AVR_ATtiny2313__
#define __AVR_ATtiny45__

#include <avr/io.h>

    .macro  vector name
    .if (. - __vectors < _VECTORS_SIZE)
    .weak   \name
    .set    \name, __bad_interrupt
    rjmp    \name
    .endif
    .endm

    .section .vectors,"ax",@progbits
    .global __vectors
    .func   __vectors
__vectors:
    rjmp    __init
    vector  __vector_1
    vector  __vector_2
    vector  __vector_3
    vector  __vector_4
    vector  __vector_5
    vector  __vector_6
    vector  __vector_7
    vector  __vector_8
    vector  __vector_9
    vector  __vector_10
    vector  __vector_11
    vector  __vector_12
    vector  __vector_13
    vector  __vector_14
    vector  __vector_15
    vector  __vector_16
    vector  __vector_17
    vector  __vector_18
    vector  __vector_19
    vector  __vector_20
    vector  __vector_21
    vector  __vector_22
    vector  __vector_23
    vector  __vector_24
    vector  __vector_25
    vector  __vector_26
    vector  __vector_27
    vector  __vector_28
    vector  __vector_29
    vector  __vector_30
    vector  __vector_31
    vector  __vector_32
    vector  __vector_33
    vector  __vector_34
    vector  __vector_35
    vector  __vector_36
    vector  __vector_37
    vector  __vector_38
    vector  __vector_39
    vector  __vector_40
    vector  __vector_41
    vector  __vector_42
    vector  __vector_43
    vector  __vector_44
    vector  __vector_45
    vector  __vector_46
    vector  __vector_47
    vector  __vector_48
    vector  __vector_49
    vector  __vector_50
    vector  __vector_51
    vector  __vector_52
    vector  __vector_53
    vector  __vector_54
    vector  __vector_55
    vector  __vector_56
    .endfunc

    .global __boot_vector
__boot_vector:

    /* Jump vector for bootloader. */
    rjmp    __boot_start

    /* Handle unexpected interrupts (enabled and no handler) by
       jumping to the __vector_default function defined by the user,
       otherwise jump to the reset address.

       This must be in a different section, otherwise the assembler
       will resolve "rjmp" offsets and there will be no relocs.  */

    .text
    .global __bad_interrupt
    .func   __bad_interrupt
__bad_interrupt:
    .weak   __vector_default
    .set    __vector_default, __vectors
    rjmp    __vector_default
    .endfunc

    .section .init0,"ax",@progbits

    .global __boot_start
__boot_start:

    .weak   __init
__init:

    .weak   __stack
    .set    __stack, RAMEND

    .weak   __heap_end
    .set    __heap_end, 0

    .section .init2,"ax",@progbits

    /* Clear out the status register. */
    clr r1
    out _SFR_IO_ADDR(SREG), r1

    /* Configure stack. */
    ldi r28,lo8(__stack)
    ldi r29,hi8(__stack)
    out _SFR_IO_ADDR(SPH), r29
    out _SFR_IO_ADDR(SPL), r28

    /* Other init sections provided by libgcc.S will be linked as needed here. */

    .section .init9,"ax",@progbits

    /* Jump to the main function. */
    rjmp    main

