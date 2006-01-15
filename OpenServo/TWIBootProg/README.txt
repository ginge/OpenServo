TWIBootProg
===========

January 14th, 2006

Mike Thompson
mpthompson@gmail.com

This is the TWI/I2C based programmer created originally for the 
OpenServo project.  It utilizes the Dimax U2C-12 USB/I2C interface
to communicate with the bootloader to program applications into 
the AVR ATtiny25/45/85 MCU Flash and EEPROM.

Programming must occur while the bootloader on the AVR MCU is active
just after power-up or reset.

This application is derived directly from the Dimax ControlPanel
application and carries license terms of the GNU General Public
License as published by the Free Software Foundation.

TODO
====

Have this application automatically place the I2C slave into the 
bootloader so that programming can occur without manually cycling 
the power on the AVR MCU being programmed.
