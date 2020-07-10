PROG=cia_irq_c128.prg cia_irq_c64.prg
SRC=cia_irq.c CC65/opencbm.c
ASM_SRC=cia-sdr-icr-1571.a65 cia-sdr-icr-1581.a65 irqdelay-1571.a65 irqdelay-1581.a65 irqdelay-1571-oneshot.a65 irqdelay-1581-oneshot.a65

ASM_OBJ=$(patsubst %.a65,%.o65,$(ASM_SRC))
ASM_INC=$(patsubst %.a65,%.inc,$(ASM_SRC))
ASM_LST=$(patsubst %.a65,%.l65,$(ASM_SRC))
C_OBJ=$(patsubst %.c,%.o,$(SRC))

CC=cl65
CL65=cl65
LD65=ld65

CFLAGS=-ICC65/ -Os
CA65_FLAGS  = --feature labels_without_colons --feature pc_assignment --feature loose_char_term
OD_FLAGS    = -w8 -txC -v -An

.SUFFIXES: .inc .o65 .a65 .c .h .o

.PHONY: all clean mrproper test

all: $(PROG)

test:
	@echo ASM_OBJ=$(ASM_OBJ)
	@echo ASM_LST=$(ASM_LST)
	@echo ASM_INC=$(ASM_LST)
	@echo C_OBJ=$(C_OBJ)

.a65.o65:
	@echo Building $@
	@$(CL65) -c $(CA65_FLAGS) -o $*.tmp -l $(patsubst %.a65,%.l65,$<) $<
	@$(LD65) -o $@ --target none $*.tmp && rm -f $*.tmp

.o65.inc:
	@echo Building $@
	@test -s $< && od $(OD_FLAGS) $< | \
	sed 's/\([0-9a-f]\{2\}\) */0x\1,/g; $$s/,$$//' > $@

clean:
	rm -f -- $(C_OBJ) $(ASM_INC) $(ASM_OBJ) $(ASM_LST)

mrproper: clean
	rm -f -- *~ */*~ $(PROG)


# dependencies:

cia_irq_c64.prg: $(SRC) $(ASM_INC)
	$(CL65) $(CFLAGS) -t c64 -o $@ $(SRC)

cia_irq_c128.prg: $(SRC) $(ASM_INC)
	$(CL65) $(CFLAGS) -t c128 -o $@ $(SRC)

cia_irq.o: $(ASM_INC)

cia-sdr-icr-1571.inc: cia-sdr-icr-1571.o65

cia-sdr-icr-1571.o65: cia-sdr-icr-1571.a65 cia-sdr-icr.a65

cia-sdr-icr-1581.inc: cia-sdr-icr-1581.o65

cia-sdr-icr-1581.o65: cia-sdr-icr-1581.a65 cia-sdr-icr.a65

irqdelay-1571.inc: irqdelay-1571.o65

irqdelay-1571.o65: irqdelay-1571.a65 irqdelay-15x1.a65

irqdelay-1581.inc: irqdelay-1581.o65

irqdelay-1581.o65: irqdelay-1581.a65 irqdelay-15x1.a65

irqdelay-1571-oneshot.inc: irqdelay-1571-oneshot.o65

irqdelay-1571-oneshot.o65: irqdelay-1571-oneshot.a65 irqdelay-15x1.a65

irqdelay-1581-oneshot.inc: irqdelay-1581-oneshot.o65

irqdelay-1581-oneshot.o65: irqdelay-1581-oneshot.a65 irqdelay-15x1.a65
