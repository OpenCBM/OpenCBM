RELATIVEPATH=../../../
include ${RELATIVEPATH}LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

PLUGIN_NAME = xuac
LIBNAME = libopencbm-${PLUGIN_NAME}
SRCS    = archlib.c xuac.c tape.c
LIBS    = -L$(RELATIVEPATH)/libmisc -lmisc
LIBS   += $(LIBUSB_LIBS)

CFLAGS += $(LIBUSB_CFLAGS)
CFLAGS += -I$(RELATIVEPATH)/include/LINUX/ -I$(RELATIVEPATH)/include/ -I../../ -I$(RELATIVEPATH)/libmisc
#LDFLAGS =

all: build-lib

clean: clean-lib

mrproper: clean

install-files: install-plugin

install: install-files

uninstall: uninstall-plugin

include ../../../LINUX/librules.make

### dependencies:

archlib.o archlib.lo: ../../archlib.h xuac.h
xuac.o xuac.lo: xuac.h
