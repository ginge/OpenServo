AVR ATtinyX5 OpenServo
======================

January 7th, 2005

Mike Thompson
mpthompson@gmail.com

NOTE: The code was tested on the AVR ATtiny45, but is intended for the 
ATtiny45 and ATtiny85 microcontrollers.  Currently, some source changes may 
be required for the servo to function on the ATtiny85.

This software was developed using AVR Studio version 4.12 and WINAVR 
20050214 GNU GCC compiler and toolchain.  It is written as a combination of 
C code and assembly code.  WINAVR doesn't yet completely support C 
development for the AVR ATtiny25/45/85 series microcontrollers and the 
following considerations must be made when compiling the bootloader.

The header files from avr-libc (AVR C Runtime Library) version 1.2.6 must 
be used which includes the iotnx5.h, iotn25.h, iotn45.h and iotn85.h header 
files for the ATtiny25/45/85 microcontrollers.  Editing changes are needed 
in io.h to include these files. 

Within AVR Studio, the project configuration options should have the target 
processor specified to be the 'attiny2313'.  The instruction set of this 
microcontroller is compatible with the ATtiny25/45/85.

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
  motion.c
  power.c
  pwm.c
  registers.c
  twi.c
  watchdog.c


TODO

Switch to ATtiny85 to gain more Flash space.
Add tables of default gain values for different servo hardware.
Add a speed parameter by which speed of the servo can be controlled.
