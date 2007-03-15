# $Id: Makefile,v 1.1.8.1 2007-03-15 11:19:27 strik Exp $

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
	-cd $(INCDIR) && rm -d $(INSTHDRS)
