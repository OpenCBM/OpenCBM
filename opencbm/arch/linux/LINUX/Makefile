# $Id: Makefile,v 1.1 2005-03-02 18:17:17 strik Exp $

include ../../LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

CFLAGS += -I../../include

LIB     = libarch.a
SRCS    = file.c

OBJS    = $(SRCS:.c=.o)

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
