# $Id: Makefile,v 1.4 2007-04-21 18:16:09 cnvogelg Exp $

include ../../LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

CFLAGS += -I../../include

LIB     = libarch.a
SRCS    = ctrlbreak.c \
	  file.c

ifeq "$(OS)" "Darwin"
SRCS += error.c
endif

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
