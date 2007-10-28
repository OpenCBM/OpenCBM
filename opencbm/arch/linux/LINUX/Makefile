# $Id: Makefile,v 1.1.8.1 2007-10-28 10:17:05 strik Exp $

include ../../LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

CFLAGS += -I../../include

LIB     = libarch.a
SRCS    = file.c

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
