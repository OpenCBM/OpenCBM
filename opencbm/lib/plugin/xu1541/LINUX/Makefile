# $Id: Makefile,v 1.8 2009-12-12 18:27:04 cnvogelg Exp $

RELATIVEPATH=../../../
include ${RELATIVEPATH}LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

PLUGIN_NAME = xu1541
LIBNAME = libopencbm-${PLUGIN_NAME}
SRCS    = archlib.c xu1541.c s1_s2_pp.c
LIBS    = -ldl

CFLAGS += $(LIBUSB_CFLAGS)
CFLAGS += -I../../../include/LINUX/ -I../../../include/ -I../../ -I $(XU1541DIR)/include
LDFLAGS += $(LIBUSB_LDFLAGS)

all: build-lib

clean: clean-lib

mrproper: clean

install-files: install-plugin

install: install-files

uninstall: uninstall-plugin

include ../../../LINUX/librules.make

### dependencies:

archlib.o archlib.lo: ../../archlib.h xu1541.h
xu1541.o xu1541.lo: xu1541.h
