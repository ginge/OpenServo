AVR OpenServo
=============

March 8th, 2006

Mike Thompson
mpthompson@gmail.com

The OpenServo software was developed using AVR Studio version 4.12 and 
WINAVR 20060125 GNU GCC compiler and toolchain.  It is written as a 
combination of C code and assembly code.  The following AVR MCUs are 
currently supported:

    ATtiny45
    ATtiny85 (Untested)
    ATmega8
    ATmega168

The servo code is meant to function with the OpenServo Bootloader for easy 
updating of the application on the servo.  With the bootloader installed, 
the compiled OpenServo application code should not exceed the following 
sizes:

    MCU             Size
    -----------------------------
    ATtiny45        3072 bytes
    ATtiny85        7168 bytes
    ATmega8         7168 bytes
    ATmega168       15360 bytes

After compilation, the map file be checked to make sure that the OpenServo
code does not reside outside application code space.

Certain platforms may require optimizing compilation for size to fit the
OpenServo code within the application code space.

Add the following linker options must be set in the project configuration 
to be compatible with the OpenServo Bootloader.

    ATtiny45/85 Link Options
    ========================

    -nostartfiles

    ATmega8 Link Options
    ====================

    -nostartfiles

    ATmega168 Link Options
    ======================

    -nostartfiles

