AVR ATtinyX5 OpenServo
======================

February 25th, 2006

Mike Thompson
mpthompson@gmail.com

NOTE: The code was tested on the AVR ATtiny45, but is intended for the 
ATtiny45 and ATtiny85 microcontrollers.  Currently, some source changes may 
be required for the servo to function on the ATtiny85.

This software was developed using AVR Studio version 4.12 and WINAVR 
20060125 GNU GCC compiler and toolchain.  It is written as a combination of 
C code and assembly code.

The servo code is meant to function with the ATtiny4X_Bootloader for easy 
updating of the application on the servo.  On the ATtiny45 the servo code 
should not exceed 3072 bytes of Flash. After compilation, the map file be 
checked to make sure that the bootloader section doesn't reside outside the 
lower 3072 bytes of Flash.

To fit within the lower 3072 bytes of Flash, the optimization must be made 
for size by setting -Os option within the bootloader project configuration.

Add the following linker options must be set in the project configuration 
to be compatible with the ATtinyX5_Bootloader.

    ATtiny45/85 Link Options
    ========================

    -nostartfiles

The order in which the source files are added to this project are imporant. 
They must be added to the project in the following order.

    bootcrt.s
    main.c
    adc.c
    eeprom.c
    estimator.c
    ipd.c
    math.c
    motion.c
    pid.c
    power.c
    pwm.c
    registers.c
    regulator.c
    timer.c
    twi.c
    watchdog.c

TODO

Switch to ATtiny85 to gain more Flash space.
Add tables of default gain values for different servo hardware.
Add a speed parameter by which speed of the servo can be controlled.
