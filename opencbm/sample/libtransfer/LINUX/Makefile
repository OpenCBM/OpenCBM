RELATIVEPATH=../../
include ${RELATIVEPATH}LINUX/config.make

LIBTRANSFER=$(RELATIVEPATH)libtransfer

LINK_FLAGS := -L$(LIBTRANSFER) -lopencbmtransfer $(LINK_FLAGS)

OBJS = main.o
PROG = samplelibtransfer

include ${RELATIVEPATH}LINUX/prgrules.make

# dependencies
$(PROG): $(LIBTRANSFER)/libopencbmtransfer.a
