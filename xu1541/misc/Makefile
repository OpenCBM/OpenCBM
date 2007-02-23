#
# Makefile
#

APP = usb_echo_test

all: $(APP)

clean:
	rm -f $(APP)

$(APP): $(APP).c
	$(CC) -Wall -o $@ $(APP).c -lusb

