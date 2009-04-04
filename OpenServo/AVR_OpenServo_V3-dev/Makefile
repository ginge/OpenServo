###############################################################################
# Makefile for the project ATmega168_OpenServo
###############################################################################

## General Flags
PROJECT = ATmega168_OpenServo
MCU = atmega168
TARGET = ATmega168_OpenServo
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -Os -fsigned-char -g
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

# path to your VMLAB project directory in WINE
VMLABDIR = ~/.wine/drive_c/avr/
# location path of avr-objcopy which supports the coff-avr option
OBJCOPYCOFF=~/Downloads/opt/cdk4avr/bin/avr-objcopy


## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -v  -Wl,-Map=ATmega168_OpenServo.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom


## Objects that must be built in order to link
OBJECTS = main.o adc.o registers.o eeprom.o watchdog.o motion.o pid.o power.o twi.o pwm.o seek.o timer.o curve.o pulsectl.o backemf.o banks.o alert.o step.o pwm-std.o filter.o heartbeat.o

## Objects explicitly added by the user
LINKONLYOBJECTS =

## Build
all: $(TARGET).elf ATmega168_OpenServo.hex ATmega168_OpenServo.eep ATmega168_OpenServo.lss size

## Compile
main.o: main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

adc.o: adc.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

registers.o: registers.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

timer.o: timer.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

eeprom.o: eeprom.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

watchdog.o: watchdog.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

motion.o: motion.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

filter.o: filter.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

pid.o: pid.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

power.o: power.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

pulsectl.o: pulsectl.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

twi.o: twi.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

pwm.o: pwm.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

pwm-std.o: pwm-std.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

curve.o: curve.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $< 

seek.o: seek.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

backemf.o: backemf.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

banks.o: banks.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

alert.o: alert.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

step.o: step.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

heartbeat.o: heartbeat.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

##Link
$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET).elf

%.hex: $(TARGET).elf
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET).elf
	avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@

%.lss: $(TARGET).elf
	avr-objdump -h -S $< > $@

size: ${TARGET}.elf
	@echo
	@avr-size ${TARGET}.elf

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) ATmega168_OpenServo.elf dep/* ATmega168_OpenServo.hex ATmega168_OpenServo.eep ATmega168_OpenServo.lss ATmega168_OpenServo.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

COFFCONVERT=$(OBJCOPYCOFF) --debugging \
--change-section-address .data-0x800000 \
--change-section-address .bss-0x800000 \
--change-section-address .noinit-0x800000 \
--change-section-address .eeprom-0x810000

coff: $(TARGET).elf
	$(COFFCONVERT) -O coff-avr $< $(TARGET).cof
#	cp -f *.c $(VMLABDIR)
#	cp -f $(TARGET).hex $(VMLABDIR)
#	cp -f $(TARGET).cof $(VMLABDIR)




