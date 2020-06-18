# prgrules.make
# common rules for compiling a single program

# required defines: PROG
# optional defines: OBJS, MAN1, LINKS, INC

OBJS ?= $(PROG).o
MAN1 ?= $(PROG).1

.PHONY: all clean

all: $(PROG)

clean:
	rm -f $(PROG) $(OBJS)

include ${RELATIVEPATH}LINUX/prg_install_rules.make

.c.o:
	$(CC) -g $(CFLAGS) -o $@ -c $<

%.inc: %.a65
%.inc: %.idh

$(OBJS): $(INC)

$(PROG): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LINK_FLAGS)
