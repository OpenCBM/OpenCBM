# $Id: Makefile,v 1.3 2007-04-29 10:46:23 strik Exp $

RELATIVEPATH=../
include ${RELATIVEPATH}LINUX/config.make

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
