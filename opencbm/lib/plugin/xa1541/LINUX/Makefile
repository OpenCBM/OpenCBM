# $Id: Makefile,v 1.3 2007-04-28 17:18:33 cnvogelg Exp $

include ../../../LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

LIBNAME = libopencbm-xa1541
SRCS    = LINUX/iec.c LINUX/parburst.c

CFLAGS += -I../../../include/LINUX/ -I../../../include/ -I../../

all: build-lib

clean: clean-lib

mrproper: clean

install-files: install-plugin

install: install-files

uninstall: uninstall-plugin

include ../../../LINUX/librules.make

### dependencies:

LINUX/iec.o LINUX/iec.lo: LINUX/iec.c ../../archlib.h
LINUX/parburst.o LINUX/parburst.lo: LINUX/parburst.c ../../archlib.h
