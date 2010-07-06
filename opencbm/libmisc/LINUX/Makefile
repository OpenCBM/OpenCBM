# $Id: Makefile,v 1.5 2010-07-06 18:18:59 cnvogelg Exp $

RELATIVEPATH=../
include ${RELATIVEPATH}LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

CFLAGS += -I../include -I./ $(LIBUSB_CFLAGS)
LDFLAGS += $(LIBUSB_LDFLAGS)

LIB     = libmisc.a
SRCS    = libstring.c configuration.c LINUX/getpluginaddress.c LINUX/dynlibusb.c

OBJS    = $(SRCS:.c=.lo)

all: $(LIB)

clean:
	rm -f $(OBJS) $(LIB)

mrproper: clean

install-files:

install: install-files

uninstall:

.c.o:
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(LIB): $(OBJS)
	$(AR) r $@ $(OBJS)
