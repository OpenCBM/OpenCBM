# $Id: Makefile,v 1.4 2005-11-20 12:23:53 strik Exp $

include ../LINUX/config.make

.PHONY: all mrproper clean install uninstall install-files

OBJS = mnib.o read.o write.o gcr.o prot.o
PROG = mnib n2d n2g pscan pshift
# MAN1 = mnib.1

all:
	make -f GNU/Makefile CBM_LNX_PATH="../" linux

mrproper: clean

clean:
	make -f GNU/Makefile CBM_LNX_PATH="../" distclean

install-files: $(PROG)
	install -m 755 -s $(PROG) $(BINDIR)
#	install -m 644 $(MAN1) $(MANDIR)

install: install-files

uninstall:
	rm -f $(BINDIR)/$(PROG)	# $(MANDIR)/$(MAN1)
