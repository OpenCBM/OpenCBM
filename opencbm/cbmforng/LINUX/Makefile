# $Id: Makefile,v 1.3 2007-04-29 10:46:22 strik Exp $

RELATIVEPATH=../
include ${RELATIVEPATH}LINUX/config.make

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

cbmforng.inc: cbmforng.a65 cbmfmacs.i65 cbmfsubs.i65 cbmforng.idh

cbmforng.o: cbmforng.c cbmforng.inc cbmforng.h cbmforng.idh
	$(CC) $(CFLAGS) -c cbmforng.c -o $@

cbmforng: cbmforng.o
	$(CC) $< -o $@ $(LINK_FLAGS)
