AVR ATmegaX Bootloader
======================

March 5th, 2006

Mike Thompson
mpthompson@gmail.com

This is an TWI/I2C based bootloader for the Atmel AVR ATmega MCUs.  The
bootloader will occupy the upper 1024 bytes of Flash leaving the lower portion
of Flash available for application code.

At boot time, the bootloader appears as an fairly standard TWI/I2C memory device
at address 127 for about three seconds before it invokes the application code.
If the bootloader receives a TWI/I2C read or write packet within the three
seconds of starting the bootloader stay active for programming.  To an outside
programmer, both the Flash and EEPROM are mapped as one contiguous address range
divided into pages which can be read or written to via the TWI/I2C bus.  The
bootloader can be exited and the application started by reading or writing the
magic address of 0xFFFF.

The bootloader will protect the reset vector in the first two bytes of Flash
so that the bootloader is always invoked at powerup or reset.  In this way the
bootloader is always active even if the Bootloader Reset Vector is unprogrammed.

To invoke the application, the bootloader will jump to the instruction address
just after the standard interrupt vectors.  This value should be OK for most
AVR Studio developed C applications, but it can be modified by making the
appropriate change to the 'bootstart.s' file.

The address map of Flash and EEPROM pages as presented by the bootloader for
each supported microcontroller is shown below.  The Flash pages devoted to the
bootloader will appear as blank pages containing 0xFF values and cannot be
written.

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
   6F     3B80-1BFF   Flash
   70     3C00-3C7F   Flash (bootloader)
   ...
   7F     3FC0-3FFF   Flash (bootloader)
   80     4000-407F   EEPROM
   ...
   83     4180-41FF   EEPROM

Bootstrap Build
===============

This project can be built as the default 'bootloader' application or as the
special 'bootstrapper' version of the application.  The bootstrapper version
builds the application so that it resides in lower Flash memory and allows
the bootloader itself to be programmed in Flash.

WARNING: The risks of having an application that can write over the
bootloader code in Flash is obvious, but this will allow the bootloader
itself to be updated withour resorting to SPI or High Voltage Serial
programming of the Flash.

Special Configuration Notes
===========================

This software was developed using AVR Studio version 4.12 and WINAVR
20060125 GNU GCC compiler and toolchain.  It is written as a combination of
C code and assembly code.

After compilation, the map file should be checked to make sure that the
bootloader section doesn't reside outside the upper 1024 bytes of Flash.

To fit within the upper 1024 bytes of Flash, the optimization must be made
for size by setting -Os option within the bootloader project configuration.

Add the following linker options must be set in the bootloader project
configuration to have the bootloader code loaded in the upper 1K of Flash.

  ATmega8 Link Options
  ====================

  -nostartfiles
  -Wl,-section-start=.bootloader=0x1800

  ATmega168 Link Options
  ======================

  -nostartfiles
  -Wl,-section-start=.bootloader=0x3800

The order in which the source files are added to the bootloader project are
imporant.  They must be added to the project in the following order.

  bootcrt.s
  boostart.s
  bootloader.c
  prog.c
  twi.c
  main.c




