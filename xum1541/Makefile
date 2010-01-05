#
# Build the firmware for the xum1541
# Copyright (c) 2009 Nate Lawson <nate@root.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version
# 2 of the License, or (at your option) any later version.
#

# You must select a model of xum1541 here. Each model indicates the CPU
# and board type (i.e., IO port/pin layout, peripherals, etc.)
#
# Currently, valid models are:
# AT90USBKEY: Atmel USB key development board with 8 MHz AT90USB1287 cpu
MODEL=      AT90USBKEY

# Enable to get debug printing via the UART (port D)
#CFLAGS=    -DDEBUG -DDEBUG_LEVEL=DBG_INFO

### Nothing user-configurable beyond this point ###

# Firmware version. Bump when changing the firmware code.
XUMFW_VERSION= 04

ifeq ($(MODEL),AT90USBKEY)
CPUMODEL=	at90usb1287
CPURATE=	8000000
BOARD=		BOARD_USBKEY
BOARD_OBJS=	board-usbkey.o
else
$(error Must define a MODEL so we know what to build)
endif

# Final name for firmware hex file
MODELVERSION= xum1541-$(MODEL)-v$(XUMFW_VERSION)

CFLAGS+= -DMODEL=$(MODEL) -DMODELNAME=\"$(MODEL)\" -DBOARD=$(BOARD) \
	-DF_CPU=$(CPURATE)UL -DF_CLOCK=F_CPU -mmcu=$(CPUMODEL) \
	-O -g -Werror -Wall -Wstrict-prototypes -Wundef -std=gnu99 -I . \
	-DUSB_DEVICE_ONLY -DUSE_NONSTANDARD_DESCRIPTOR_NAMES \
	-DINTERRUPT_CONTROL_ENDPOINT \
        -DFIXED_CONTROL_ENDPOINT_SIZE=8 \
        -DUSE_SINGLE_DEVICE_CONFIGURATION \
	-DUSE_STATIC_OPTIONS="(USB_DEVICE_OPT_FULLSPEED | USB_OPT_REG_ENABLED | USB_OPT_AUTO_PLL)" \
        -funsigned-char -funsigned-bitfields -ffunction-sections \
        -fpack-struct -fshort-enums -finline-limit=20

MYUSB_OBJS= \
	LUFA/Drivers/USB/LowLevel/DevChapter9.o \
	LUFA/Drivers/USB/LowLevel/Endpoint.o \
	LUFA/Drivers/USB/LowLevel/LowLevel.o \
	LUFA/Drivers/USB/HighLevel/Events.o \
	LUFA/Drivers/USB/HighLevel/USBTask.o \
	LUFA/Drivers/USB/HighLevel/USBInterrupt.o

IEC_OBJS= iec.o s1.o s2.o pp.o p2.o nib.o

OBJS=     main.o commands.o descriptor.o \
          $(BOARD_OBJS) $(MYUSB_OBJS) $(IEC_OBJS)

.c.o:
	avr-gcc $(CFLAGS) -c -o $@ $< 

all: $(OBJS)
	avr-gcc $(CFLAGS) -o $(MODELVERSION).bin $(OBJS) $(LDFLAGS)
	avr-objcopy -O ihex $(MODELVERSION).bin $(MODELVERSION).hex

clean:
	rm -f $(OBJS) $(MODELVERSION).bin
