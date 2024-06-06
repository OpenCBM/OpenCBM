RELATIVEPATH=../../
include ${RELATIVEPATH}LINUX/config.make

CFLAGS     := $(subst ../,../../,$(CFLAGS))
LINK_FLAGS := $(subst ../,../../,$(LINK_FLAGS))

PROG = fdx000copy

EXTRA_A65_INC= \
	turboreadfdx000.inc

fdx000copy.o: turboreadfdx000.inc

include ${RELATIVEPATH}LINUX/prgrules.make
