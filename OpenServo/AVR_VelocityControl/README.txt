= OPENSERVO VELOCITY CONTROL =

This is a adaption of the OpenServo V3 code for a pure velocity control. You can command a seek velocity with the two registers
REG_SEEK_VELOCITY_HI        0x22
REG_SEEK_VELOCITY_LO        0x23

The algorithm compares the seek velocity with the actual velocity measured with the back-emf method and generates an apropriate motor PWM.

A description of the used PI+FF algorithm can be found here:
http://www.openservo.com/VelocityControl

and for further information and discussion I refer to this thread:
http://www.openservo.com/Forums/viewtopic.php?t=812


19.04.2008, Stefan Engelke <stefan@tinkerer.eu>