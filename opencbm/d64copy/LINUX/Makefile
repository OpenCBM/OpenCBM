# $Id: Makefile,v 1.1.10.1 2007-10-28 10:54:25 strik Exp $

include ../LINUX/config.make

.PHONY: all mrproper clean install uninstall install-files

OBJS = main.o
PROG = d64copy
MAN1 = d64copy.1

.c.o:
	$(CC) $(ALL_CFLAGS) -c $< -o $@

all: $(PROG)

mrproper: clean

clean:
	rm -f $(PROG) $(OBJS)

$(PROG): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LINK_FLAGS)

install-files: $(PROG)
	install -m 755 -s $(PROG) $(BINDIR)
	install -m 644 $(MAN1) $(MANDIR)

install: install-files

uninstall:
	rm -f $(BINDIR)/$(PROG) $(MANDIR)/$(MAN1)
