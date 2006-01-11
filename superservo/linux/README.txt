SUPER SERVO LINUX I2C CLIENT DRIVER

This I2C client driver provides a /proc interface to the super servo provided that
the servo is on a linux supported I2C adapter and the adapter's driver is loaded.
The /proc interface for the servos is in /proc/sys/bus/servos. Each servo will show
up as a directory in the format b##s## where ## is the adapter bus number and the
servo address number, respectively. In each of the servo directories, there will
be proc files for reading or writing the position, speed and PID coefficients, as 
well as reading the servo device info (name, copyright, version, etc) and enabling
the PWM output.

Colin