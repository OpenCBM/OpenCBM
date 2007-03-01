#
# Makefile
#

ECHO_TEST = usb_echo_test
EVENT_LOG = read_event_log

all: $(ECHO_TEST) $(EVENT_LOG)

clean:
	rm -f $(ECHO_TEST) $(EVENT_LOG)

$(ECHO_TEST): $(ECHO_TEST).c
	$(CC) -Wall -o $@ $< -lusb

$(EVENT_LOG): $(EVENT_LOG).c
	$(CC) -Wall -o $@ $< -lusb

