The files in this directory updates WinAVR 20050214 which doesn't natively
support the AVR ATtinyX5 MCUs used in the OpenServo project.  After installing
WinAVR 20050214 and AVR Studio 4.12, the these header files should be
copied into the WinAVR include directory "C:\WinAVR\avr\include\avr".

When using WinAVR 20050214 with AVR Studio 4.12, the project configuration 
options should have the target processor specified to be the 'attiny2313' if
you are writing code for the 'attiny25', 'attiny45' or the 'attiny85'.  The
instruction set of the ATtiny2313 MCU is compatible with the ATtinyX5 MCUs.

A new version of WinAVR should be available in January or February of 2006
which should natively support the AVR ATtinyX5 MCUs.

--Mike
