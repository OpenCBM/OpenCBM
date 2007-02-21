# ======================================================================
# Common Makefile for USBtiny applications
#
# Macros to be defined before including this file:
#
# USBTINY	- the location of this directory
# TARGET_ARCH	- gcc -mmcu= option with AVR device type
# OBJECTS	- the objects in addition to the USBtiny objects
# FLASH		- command to upload firmware-usbtiny.hex to flash
# STACK		- maximum stack size
# SCHEM		- Postscript version of the schematic to be generated
#
# Copyright (C) 2006 Dick Streefland
#
# This is free software, licensed under the terms of the GNU General
# Public License as published by the Free Software Foundation.
# ======================================================================

CC	= avr-gcc
CFLAGS	= -Os -g -Wall -I. -I$(USBTINY)
ASFLAGS	= -Os -g -Wall -I.
LDFLAGS	= -g
MODULES = crc.o int.o usb.o $(OBJECTS)
UTIL	= $(USBTINY)/..

firmware-usbtiny.hex:

all:		firmware-usbtiny.hex $(SCHEM)

clean:
	rm -f firmware-usbtiny.bin *.o tags *.sch~ *~

clobber:	clean
	rm -f firmware-usbtiny.hex $(SCHEM)

firmware-usbtiny.bin:	$(MODULES)
	$(LINK.o) -o $@ $(MODULES)

firmware-usbtiny.hex:	firmware-usbtiny.bin $(UTIL)/check.py
	@$(UTIL)/check.py firmware-usbtiny.bin $(STACK) $(FLASHSIZE) $(RAMSIZE)
	avr-objcopy -j .text -j .data -O ihex firmware-usbtiny.bin firmware-usbtiny.hex

disasm:		firmware-usbtiny.bin
	avr-objdump -S firmware-usbtiny.bin

program:		firmware-usbtiny.hex
	$(FLASH)

crc.o:		$(USBTINY)/crc.S $(USBTINY)/def.h usbtiny.h
	$(COMPILE.c) $(USBTINY)/crc.S
int.o:		$(USBTINY)/int.S $(USBTINY)/def.h usbtiny.h
	$(COMPILE.c) $(USBTINY)/int.S
usb.o:		$(USBTINY)/usb.c $(USBTINY)/def.h $(USBTINY)/usb.h usbtiny.h
	$(COMPILE.c) $(USBTINY)/usb.c

firmware-usbtiny.o:		$(USBTINY)/usb.h
