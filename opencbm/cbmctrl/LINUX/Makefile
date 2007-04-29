# $Id: Makefile,v 1.3 2007-04-29 10:46:22 strik Exp $

RELATIVEPATH=../
include ${RELATIVEPATH}LINUX/config.make

.PHONY: all mrproper clean install uninstall install-files

PROGS   = cbmctrl
OBJS    = cbmctrl.o
INC     = tdchange.inc
MAN1    = cbmctrl.1

all: $(PROGS)

clean:
	rm -f $(PROGS) $(OBJS) *.o65

mrproper: clean
	rm -f $(INC)

install-files: $(PROGS)
	install -m 755 -s $(PROGS) $(BINDIR)
	install -m 644 $(MAN1) $(MANDIR)

install: install-files

uninstall:
	for f in $(PROGS); do rm -f $(BINDIR)/$$f; done
	for f in $(MAN1); do rm -f $(MANDIR)/$$f{,.gz}; done

cbmctrl.o: cbmctrl.c tdchange.inc
	$(CC) $(CFLAGS) -c cbmctrl.c -o $@
	
cbmctrl: $(OBJS)
	$(CC) $(CFLAGS) $< -o $@ $(LINK_FLAGS)
