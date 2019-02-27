RELATIVEPATH=../opencbm/
include ${RELATIVEPATH}LINUX/config.make

CFLAGS+= -g $(LIBUSB_CFLAGS) -I $(DFU_SRC) -I $(LIBMISC) -I ../opencbm/include/ -Wall
LINK_FLAGS= $(LIBUSB_LDFLAGS) $(LIBUSB_LIBS)

DFU_SRC= dfu-programmer-0.5.4/src
LIBMISC= ../opencbm/libmisc/

OBJS= main.o util.o
OBJS+= $(DFU_SRC)/arguments.o $(DFU_SRC)/atmel.o $(DFU_SRC)/commands.o $(DFU_SRC)/dfu.o $(DFU_SRC)/intel_hex.o $(DFU_SRC)/util.o

PROG = xum1541cfg
LINKS = 

include ${RELATIVEPATH}LINUX/prgrules.make
