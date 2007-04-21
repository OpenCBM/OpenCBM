# $Id: Makefile,v 1.1 2007-04-21 09:05:48 wmsr Exp $

include ../../LINUX/config.make

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
