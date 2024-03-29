###############################################################################
# Makefile for the project smartAlarm
###############################################################################

## General Flags
PROJECT = smartAlarm
MCU = atmega8
TARGET = smartAlarm.elf
CC = avr-gcc.exe

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99     -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=smartAlarm.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Include Directories
INCLUDES = -I"C:\Documents and Settings\Administrator\My Documents\Projects\smartAlarm\firmware\gcc\avrlib" -I"C:\Documents and Settings\Administrator\My Documents\Projects\smartAlarm\firmware\gcc\." 

## Objects that must be built in order to link
OBJECTS = main.o rprintf.o timer.o uart.o buffer.o cmdline.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) smartAlarm.hex smartAlarm.eep smartAlarm.lss size

## Compile
main.o: ../main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

rprintf.o: ../avrlib/rprintf.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

timer.o: ../avrlib/timer.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

uart.o: ../avrlib/uart.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

buffer.o: ../avrlib/buffer.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

cmdline.o: ../avrlib/cmdline.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) smartAlarm.elf dep/* smartAlarm.hex smartAlarm.eep smartAlarm.lss smartAlarm.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

