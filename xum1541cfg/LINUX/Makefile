RELATIVEPATH=../opencbm/
include ${RELATIVEPATH}LINUX/config.make

CFLAGS+= -g -I /opt/local/include -I $(DFU_SRC) -Wall
LINK_FLAGS= -L/opt/local/lib -lusb

DFU_SRC= dfu-programmer-0.5.4/src

OBJS= main.o util.o
OBJS+= $(DFU_SRC)/arguments.o $(DFU_SRC)/atmel.o $(DFU_SRC)/commands.o $(DFU_SRC)/dfu.o $(DFU_SRC)/intel_hex.o $(DFU_SRC)/util.o

PROG = xum1541cfg
LINKS = 

#$(LIBD82COPY)/d82copy.o $(LIBD82COPY)/d82copy.lo: \
#  $(LIBD82COPY)/d82copy.c $(LIBD82COPY)/d82copy_int.h \
#  ../include/arch.h
#$(LIBD82COPY)/fs.o $(LIBD82COPY)/fs.lo: \
#  $(LIBD82COPY)/fs.c $(LIBD82COPY)/d82copy_int.h \
#  ../include/arch.h
#$(LIBD82COPY)/std.o $(LIBD82COPY)/std.lo: \
#  $(LIBD82COPY)/std.c ../include/opencbm.h \
#  $(LIBD82COPY)/d82copy_int.h

include ${RELATIVEPATH}LINUX/prgrules.make
