RELATIVEPATH=../../
include ${RELATIVEPATH}LINUX/config.make

LIBTRANSFER=$(RELATIVEPATH)libtransfer

CFLAGS     := $(subst ../,../../,$(CFLAGS))
LINK_FLAGS := -L$(LIBTRANSFER) -lopencbmtransfer $(subst ../,../../,$(LINK_FLAGS))

OBJS = main.o
PROG = samplelibtransfer

include ${RELATIVEPATH}LINUX/prgrules.make

# dependencies
$(PROG): $(LIBTRANSFER)/libopencbmtransfer.a
