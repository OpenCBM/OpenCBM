RELATIVEPATH=../
include ${RELATIVEPATH}LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

#CFLAGS += -I../include

LIBNAME = libopencbmtransfer
SRCS    = \
	  o65.c \
	  pp.c \
	  s1.c \
	  s2.c \
	  s3.c \
	  turbo.c

INCS    = pp-1541.inc \
	  pp-1571.inc \
	  s1-1541-1571.inc \
	  s1-1581.inc \
	  s1-fdx000.inc \
	  s2-1541-1571.inc \
	  s2-1581.inc \
	  s2-fdx000.inc \
	  s3-1571.inc \
	  s3-1581.inc \
	  s3-fdx000.inc \
	  turbomain-1541-1571.inc \
	  turbomain-1581.inc \
	  turbomain-fdx000.inc \


all: build-lib

clean: clean-lib

mrproper: clean

install-files: install-lib

install: install-files update-libcache

uninstall: uninstall-lib

include ../LINUX/librules.make

### dependencies:

pp-1541.inc: pp-1541.a65 common.i65
pp-1571.inc: pp-1571.a65 common.i65

s1.o s1.lo: s1.c s1-1541-1571.inc        s1-1581.inc s1-fdx000.inc
s2.o s2.lo: s2.c s2-1541-1571.inc        s2-1581.inc s2-fdx000.inc
s3.o s3.lo: s3.c             s3-1571.inc s3-1581.inc s3-fdx000.inc
pp.o pp.lo: pp.c pp-1541.inc pp-1571.inc
turbo.o turbo.lo: turbomain-1541-1571.inc turbomain-1581.inc turbomain-fdx000.inc
