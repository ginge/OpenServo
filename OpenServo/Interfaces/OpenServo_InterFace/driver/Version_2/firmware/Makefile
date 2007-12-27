# ======================================================================
# Makefile for USB SPI interface
#
# Copyright (C) 2006 Dick Streefland
#
# This is free software, licensed under the terms of the GNU General
# Public License as published by the Free Software Foundation.
# ======================================================================

USBTINY		= ../usbtiny
TARGET_ARCH	= -mmcu=atmega8
OBJECTS		= main.o
FLASH		= avrdude -c usbtiny -p m8 -U lfuse:w:0x9f:m -U hfuse:w:0xc8:m -U flash:w:main.hex
STACK		= 32
SCHEM		= spi.ps
LDEXTRAFLAGS    = 
include $(USBTINY)/common.mk
