# $Id: Makefile,v 1.1.8.3 2007-10-28 11:29:01 strik Exp $

include ../../LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

ALL_CFLAGS += -I../../include

SHLIB   = libarch_fpic.a
LIB     = libarch.a
SRCS    = file.c

OBJS    = $(SRCS:.c=.o)
SHOBJS  = $(SRCS:.c=.lo)

all: $(SHLIB) $(LIB)

clean:
	rm -f $(OBJS) $(SHOBJS) $(SHLIB) $(LIB)

mrproper: clean

install-files:

install: install-files

uninstall:

.c.o:
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(SHLIB): $(SHOBJS)
	$(AR) r $@ $(SHOBJS)

$(LIB): $(OBJS)
	$(AR) r $@ $(OBJS)
