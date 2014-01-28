DFU_SRC= dfu-programmer-0.5.4/src

CFLAGS= -g -I /opt/local/include -I $(DFU_SRC) -Wall
LDFLAGS= -L/opt/local/lib -lusb

OBJS= main.o util.o
OBJS+= $(DFU_SRC)/arguments.o $(DFU_SRC)/atmel.o $(DFU_SRC)/commands.o $(DFU_SRC)/dfu.o $(DFU_SRC)/intel_hex.o $(DFU_SRC)/util.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

xum1541cfg: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -f $(OBJS) xum1541cfg
