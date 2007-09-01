#
# Makefile
#

# determine libusb location
OS=$(shell uname -s)
ifeq "$(OS)" "Darwin"
LIBUSB_DIR = $(HOME)/libusb
else
LIBUSB_DIR = /usr
endif

.PHONY: all clean xu1541lib

# add libusb
CFLAGS  = -I$(LIBUSB_DIR)/include -I../include/ -Wall
LDFLAGS = -L$(LIBUSB_DIR)/lib
LIBS    = -lusb -L../lib -lxu1541

ECHO_TEST = usb_echo_test
EVENT_LOG = read_event_log

all: $(ECHO_TEST) $(EVENT_LOG)

clean:
	rm -f $(ECHO_TEST) $(EVENT_LOG)

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)

$(ECHO_TEST): xu1541lib

$(EVENT_LOG): xu1541lib

xu1541lib:
	make -C ../lib/
