OpenServo test program. Copyright barry Carter 2007
<Barry.Carter@headfuzz.co.uk>

This is a program to allow testing of the OpenServo boards you have plugged into
the bus.

1) Depends:

This program depends on OSIFdll.dll in Windows (win32) or libOSIFlib.so.1.x.y on
BSD/Mac/Linux.
This package is available on the OpenServo CVS server under the drivers section
in the OSIF directory.

This program is designed in QT. QT is a GPL widget and cross platform tool. As
such, this program requires the core qt dll's if compiled in shared mode, or the
precompiled QT libraries in static mode.


2) Usage:

The application is mostly self explanitory. In any input box you can use 8 bit
hex values prepended by 0x, or integers.

Once initialised the application will attempt to scan the USB bus for the OSIF
device. You should see in the log window if the initialisation was successful.
If the device was initialised successfully you should scan the I2C busses for
devices. You will not be allowed to send data unless you have selected a valid
I2C device.

Generic Communications to any device on the USB bus is possible. The boxes in
the generic comms section allow you to send arbitary length commands to the
selected USB device. The data input box allow you to send commands to the device
with space delimited intereger or hex values. Invalid values in this box will
either cause an excception or more likely invalid data to be sent to the device.
Make sure to set the correct length for the data, or no data will be written.

3) Bugs:

Quite a few. This is version 0.2, so give me a break. If you dont have the DLL
in the path dont expect anything at all to work.

It is possible for the PWM on/off value to go astray if you reboot the servo.
Currently this value is not checked on the servo. Maybe v0.3

The PWM labels currently show a 16bit variable. This is incorrect as each PWM
channel should be show seperately.

There is minimal type checking on the input fields, and overflows are entirely
posible. Try not to write data ofer 255 bytes ;). Most bad data will be
completely ignored, or just sent to the I2C device verbatim.

If you write completely bogus data to the OSIF, the underlying shared library
could break. These problems will be documented with the OSIF library.

Improvements welcome.