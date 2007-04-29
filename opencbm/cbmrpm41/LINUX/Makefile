# $Id: Makefile,v 1.2 2007-04-29 10:46:22 strik Exp $

RELATIVEPATH=../
include ${RELATIVEPATH}LINUX/config.make

.PHONY: all mrproper clean install uninstall install-files

PROG = cbmrpm41
INC  = $(PROG).inc
MAN1 = $(PROG).1

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

$(PROG).inc: $(PROG).a65 $(PROG).idh

$(PROG).o: $(PROG).c $(PROG).inc $(PROG).h $(PROG).idh
	$(CC) $(CFLAGS) -c $(PROG).c -o $@

$(PROG): $(PROG).o
	$(CC) $< -o $@ $(LINK_FLAGS)
