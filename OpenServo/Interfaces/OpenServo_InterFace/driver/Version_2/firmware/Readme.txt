OpenServo Interface  (OSIF) Firmware version 1.06

Copyright 2008 Barry Carter RobotFuzz www.robotfuzz.com

This is the core firmware for the OpenServo InterFace (OSIF) It allows for USB to I2C and USB GPIO control

Changelog:
1.07
Fixed issue with clocking too long.
Added slave nacking support
Fixed formatting
usbtiny 1.7 integrated

1.06
Changed the default I2C speed to 100khz and added support functions to change this value.
Added support for GPIOs using spare pin.

1.05
Added return logic to allow for a non critical error to halt the OSIF. Upgraded to USBtiny 1.3

1.04
Tidied some of the nastier code blocks into reusable portions of code. Added support for 400khz I2C.

1.03
Changed I2C module to allow for faster comms by not using delay loops.

1.02
Fixed race condition that caused lockup if I2C device did not respond correctly.

1.00
Initial release
