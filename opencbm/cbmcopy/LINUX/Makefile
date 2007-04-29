# $Id: Makefile,v 1.3 2007-04-29 10:46:22 strik Exp $

RELATIVEPATH=../
include ${RELATIVEPATH}LINUX/config.make

.PHONY: all mrproper clean install uninstall install-files

OBJS = main.o pc64.o t64.o raw.o
PROG = cbmcopy
MAN1 = cbmcopy.1
LINKS = cbmread cbmwrite

all: $(PROG)

mrproper: clean

.c.o:
	$(CC) -g $(CFLAGS) -o $@ -c $<

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
	rm -f $(BINDIR)/$(PROG) $(MANDIR)/$(MAN1)
	for l in $(LINKS); do rm -f $(BINDIR)/$$l $(MANDIR)/$$l.1; done
	-rmdir -p $(BINDIR) $(MANDIR)
