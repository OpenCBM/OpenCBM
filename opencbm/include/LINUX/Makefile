RELATIVEPATH=../
include ${RELATIVEPATH}LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

INSTHDRS = opencbm.h d64copy.h cbmcopy.h

all:

clean:

mrproper: clean

install-files:
	install -d $(DESTDIR)$(INCDIR)
	install -m 644 $(INSTHDRS) $(DESTDIR)$(INCDIR)

install: install-files

uninstall:
	-cd $(DESTDIR)$(INCDIR) && rm -d $(INSTHDRS)
