# $Id: Makefile,v 1.3 2007-04-22 18:46:50 wmsr Exp $

include ../../LINUX/config.make
CFLAGS     := $(subst ../,../../,$(CFLAGS))
LINK_FLAGS := $(subst ../,../../,$(LINK_FLAGS))

.PHONY: all mrproper clean install uninstall install-files

PROG = frm_analyzer
INC  = frm_analyzer.inc
MAN1 = frm_analyzer.1

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

frm_analyzer.o: frm_analyzer.c frm_analyzer.inc
	$(CC) $(CFLAGS) -c frm_analyzer.c -o $@

frm_analyzer: frm_analyzer.o
	$(CC) $< -o $@ $(LINK_FLAGS)
