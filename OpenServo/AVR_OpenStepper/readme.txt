OpenServo Stepper motor control module

This is the source for the OpenServo stepper motor output control
This source tree is currently outside of the mainline as it is being structured

This is a work in progress and has not been tested on real hardware.

Code copyright 2008 Jay Ragsdale and Barry Carter

Description: (step.c)

This allow for a standard 4 pole stepper motor to be driven from an 
AVR attiny84. This is the only supported target hardware at the moment,
but further platforms will be made available.

Output is driven from the PID algorithm which is fed into the CTC timer. When
the timer overflows the interrupt routine uses a lookup table to output the
correct pattern for the step sequence.

Description: (hall.c)

TBD