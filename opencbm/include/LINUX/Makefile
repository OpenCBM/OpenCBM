# $Id: Makefile,v 1.1 2005-03-02 18:17:19 strik Exp $

include ../LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

INSTHDRS = opencbm.h d64copy.h cbmcopy.h

all:

clean:

mrproper: clean

install-files:
	install -m 644 $(INSTHDRS) $(INCDIR)

install: install-files

uninstall:
	cd $(INCDIR) && rm -d $(INSTHDRS)
