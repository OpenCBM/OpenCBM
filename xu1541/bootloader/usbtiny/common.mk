# ======================================================================
# Common Makefile for USBtiny applications
#
# Macros to be defined before including this file:
#
# USBTINY	- the location of this directory
# TARGET_ARCH	- gcc -mmcu= option with AVR device type
# OBJECTS	- the objects in addition to the USBtiny objects
# FLASH		- command to upload main.hex to flash
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
LDFLAGS	+= -g 
MODULES = crc.o int.o usb.o $(OBJECTS)
UTIL	= $(USBTINY)/..

bootldr-usbtiny.hex:

all:		bootldr-usbtiny.hex $(SCHEM)

clean:
	rm -f bootldr-usbtiny.bin *.o tags *.sch~ *~

clobber:	clean
	rm -f bootldr-usbtiny.hex $(SCHEM)

bootldr-usbtiny.bin:	$(MODULES)
	$(LINK.o) -o $@ $(MODULES)

bootldr-usbtiny.hex:	bootldr-usbtiny.bin $(UTIL)/check.py
	@$(UTIL)/check.py bootldr-usbtiny.bin $(STACK) $(FLASHSIZE) $(RAMSIZE)
	avr-objcopy -j .text -j .data -O ihex bootldr-usbtiny.bin bootldr-usbtiny.hex

disasm:		bootldr-usbtiny.bin
	avr-objdump -S bootldr-usbtiny.bin

program:		bootldr-usbtiny.hex
	$(FLASH)

crc.o:		$(USBTINY)/crc.S $(USBTINY)/def.h usbtiny.h
	$(COMPILE.c) $(USBTINY)/crc.S
int.o:		$(USBTINY)/int.S $(USBTINY)/def.h usbtiny.h
	$(COMPILE.c) $(USBTINY)/int.S
usb.o:		$(USBTINY)/usb.c $(USBTINY)/def.h $(USBTINY)/usb.h usbtiny.h
	$(COMPILE.c) $(USBTINY)/usb.c

bootldr-usbtiny.o:		$(USBTINY)/usb.h
