# $Id: Makefile,v 1.4 2007-04-29 10:46:23 strik Exp $

RELATIVEPATH=../../../
include ${RELATIVEPATH}LINUX/config.make

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
