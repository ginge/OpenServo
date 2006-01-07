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

/* Hack to get around GCC limitation regarding ATtiny45 for now. */
#undef __AVR_ATtiny2313__
#define __AVR_ATtiny45__

/* Don't build if BOOTSTRAPPER defined. */
#ifndef BOOTSTRAPPER

#include <avr/io.h>

	.text

	/* Define this code section as the bootloader. */
	.section	.bootloader,"ax",@progbits

	.global	bootstart
bootstart:

	.global	__init
__init:

	.weak	__stack
	.set	__stack, RAMEND

	/* Make sure global interrupts are disabled. */
	cli

	/* Clear the status register. */
	clr	r1
	out	_SFR_IO_ADDR(SREG), r1

	/* Clear out MCUSR. */
	out _SFR_IO_ADDR(MCUSR), r1 

	/* Disable watchdog. */
	in r16, _SFR_IO_ADDR(WDTCR)
	ori r16, (1<<WDCE)|(1<<WDE)
	out _SFR_IO_ADDR(WDTCR), r16
	clr r16
	out _SFR_IO_ADDR(WDTCR), r16

	/* Configure stack. */
	ldi	r28,lo8(__stack)
	ldi	r29,hi8(__stack)
	out	_SFR_IO_ADDR(SPH),r29
	out	_SFR_IO_ADDR(SPL),r28

	/* Call the boot loader function. */
	rcall bootloader

	/* Jump to boot vector address. */
	rjmp	__boot_vector

	.size	bootstart,.-bootstart

#endif /* !BOOTSTRAPPER */
