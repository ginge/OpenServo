AVR ATtiny25/45/85 Bootloader
=============================

February 25th, 2006

Mike Thompson
mpthompson@gmail.com

This is an TWI/I2C based bootloader for the Atmel AVR ATtiny25/45/85 series of 
microcontrollers.  The bootloader will occupy the upper 1024 bytes of Flash 
leaving the lower portion of Flash available for application code.

At boot time, the bootloader appears as an fairly standard TWI/I2C memory device 
at address 127 for about three seconds before it invokes the application code. 
If the bootloader receives a TWI/I2C read or write packet within the three 
seconds of starting the bootloader stay active for programming.  To an outside 
programmer, both the Flash and EEPROM are mapped as one contiguous address range 
divided into pages which can be read or written to via the TWI/I2C bus.  The 
bootloader can be exited and the application started by reading or writing the 
magic address of 0xFFFF.

The bootloader will protect the reset vector in the first two bytes of Flash 
so that the bootloader is always invoked at powerup or reset.  Unfortunately the
AVR ATtiny25/45/85 microcontrollers don't have on-chip support for a bootloader, 
but this seems to work fairly well.

To invoke the application, the bootloader will jump to instruction address 0x001E, 
the first byte immediately after the exception vectors.  This value should be
OK for most AVR Studio developed C applications, but it can be modified by making
the appropriate change to the 'bootstart.s' file.

The address map of Flash and EEPROM pages as presented by the bootloader for 
each supported microcontroller is shown below.  The Flash pages devoted to the
bootloader will appear as blank pages containing 0xFF values and cannot be
written.

  ATtiny25 Flash/EEPROM Map
  =========================

  Page size = 32 bytes

  Page     Address    Type
  ---------------------------------------------
   00     0000-001F   Flash
   ...
   1F     03E0-03FF   Flash
   20     0400-041F   Flash (bootloader)
   ...
   3F     07E0-07FF   Flash (bootloader)
   40     0800-081F   EEPROM 
   ...
   43     0860-087F   EEPROM


  ATtiny45 Flash/EEPROM Map
  =========================

  Page size = 64 bytes

  Page     Address    Type
  ---------------------------------------------
   00     0000-003F   Flash
   ...
   2F     0BC0-0BFF   Flash
   30     0C00-0C3F   Flash (bootloader)
   ...
   3F     0FC0-0FFF   Flash (bootloader)
   40     1000-103F   EEPROM 
   ...
   43     10C0-10FF   EEPROM


  ATtiny85 Flash/EEPROM Map
  =========================

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

To build the bootstrapper application, choose bootstrapper as the active
configuration from the project options [Note: a bug in AVR Studio prevents
the boostrapper configuration from building properly at this time.  The
bootstrapper can be build at a separate AVR Studio project.]


Special Configuration Notes
===========================

NOTE: The code was tested on the AVR ATtiny45, but is intended for the 
ATtiny25/45/85 series of microcontrollers.  Currently, some source changes may 
be required for the bootloader to work on the ATtiny25 or ATtiny85.

This software was developed using AVR Studio version 4.12 and WINAVR 
20060125 GNU GCC compiler and toolchain.  It is written as a combination of 
C code and assembly code.

After compilation, the map file should be checked to make sure that the 
bootloader section doesn't reside outside the upper 1024 bytes of Flash.

To fit within the upper 1024 bytes of Flash, the optimization must be made 
for size by setting -Os option within the bootloader project configuration.

Add the following linker options must be set in the bootloader project 
configuration to have the bootloader code loaded in the upper 1K of Flash.

  ATtiny25 Link Options
  =====================

  -nostartfiles
  -Wl,-section-start=.bootloader=0x0400

  ATtiny45 Link Options
  =====================

  -nostartfiles
  -Wl,-section-start=.bootloader=0x0C00

  ATtiny85 Link Options
  =====================

  -nostartfiles
  -Wl,-section-start=.bootloader=0x1C00

The order in which the source files are added to the bootloader project are
imporant.  They must be added to the project in the following order.

  bootcrt.s
  boostart.s
  bootloader.c
  prog.c
  twi.c
  main.c


