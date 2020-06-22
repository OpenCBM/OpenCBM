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
	cia-sdr-icr-1571.inc \
	cia-sdr-icr-1581.inc \
	dumps/dump1-4.ibin \
	dumps/dump1-50.ibin \
	dumps/dump1-51.ibin \
	dumps/dump1-5.ibin \
	dumps/dump1-64.ibin \
	dumps/dump1-6.ibin \
	dumps/dump1-7.ibin \
	dumps/dump1-8.ibin \
	dumps/dump2-4485-50.ibin \
	dumps/dump2-4485-51.ibin \
	dumps/dump2-4485-64.ibin \
	dumps/dump2-4485-7.ibin \
	dumps/dump2-4485-8.ibin \
	dumps/dump2-4.ibin \
	dumps/dump2-50.ibin \
	dumps/dump2-51.ibin \
	dumps/dump2-5.ibin \
	dumps/dump2-64.ibin \
	dumps/dump2-6.ibin \
	dumps/dump2-7.ibin \
	dumps/dump2-8.ibin

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

.SUFFIXES: .bin .ibin

.bin.ibin:
	test -s $< && od $(OD_FLAGS) $< | \
	sed 's/\([0-9a-f]\{2\}\) */0x\1,/g; $$s/,$$//' > $@
