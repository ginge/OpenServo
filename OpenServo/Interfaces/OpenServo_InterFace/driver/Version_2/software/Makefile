LIBUSB_CONFIG   = libusb-config
CFLAGS+=-g -Wall `$(LIBUSB_CONFIG) --cflags`
LFLAGS+=`$(LIBUSB_CONFIG) --libs` -lusb

all:
	make osifupdate

clean:
	rm *.o
	rm osifupdate

osifupdate: main.c OSIFhex.o
	gcc $(CFLAGS) main.c OSIFhex.o -o osifupdate $(LFLAGS)
