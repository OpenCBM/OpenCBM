# $Id: Makefile,v 1.1 2005-03-02 18:17:17 strik Exp $

include ../LINUX/config.make

.PHONY: all mrproper clean install uninstall install-files

PROG = cbmformat
INC  = cbmformat.inc
MAN1 = cbmformat.1

all: $(PROG)

clean:
	rm -f $(PROG) *.o *.o65

mrproper: clean
	rm -f $(INC)

install-files: $(PROG)
	install -m 755 -s $(PROG) $(BINDIR)
	install -m 644 $(MAN1) $(MANDIR)

install: install-files

uninstall:
	rm -f $(BINDIR)/$(PROG) $(MANDIR)/$(MAN1)

cbmformat.o: cbmformat.c cbmformat.inc
	$(CC) $(CFLAGS) -c cbmformat.c -o $@

cbmformat: cbmformat.o
	$(CC) $< -o $@ $(LINK_FLAGS)
