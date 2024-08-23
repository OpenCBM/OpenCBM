RELATIVEPATH=../../
include ${RELATIVEPATH}LINUX/config.make

LIBTRANSFER=$(RELATIVEPATH)libtransfer

CFLAGS += -I$(LIBTRANSFER)
LINK_FLAGS += -L$(LIBTRANSFER) -lopencbmtransfer

OBJS = main.o
PROG = samplelibtransfer

include ${RELATIVEPATH}LINUX/prgrules.make

# dependencies
$(PROG): $(LIBTRANSFER)/libopencbmtransfer.a
