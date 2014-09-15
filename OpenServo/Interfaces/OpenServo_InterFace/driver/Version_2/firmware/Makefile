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
OBJECTS		= main.o i2c_master.o uart.o pwm_io.o spi.o
FLASH_CMD	= avrdude -c usbtiny -p m8 -U lfuse:w:0x9f:m -U hfuse:w:0xc8:m -U flash:w:main.hex
FUSES_CMD	= avrdude -c usbtiny -p m8 -U lfuse:w:0x9f:m
STACK		= 32
FLASH		= 8192
SRAM		= 1024
SCHEM		= spi.ps
LDEXTRAFLAGS	= 
include $(USBTINY)/common.mk
