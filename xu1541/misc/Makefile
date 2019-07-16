#
# Makefile
#

# call with "export XMINGW=mingw" before if you want to use the MINGW cross compiler

include ../include/common.mk

# remove if you want to generate debugging information
LDFLAGS_EXTRA+=-s

.PHONY: all clean mrproper xu1541lib

# add libusb
CFLAGS  = $(CFLAGS_EXTRA) -I$(LIBUSB_DIR)/include -I../include/ -Wall
LDFLAGS = $(LDFLAGS_EXTRA) -L$(LIBUSB_DIR)/lib
LIBS    = -L../lib -lxu1541$(LIB_WIN)

ECHO_TEST = usb_echo_test
EVENT_LOG = read_event_log

all: $(ECHO_TEST)$(EXE_SUFFIX) $(EVENT_LOG)$(EXE_SUFFIX)

clean:
	rm -f $(ECHO_TEST)$(EXE_SUFFIX) $(EVENT_LOG)$(EXE_SUFFIX)

mrproper: clean
	rm -f $(ECHO_TEST) $(EVENT_LOG) $(ECHO_TEST).exe $(EVENT_LOG).exe

%: %.c
	$(CC) -o $@ $< $(LIBS) $(CFLAGS) $(LDFLAGS) 

%.exe: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)

$(ECHO_TEST)$(EXE_SUFFIX): xu1541lib

$(EVENT_LOG)$(EXE_SUFFIX): xu1541lib

xu1541lib:
	$(MAKE) -C ../lib/
