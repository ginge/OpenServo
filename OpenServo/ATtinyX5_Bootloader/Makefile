###############################################################################
# Makefile for the project ATtiny84_Bootloader
###############################################################################

## General Flags
PROJECT = ATtin84_Bootloader
MCU = attiny84
TARGET = ATtiny84_Bootloader.elf
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -Os -fsigned-char
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -nostartfiles -Wl,-section-start=.bootloader=0x1C00 -Wl,-Map=ATtiny84_Bootloader.map -Tavr4.x

## Objects that must be built in order to link
OBJECTS = bootcrt.o bootstart.o bootloader.o main.o prog.o twi.o

## Objects explicitly added by the user
LINKONLYOBJECTS =

## Build
all: $(TARGET) ATtiny84_Bootloader.hex ATtiny84_Bootloader.lss size

## Compile
bootcrt.o: bootcrt.s
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

bootstart.o: bootstart.s
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

bootloader.o: bootloader.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

main.o: main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

prog.o: prog.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

twi.o: twi.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) ATtiny84_Bootloader.elf dep/* ATtiny84_Bootloader.hex ATtiny84_Bootloader.lss ATtiny84_Bootloader.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

