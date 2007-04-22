# $Id: Makefile,v 1.3 2007-04-22 18:46:50 wmsr Exp $

include ../../LINUX/config.make
CFLAGS     := $(subst ../,../../,$(CFLAGS))
LINK_FLAGS := $(subst ../,../../,$(LINK_FLAGS))

.PHONY: all mrproper clean install uninstall install-files

OBJS = main.o
PROG = weaktest

.c.o:
	$(CC) $(CFLAGS) -I../../libd64copy -c $< -o $@

all: $(PROG)

mrproper: clean

clean:
	rm -f $(PROG) $(OBJS)

$(PROG): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LINK_FLAGS)

install-files: $(PROG)
	install -m 755 -s $(PROG) $(BINDIR)

install: install-files

uninstall:
	rm -f $(BINDIR)/$(PROG) $(MANDIR)/$(MAN1)
