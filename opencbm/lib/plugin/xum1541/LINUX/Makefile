# $Id: Makefile,v 1.1 2009-12-09 05:39:00 natelawson Exp $

RELATIVEPATH=../../../
include ${RELATIVEPATH}LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

PLUGIN_NAME = xum1541
LIBNAME = libopencbm-${PLUGIN_NAME}
SRCS    = archlib.c xum1541.c s1_s2_pp.c
LIBS    = -lusb -ldl

CFLAGS += -I$(LIBUSBDIR)/include
CFLAGS += -I../../../include/LINUX/ -I../../../include/ -I../../ -I $(XUM1541DIR)
LDFLAGS += -L$(LIBUSBDIR)/lib

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
