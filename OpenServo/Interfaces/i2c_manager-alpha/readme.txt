Important note: This is a Work In Progress and an Alpha Test Release for early preview purposes: some major changes may occur before it goes to Beta release.

Current tests are being run under Windows and Linux (Ubuntu) on V3 OpenServo hardware using the “bank switched” firmware (OpenServo_V3-dev). 2.1 OpenServo hardware and other firmware versions will be more full tested soon.

This directory contains:

i) An alpha test release of the "I2C Manager" and a project that uses it to create an OSIF DLL that works with the "Dimax/Diolan U2C12 USB-I2C/SPI/GPIO" rather than Barry Carter's OpenServo Interface hardware- although it is hopped an "I2C Manager layer" will be developed for that too, allowing both to be used at the same time from the same application.

ii) An alpha test release of an "OpenServo (software) Interface" (OSI) that use the "I2C Manager" to access OpenServo devices on the I2C bus. Included is a simple test application that has been successfully run on both Windows and Lunux (Ubuntu).
 
How to use this software with Barry Carter's "OpenServo Interface Test" and "Motion Profiler" application on Windows. Briefly:

Install the Dimax/Diolan U2C12 USB-I2C/SPI/GPIO drivers. Then copy the i2cm_diolan.dll and osif.dll files found in this directory tree to the location that the OSID.DLL would normally be found (for example, where the applications are installed).

Currently this version of the OSIF DLL is being tested under Windows. Whilst it has been running under Linux, the latest version has not yet been tested there and builds are not provided.

10 December 2007
