LIBUSB_CONFIG   = libusb-config
CFLAGS+=-g -Wall -pedantic `$(LIBUSB_CONFIG) --cflags` -I../../include/
LFLAGS+=`$(LIBUSB_CONFIG) --libs` -lusb

APP = xu1541_update
OBJS = xu1541_update.o ihex.o flash.o
LIBS = -L ../../lib/ -lxu1541

.INTERMEDIATE: $(OBJS)
.PHONY:	all clean lib

all:
	make ../$(APP)

clean:
	rm -f *.o *~

../$(APP): $(OBJS) lib
	gcc $(OBJS) -o $@ $(LFLAGS) $(LIBS)

lib:
	make -C ../../lib/
