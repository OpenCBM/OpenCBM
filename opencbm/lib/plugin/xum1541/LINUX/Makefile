# $Id: Makefile,v 1.4 2010-02-21 09:54:06 strik Exp $

RELATIVEPATH=../../../
include ${RELATIVEPATH}LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

PLUGIN_NAME = xum1541
LIBNAME = libopencbm-${PLUGIN_NAME}
SRCS    = archlib.c xum1541.c s1_s2_pp.c parburst.c
LIBS    = -ldl

CFLAGS += $(LIBUSB_CFLAGS)
CFLAGS += -I$(RELATIVEPATH)/include/LINUX/ -I$(RELATIVEPATH)/include/ -I../../ -I $(XUM1541DIR) -I$(RELATIVEPATH)/libmisc
LDFLAGS += $(LIBUSB_LDFLAGS)

all: build-lib

clean: clean-lib

mrproper: clean

install-files: install-plugin

install: install-files

uninstall: uninstall-plugin

include ../../../LINUX/librules.make

### dependencies:

archlib.o archlib.lo: ../../archlib.h xum1541.h
xum1541.o xum1541.lo: xum1541.h
