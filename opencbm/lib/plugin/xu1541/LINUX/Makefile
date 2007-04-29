# $Id: Makefile,v 1.6 2007-04-29 10:46:23 strik Exp $

RELATIVEPATH=../../../
include ${RELATIVEPATH}LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

LIBNAME = libopencbm-xu1541
SRCS    = archlib.c xu1541.c s1_s2_pp.c
LIBS    = -lusb -ldl

CFLAGS += -I$(LIBUSBDIR)/include
CFLAGS += -I../../../include/LINUX/ -I../../../include/ -I../../ -I $(XU1541DIR)
LDFLAGS += -L$(LIBUSBDIR)/lib

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
