# $Id: Makefile,v 1.2 2008-09-01 18:37:16 strik Exp $

RELATIVEPATH=../
include ${RELATIVEPATH}LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

CFLAGS += -I../include

LIB     = libmisc.a
SRCS    = libstring.c configuration.c

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
