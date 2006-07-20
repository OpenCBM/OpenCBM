# $Id: Makefile,v 1.3 2006-07-20 11:45:28 strik Exp $

include ../../LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

CFLAGS += -I../../include

LIB     = libarch.a
SRCS    = ctrlbreak.c \
	  file.c

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
