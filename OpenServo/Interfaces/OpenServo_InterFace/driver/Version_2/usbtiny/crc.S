; ======================================================================
; Calculate and append CRC
;
; There are two versions of the CRC16 calculation, selectable by the
; USBTINY_FAST_CRC macro. The default implementation calculates one bit
; at a time, and is compact but relatively slow. The "fast" version
; processes 4 bits at a time, and is about twice as fast, but 42 bytes
; larger.
;
; The fast version calculates 4 bits at a time, using a precomputed table
; of 16 values. Each value is 16 bits, but only the 8 significant bits
; are stored. The table should not cross a 256-byte page. The check.py
; script will check for this. An 8 bit algoritm would be even faster,
; but requires a lookup table of 512 bytes.
;
; Copyright 2006-2010 Dick Streefland
;
; This is free software, licensed under the terms of the GNU General
; Public License as published by the Free Software Foundation.
; ======================================================================

#include "def.h"

#if	USBTINY_FAST_CRC
; ----------------------------------------------------------------------
; void crc(unsigned char *data, unsigned char len);
; ----------------------------------------------------------------------
#define	data	r24
#define	len	r22

#define	b	r18
#define	tmp	r19
#define	zl	r20
#define	crc_l	r24
#define	crc_h	r25

	.text
	.global	crc
	.type	crc, @function
crc:
	; crc = 0xffff
	movw	XL, r24
	ldi	crc_h, 0xff
	ldi	crc_l, 0xff
	ldi	zl, lo8(crc4tab)
	ldi	ZH, hi8(crc4tab)
	rjmp	entry

next_byte:
	; crc ^= b
	ld	b, X+
	eor	crc_l, b

	; index1 = crc & 0x0f
	mov	ZL, crc_l
	andi	ZL, 0x0f

	; tmp = crc4tab[index1]
	add	ZL, zl
	lpm	tmp, Z+

	; index2 = (crc >> 4)
	mov	ZL, crc_l
	swap	ZL

	; crc >>= 8
	mov	crc_l, crc_h

	; index2 = (index2 ^ tmp) & 0xf
	mov	crc_h, tmp
	andi	tmp, 1
	eor	ZL, tmp
	andi	ZL, 0x0f

	; treat upper byte of CRC remainder
	swap	crc_h
	mov	tmp, crc_h
	andi	crc_h, 0x0f
	andi	tmp, 0xe0
	eor	crc_l, tmp

	; crc ^= crc4tab[index]
	add	ZL, zl
	lpm	tmp, Z+
	eor	crc_h, tmp
	andi	tmp, 1
	eor	crc_h, tmp
	eor	crc_l, tmp

entry:
	; next byte
	dec	len
	brpl	next_byte

done:
	; crc ^= 0xffff
	com	crc_l
	com	crc_h

	; append crc to buffer
	st	X+, crc_l
	st	X+, crc_h

	ret

; ----------------------------------------------------------------------
; CRC table. As bits 1..8 are always zero, omit them.
; ----------------------------------------------------------------------
	.section .progmem.crc,"a",@progbits
;;;	.align	4		; crude way to avoid crossing a page boundary
crc4tab:
	.byte	0x00+0x00
	.byte	0xcc+0x01
	.byte	0xd8+0x01
	.byte	0x14+0x00
	.byte	0xf0+0x01
	.byte	0x3c+0x00
	.byte	0x28+0x00
	.byte	0xe4+0x01
	.byte	0xa0+0x01
	.byte	0x6c+0x00
	.byte	0x78+0x00
	.byte	0xb4+0x01
	.byte	0x50+0x00
	.byte	0x9c+0x01
	.byte	0x88+0x01
	.byte	0x44+0x00
/* ---------------------------------------------------------------------- *\
#!/usr/bin/python
for crc in range(16):
	for bit in range(4):
		xor = crc & 1
		crc >>= 1
		if xor:
			crc ^= 0xA001	# X^16 + X^15 + X^2 + 1 (reversed)
	print "\t.byte\t0x%02x+0x%02x" % (crc >> 8, crc & 0xff)
\* ---------------------------------------------------------------------- */
#else
; ----------------------------------------------------------------------
; void crc(unsigned char *data, unsigned char len);
; ----------------------------------------------------------------------
#define	data	r24
#define	len	r22

#define	b	r18
#define	con_01	r19
#define	con_a0	r20
#define	crc_l	r24
#define	crc_h	r25

	.text
	.global	crc
	.type	crc, @function
crc:
	movw	XL, r24
	ldi	crc_h, 0xff
	ldi	crc_l, 0xff
	tst	len
	breq	done1
	ldi	con_a0, 0xa0
	ldi	con_01, 0x01
next_byte:
	ld	b, X+
	eor	crc_l, b
	ldi	b, 8
next_bit:
	lsr	crc_h
	ror	crc_l
	brcc	noxor
	eor	crc_h, con_a0
	eor	crc_l, con_01
noxor:
	dec	b
	brne	next_bit
	dec	len
	brne	next_byte
done1:
	com	crc_l
	com	crc_h
	st	X+, crc_l
	st	X+, crc_h
	ret
#endif
