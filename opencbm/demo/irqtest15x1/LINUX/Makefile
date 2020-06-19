RELATIVEPATH=../
include ../config.make

.PHONY: all mrproper clean install uninstall install-files

DEMO    = cia_irq

PROGS   = $(DEMO)

INC= \
	irqdelay-1571.inc \
	irqdelay-1571-oneshot.inc \
	irqdelay-1581.inc \
	irqdelay-1581-oneshot.inc \


all: $(PROGS)

clean:
	rm -f $(PROGS) *.o *.o65

mrproper: clean
	rm -f $(INC) *~ */*~

$(DEMO).o: $(DEMO).c $(INC)
	$(CC) $(CFLAGS) -c $(DEMO).c -o $@

$(DEMO): $(DEMO).o
	$(CC) $< -o $@ $(LINK_FLAGS)

install-files:

install:

uninstall:

