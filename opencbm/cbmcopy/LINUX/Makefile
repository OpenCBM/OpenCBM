# $Id: Makefile,v 1.1.10.1 2007-10-28 10:54:24 strik Exp $

include ../LINUX/config.make

.PHONY: all mrproper clean install uninstall install-files

OBJS = main.o pc64.o t64.o raw.o
PROG = cbmcopy
MAN1 = cbmcopy.1
LINKS = cbmread cbmwrite

all: $(PROG)

mrproper: clean

.c.o:
	$(CC) -g $(ALL_CFLAGS) -o $@ -c $<

clean:
	rm -f $(PROG) $(OBJS)

$(PROG): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LINK_FLAGS)

install-files: $(PROG)
	install -m 755 -s $(PROG) $(BINDIR)
	for l in $(LINKS); do ln -sf $(PROG) $(BINDIR)/$$l; done
	install -m 644 $(MAN1) $(MANDIR)
	for l in $(LINKS); do ln -sf $(MAN1) $(MANDIR)/$$l.1; done

install: install-files

uninstall:
	rm -f $(BINDIR)/$(PROG) $(BINDIR)/$(LINKS)
	rm -f $(MANDIR)/$(MAN1) $(MANDIR)/$(LINKS)
