AVR ATmegaX Bootloader
======================

March 29th, 2007

Mike Thompson
mpthompson@gmail.com

This is an TWI/I2C based bootloader for the Atmel AVR ATmega MCUs meant 
for use with the OpenServo project.  The bootloader will occupy the 
upper 1024 bytes of flash memory leaving the lower portion of flash 
memory available for application code.

At boot time, the bootloader appears as an fairly standard I2C memory 
device at address 127 for about three seconds before it invokes the 
application code. If the bootloader receives a TWI/I2C read or write 
packet within three seconds of starting the bootloader will stay active 
for programming.  To an outside programmer, both the flash and EEPROM 
are mapped as one contiguous address range divided into pages which can 
be read or written to via the TWI/I2C bus.  The bootloader can be exited 
and the application started by reading or writing the magic address of 
0xFFFF.

To invoke the application, the bootloader vector will jump to the reset 
vector location at address 0x0000.

The address map of flash and EEPROM pages as presented by the bootloader 
for each supported microcontroller is shown below.  The flash pages 
devoted to the bootloader will appear as blank pages containing 0xFF 
values and cannot be written.

  ATmega8 Flash/EEPROM Map
  ========================

  Page size = 64 bytes

  Page     Address    Type
  ---------------------------------------------
   00     0000-003F   Flash
   ...
   6F     1BC0-1BFF   Flash
   70     1C00-1C3F   Flash (bootloader)
   ...
   7F     1FC0-1FFF   Flash (bootloader)
   80     2000-203F   EEPROM
   ...
   87     21C0-21FF   EEPROM


  ATmega168 Flash/EEPROM Map
  ==========================

  Page size = 128 bytes

  Page     Address    Type
  ---------------------------------------------
   00     0000-007F   Flash
   ...
   77     3B80-3BFF   Flash
   78     3C00-3C7F   Flash (bootloader)
   ...
   7F     3FC0-3FFF   Flash (bootloader)
   80     4000-407F   EEPROM
   ...
   83     4180-41FF   EEPROM

AVR Fuses
=========

To use this bootloader with the OpenServo version 2.1 hardware, the 
following MCU fuses must be set:

    Boot Flash section size set to 512 words
    Boot Reset vector enabled
    Brown-out detection level at VCC=4.3 V
    Internal RC Oscillator 8MHz; Start-up time 6CK/14CK + 65 ms

Bootstrap Build
===============

This project can be built as the default 'bootloader' application or as 
the special 'bootstrapper' version of the application.  The bootstrapper 
version builds the application so that it resides in lower Flash memory 
and allows the bootloader itself to be programmed in Flash.

WARNING: The risks of having an application that can write over the 
bootloader code in Flash is obvious, but this will allow the bootloader 
itself to be updated withour resorting to SPI or High Voltage Serial 
programming of the Flash.

Special Configuration Notes
===========================

This software was developed using AVR Studio version 4.13 Build 528 and 
WINAVR 20070122 GNU GCC compiler and toolchain.  Because of changes to 
these tools over time the functionality of this bootloader with other 
versions of these tools cannot be guaranteed over time.

After compilation, the map file should be checked to make sure that the 
bootloader section doesn't reside outside the upper 2048 bytes of Flash.

The following linker options must be set in the bootloader project 
configuration to have the bootloader code loaded in the upper 2048 bytes 
of Flash.

  ATmega8 Link Options
  ====================

  -nostartfiles
  -Wl,-Ttext,0x1C00

  ATmega168 Link Options
  ======================

  -nostartfiles
  -Wl,-Ttext,0x3C00


