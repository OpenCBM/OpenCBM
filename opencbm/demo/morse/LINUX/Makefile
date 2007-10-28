# $Id: Makefile,v 1.1.10.1 2007-10-28 10:54:25 strik Exp $

include ../config.make

.PHONY: all mrproper clean install uninstall install-files

DEMO    = morse

PROGS   = $(DEMO)
INC     = $(DEMO).inc

all: $(PROGS)

clean:
	rm -f $(PROGS) *.o *.o65

mrproper: clean
	rm -f $(INC)

$(DEMO).o: $(DEMO).c $(DEMO).inc
	$(CC) $(ALL_CFLAGS) -c $(DEMO).c -o $@

$(DEMO): $(DEMO).o
	$(CC) $< -o $@ $(LINK_FLAGS)

install-files:

install:

uninstall:
