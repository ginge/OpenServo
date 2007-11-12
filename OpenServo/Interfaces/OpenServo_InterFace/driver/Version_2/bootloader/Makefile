# ======================================================================
# Makefile for USB SPI interface
#
# Copyright (C) 2006 Dick Streefland
#
# This is free software, licensed under the terms of the GNU General
# Public License as published by the Free Software Foundation.
# ======================================================================

BOOTLOADER_ADDRESS = 0x1800
USBTINY		= ../usbtiny
TARGET_ARCH	= -mmcu=atmega8
OBJECTS		= main.o
FLASH		= avrdude -p m8 -U flash:w:main.hex
STACK		= 32
SCHEM		= spi.ps
LDEXTRAFLAGS    = -Wl,--section-start=.text=$(BOOTLOADER_ADDRESS)

include $(USBTINY)/common.mk
