# $Id: Makefile,v 1.1 2006-02-12 15:13:29 strik Exp $

include ../LINUX/config.make

.PHONY: all mrproper clean install uninstall install-files

PROG = cbmforng
INC  = cbmforng.inc
MAN1 = cbmforng.1

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

cbmforng.inc: cbmforng.a65 cbmfmacs.i65 cbmfsubs.i65

cbmforng.o: cbmforng.c cbmforng.inc
	$(CC) $(CFLAGS) -c cbmforng.c -o $@

cbmforng: cbmforng.o
	$(CC) $< -o $@ $(LINK_FLAGS)
