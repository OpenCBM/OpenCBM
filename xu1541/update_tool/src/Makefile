LIBUSB_CONFIG   = libusb-config
CFLAGS+=-g -Wall -pedantic `$(LIBUSB_CONFIG) --cflags`
LFLAGS+=`$(LIBUSB_CONFIG) --libs` -lusb

all:
	make ../xu1541_update

clean:
	rm *.o *~

../xu1541_update: main.cpp cflashmem.o cpage.o cbootloader.o
	g++ $(CFLAGS) main.cpp cflashmem.o cpage.o cbootloader.o -o $@ $(LFLAGS)
