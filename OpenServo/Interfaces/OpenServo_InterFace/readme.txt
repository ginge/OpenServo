	OpenServo InterFace (OSIF)  - Based on USB-tiny-I2C by Till Harbaum www.harbaum.org
                 All source code and schematics are Copyright Barry Carter 2007
                       Unless otherwise stated. Barry.Carter@gmail.com


Presented is a simple USB interface to connect to the OpenServo servo driver board. The board consists of an Atmel Mega8 as it's 
primary driver chip. The descriptions of the software are in the driver directory, and the recommended reading on implementation 
is in there. 

There are two boards in CVS, the first is a full implementation on a large board, the second a more compact version.

=== Full version ===

The board has a standard I2C connector, as well as an 8 pin OpenServo connector. The OpenServo connector also allows for 
flashing of the onboard Atmel Mega8 in addition to the ISP-10 connector

There is an external power source input to allow for running the OpenServo from a compatible power supply. If you wish to use
the external supply, you must remove the jumper JP1, and under NO circumstances connect this while it is externally powered.
To do so would damage the USB-mega-I2C interface with the possibility of damaging the USB port.

Any spare Mega8 pins have been broken out onto 2x8 GPIO pins. These pins are not currently controllable, and may be implemented
in the future.


=== Compact version ===

This board has an 8 pin OpenServo connector onboard. This connector allow for easy control of the servo. For programming 
construct the mirror PCB in CVS or construct a cable.
