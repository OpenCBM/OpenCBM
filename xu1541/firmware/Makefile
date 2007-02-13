# ======================================================================
# Makefile for xu1541
#
# Copyright (C) 2006 Till Harbaum
#
# This is free software, licensed under the terms of the GNU General
# Public License as published by the Free Software Foundation.
# ======================================================================
USBTINY         = ./usbtiny
TARGET_ARCH     = -DF_CPU=12000000 -DAVR -DUSBTINY -mmcu=atmega8
OBJECTS         = main.o xu1541.o s1.o s2.o pp.o
CABLE           = stk200
#CABLE           = usbasp
AVRDUDE         = avrdude
#AVRDUDE         = sudo /usr/local/avr/bin/avrdude
FLASH           = $(AVRDUDE) -c $(CABLE) -p atmega8 -U lfuse:w:0x9f:m -U hfuse:w:0xc9:m -U flash:w:main.hex
STACK           = 32
FLASHSIZE       = 8192
RAMSIZE         = 1024

include $(USBTINY)/common.mk
