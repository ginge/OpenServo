###############################################################################
# Makefile for the project ATmega168_Bootloader
###############################################################################

## General Flags
PROJECT = ATmega168_Bootloader
MCU = atmega168
TARGET = ATmega168_Bootloader.elf
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
LDFLAGS += -nostartfiles -Wl,-section-start=.bootloader=0x3C00 -Wl,-Map=ATmega168_Bootloader.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0


## Objects that must be built in order to link
OBJECTS = bootcrt.o bootstart.o bootloader.o main.o prog.o twi.o

## Objects explicitly added by the user
LINKONLYOBJECTS =

## Build
all: $(TARGET) ATmega168_Bootloader.hex ATmega168_Bootloader.eep ATmega168_Bootloader.lss size

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
	-rm -rf $(OBJECTS) ATmega168_Bootloader.elf dep/* ATmega168_Bootloader.hex ATmega168_Bootloader.eep ATmega168_Bootloader.lss ATmega168_Bootloader.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

