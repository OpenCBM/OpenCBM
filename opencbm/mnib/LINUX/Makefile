# $Id: Makefile,v 1.1.2.1 2005-09-16 12:39:54 strik Exp $

include ../LINUX/config.make

.PHONY: all mrproper clean install uninstall install-files

OBJS = mnib.o read.o write.o gcr.o prot.o
PROG = mnib
# MAN1 = mnib.1

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

all: $(PROG)

mrproper: clean

clean:
	rm -f $(PROG) $(OBJS)

$(PROG): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LINK_FLAGS)

install-files: $(PROG)
	install -m 755 -s $(PROG) $(BINDIR)
#	install -m 644 $(MAN1) $(MANDIR)

install: install-files

uninstall:
	rm -f $(BINDIR)/$(PROG)	# $(MANDIR)/$(MAN1)
