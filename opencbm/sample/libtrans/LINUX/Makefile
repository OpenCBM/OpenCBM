# $Id: Makefile,v 1.2 2007-04-29 10:46:23 strik Exp $

RELATIVEPATH=../../
include ${RELATIVEPATH}LINUX/config.make
CFLAGS     := $(subst ../,../../,$(CFLAGS))
LINK_FLAGS := $(subst ../,../../,$(LINK_FLAGS))

LINK_FLAGS   += -L../../libtrans/ -ltrans

.PHONY: all mrproper clean install uninstall install-files

OBJS = main.o
PROG = samplelibtransf
#MAN1 = d64copy.1

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
	rm -f $(BINDIR)/$(PROG) # $(MANDIR)/$(MAN1)
