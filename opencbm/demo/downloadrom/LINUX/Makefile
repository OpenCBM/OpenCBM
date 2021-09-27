RELATIVEPATH=../
include ../config.make

.PHONY: all mrproper clean install uninstall install-files

DOWNLOADROM = downloadrom

PROGS   = $(DOWNLOADROM)
INC     = $(DOWNLOADROM).inc

all: $(PROGS)

clean:
	rm -f $(PROGS) *.o *.o65

mrproper: clean
	rm -f $(INC)

$(DOWNLOADROM).o: $(DOWNLOADROM).c $(DOWNLOADROM).inc
	$(CC) $(CFLAGS) -c $(DOWNLOADROM).c -o $@

$(DOWNLOADROM): $(DOWNLOADROM).o
	$(CC) $< -o $@ $(LINK_FLAGS)

install-files:

install:

uninstall:
