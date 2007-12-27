::: Java Wrapper for the Dimax-U2C-Library
::::::::::::::::::::::::::::::::::::::::::::

This little peace of software provides access to Dimax I2C-USB Adapters 
through the Java Native Interface. 

It consists of two parts:

- A C++ library wich wrappes the U2C-library into a JNI compatible form.

- A small Java framework wich respresents the transaction logic and 
  provides the JNI-access.

The U2C drivers has to be installed and the U2CWrapper library has to be 
either in the root-classpath of your application or in the "lib" path of 
the virtual machine else a UnsatisfiedLinkError-Exception is thrown.

NOTE: At the moment there is only the Device-Handling and the 
Highlevel-I2C-Communication implemented!

This repository has the following directories:

- c++src/	Contains the c++ source code of the original Dimax library and 
            the wrapper library.

- src/		Contains the java source code of the i2c framework and a small 
            test application to show the usage.

- classes/ 	Contains a compiled version of the java framework, the test 
            application and the wrapper library. You can start the test 
            application with "java Test"

- doc/		Documentation of the java framework generated with JavaDoc.

I hope the U2CWrapper will be useful for all the other java enthusiasts 
with a Dimax U2C device out there.

Stefan Engelke