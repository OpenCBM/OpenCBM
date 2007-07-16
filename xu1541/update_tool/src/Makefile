LIBUSB_CONFIG   = libusb-config
CFLAGS+=-g -Wall -pedantic `$(LIBUSB_CONFIG) --cflags`
LFLAGS+=`$(LIBUSB_CONFIG) --libs` -lusb

APP = xu1541_update
OBJS = xu1541_update.o ihex.o flash.o

.INTERMEDIATE: $(OBJS)

all:
	make ../$(APP)

clean:
	rm -f *.o *~

../$(APP): $(OBJS)
	gcc $(OBJS) -o $@ $(LFLAGS)
