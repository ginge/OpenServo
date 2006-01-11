# something is wrong with the CVS version of the makefile
# - checkouts are trying to make it as a directory or soemthing???
# anyway - hopefully this dummy check-in will fix it.
CC=gcc -I/usr/src/kernel-source-2.4.18/include
CFLAGS=-O2 -Wall

# name of the module we are building
MODNAME=servo

# target host name for cross compiling
TARGET=drake

# building to remote target device
INSMOD=ssh $(TARGET) /sbin/insmod /home/cmackenzie/servo/$(MODNAME).o
RMMOD=ssh $(TARGET) /sbin/rmmod $(MODNAME)

# building locally on target device
# INSMOD=insmod $(MODNAME).o
# RMMOD=rmmod $(MODNAME)

all: $(MODNAME).o 
	cp $(MODNAME).h /usr/include/linux

clean: 
	rm -f *.o *~

$(MODNAME).o: $(MODNAME).h

load: all
	$(INSMOD)

unload: all
	$(RMMOD)
