#
# Compile C from source dir and header dir into dist dir and create ELF and finally HEX files
# to upload it to AVR chip.
#
# https://stackoverflow.com/a/1484873
# https://www.nongnu.org/avrdude/user-manual/avrdude_4.html
# https://www.ladyada.net/learn/avr/avrdude.html
#
# EXAMPLE:
# avr-gcc -c -mmcu=atmega16 file1.c -o file1.o
# avr-gcc -c -mmcu=atmega16 file2.c -o file2.o
# avr-gcc -c -mmcu=atmega16 file3.c -o file3.o
# avr-gcc -mmcu=atmega16 file1.o file2.o file3.o -o proj.elf
# avr-objcopy -O ihex -j .text -j .data proj.elf proj.hex

TARGET = main
CC = avr-gcc
CFLAGS = -g -Wall -I$(HEADERS_DIR)
OBJCOPY = avr-objcopy
AVRDUDE = avrdude
SRC_DIR = src
HEADERS_DIR = includes
DIST_DIR = dist

define DEFAULT_CONFIG
PROGRAMMER=arduino_mymod
MCU=atmega328p
MHZ=16
DEVICE=/dev/ttyUSB0
endef
export DEFAULT_CONFIG

getVar = $(shell sed -En 's/$1=(.*)/\1/p' configure)

.PHONY: default all clean init configure upload

default: init $(DIST_DIR)/$(TARGET).hex
all: default upload

init: $(DIST_DIR)
$(DIST_DIR):
	@test -f configure || { echo "No configure file available. Run configure first."; exit 1; }
	mkdir -p $(DIST_DIR)

OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(DIST_DIR)/%.o, $(wildcard $(SRC_DIR)/*.c))
HEADERS = $(wildcard $(HEADERS_DIR)/*.h)

$(DIST_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c -mmcu=$(call getVar,MCU) -Os -DF_CPU=$(call getVar,MHZ)000000 $< -o $@

.PRECIOUS: $(DIST_DIR)/$(TARGET).hex $(OBJECTS)

$(DIST_DIR)/$(TARGET).hex: $(DIST_DIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex -j .text -j .data $< $@

$(DIST_DIR)/$(TARGET).elf: $(OBJECTS)
	$(CC) -Wall -mmcu=$(call getVar,MCU) -Os -DF_CPU=$(call getVar,MHZ)000000 $(OBJECTS) -o $@

upload:
	@test -f $(DIST_DIR)/$(TARGET).hex || { echo "$(DIST_DIR)/$(TARGET).hex does not exist. Run make first."; exit 1; }
	$(AVRDUDE) -c $(call getVar,PROGRAMMER) -p $(call getVar,MCU) -P $(call getVar,DEVICE) -U flash:w:$(DIST_DIR)/$(TARGET).hex

clean:
	-rm -rf $(DIST_DIR)

configure:
	@echo "$$DEFAULT_CONFIG" > $@
	@echo "Configfile '$@' created with default values."

help:
	@echo "Makes code for AVR chips"
	@echo "  make:            compile from $(SRC_DIR)/ and $(HEADERS_DIR)/ and create objects, elf and hex files in $(DIST_DIR)/"
	@echo "  make upload:     upload hex file to chip"
	@echo "  make all:        make + make upload"
	@echo "  make configure:  creates a config file"
	@echo "  make clean:      delete $(DIST_DIR)/"
