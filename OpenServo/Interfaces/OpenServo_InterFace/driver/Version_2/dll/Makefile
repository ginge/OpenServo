CC=gcc
CFLAGS=-I.
DEPS = OSIFdll.h OSIFflash.h OSIFlib.h
OBJ = OSIFdll.o OSIFflash.o OSIFhex.o OSIFio.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -fPIC

libOSIFlib: $(OBJ)
	gcc -shared -Wl,-soname,libOSIFlib.so.1 -o libOSIFlib.so.1.0.1 $^ $(CFLAGS) -lusb -lm

clean:
	rm *.o
	rm libOSIFlib.so.1.0.1
