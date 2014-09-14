// ======================================================================
// Common definitions for the USB driver
//
// Copyright 2006-2010 Dick Streefland
//
// This is free software, licensed under the terms of the GNU General
// Public License as published by the Free Software Foundation.
// ======================================================================

#ifdef __ASSEMBLER__
#define	__SFR_OFFSET		0
#endif
#include <avr/io.h>
#include "usbtiny.h"

// Preprocessor magic
#define	CAT2(a,b)		CAT2EXP(a, b)
#define	CAT2EXP(a,b)		a ## b
#define	CAT3(a,b,c)		CAT3EXP(a, b, c)
#define	CAT3EXP(a,b,c)		a ## b ## c

// I/O Ports for USB
#define	USB_IN			CAT2(PIN, USBTINY_PORT)
#define	USB_OUT			CAT2(PORT, USBTINY_PORT)
#define	USB_DDR			CAT2(DDR, USBTINY_PORT)

// I/O bit masks for USB
#define	USB_MASK_DMINUS		(1 << (USBTINY_DMINUS))
#define	USB_MASK_DPLUS		(1 << (USBTINY_DPLUS))
#define	USB_MASK		(USB_MASK_DMINUS | USB_MASK_DPLUS)

// Interrupt configuration
#if	defined	EICRA
#  define USB_INT_CONFIG	EICRA
#else
#  define USB_INT_CONFIG	MCUCR
#endif
#define	USB_INT_CONFIG_SET	((1 << CAT3(ISC,USBTINY_INT,1)) | (1 << CAT3(ISC,USBTINY_INT,0)))
#define USB_INT_VECTOR		CAT3(INT, USBTINY_INT, _vect)

// Interrupt enable
#if	defined	GIMSK
#  define USB_INT_ENABLE	GIMSK
#elif	defined	EIMSK
#  define USB_INT_ENABLE	EIMSK
#else
#  define USB_INT_ENABLE	GICR
#endif
#define	USB_INT_ENABLE_BIT	CAT2(INT,USBTINY_INT)

// Interrupt pending bit
#if	defined	EIFR
#  define USB_INT_PENDING	EIFR
#else
#  define USB_INT_PENDING	GIFR
#endif
#define	USB_INT_PENDING_BIT	CAT2(INTF,USBTINY_INT)
#if	defined INF0 && ! defined INTF0
#  define	INTF0		INF0	// fix for incorrect definition in iotn13.h
#endif

// USB PID values
#define	USB_PID_SETUP		0x2d
#define	USB_PID_OUT		0xe1
#define	USB_PID_IN		0x69
#define	USB_PID_DATA0		0xc3
#define	USB_PID_DATA1		0x4b
#define	USB_PID_ACK		0xd2
#define	USB_PID_NAK		0x5a
#define	USB_PID_STALL		0x1e

// Various constants
#define	USB_BUFSIZE		11	// PID + data + CRC

// Bit manipulation macros
#define	BIT_CLR(reg,bit)	{ (reg) &= ~ _BV(bit); }
#define	BIT_SET(reg,bit)	{ (reg) |=   _BV(bit); }
#define	BIT_TST(reg,bit)	(((reg) & _BV(bit)) != 0)

// I/O port manipulation macros
#define	DDR_CLR(p,b)		BIT_CLR(DDR  ## p, b)
#define	DDR_SET(p,b)		BIT_SET(DDR  ## p, b)
#define	PORT_CLR(p,b)		BIT_CLR(PORT ## p, b)
#define	PORT_SET(p,b)		BIT_SET(PORT ## p, b)
#define	PORT_TST(p,b)		BIT_TST(PORT ## p, b)
#define	PIN_TST(p,b)		BIT_TST(PIN  ## p, b)
#define	PIN_SET(p,b)		BIT_SET(PIN  ## p, b)

// Macros that can be used with an argument of the form (port,bit)
#define	INPUT(bit)		DDR_CLR bit
#define	OUTPUT(bit)		DDR_SET bit
#define	CLR(bit)		PORT_CLR bit
#define	SET(bit)		PORT_SET bit
#define	ISSET(bit)		PORT_TST bit
#define	TST(bit)		PIN_TST bit
#define	TOGGLE(bit)		PIN_SET bit
