# $Id: Makefile,v 1.1 2005-03-02 18:17:17 strik Exp $

include ../LINUX/config.make

.PHONY: all mrproper clean install uninstall install-files

PROGS   = cbmctrl
OBJS    = cbmctrl.o
MAN1    = cbmctrl.1

all: $(PROGS)

clean:
	rm -f $(PROGS) $(OBJS)

mrproper: clean

install-files: $(PROGS)
	install -m 755 -s $(PROGS) $(BINDIR)
	install -m 644 $(MAN1) $(MANDIR)

install: install-files

uninstall:
	for f in $(PROGS); do rm -f $(BINDIR)/$$f; done
	for f in $(MAN1); do rm -f $(MANDIR)/$$f{,.gz}; done

cbmctrl: $(OBJS)
	$(CC) $(CFLAGS) $< -o $@ $(LINK_FLAGS)
