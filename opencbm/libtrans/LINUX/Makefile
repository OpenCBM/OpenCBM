RELATIVEPATH=../
include ${RELATIVEPATH}LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

#CFLAGS += -I../include

LIB     = libtrans.a
SRCS    = pp.c \
	  s1.c \
	  s2.c \
	  s3.c \
	  turbo.c

INCS    = pp-1541.inc \
	  pp-1571.inc \
	  s1-1541-1571.inc \
	  s1-1581.inc \
	  s2-1541-1571.inc \
	  s2-1581.inc \
	  s3-1571.inc \
	  s3-1581.inc \
	  turbomain-1541-1571.inc \
	  turbomain-1581.inc \


OBJS    = $(SRCS:.c=.lo)

all: $(LIB)

clean:
	rm -f $(OBJS) $(LIB)

mrproper: clean
	rm -f $(INCS)
	rm -f *~ LINUX/*~ WINDOWS/*~

install-files:

install: install-files

uninstall:

pp-1541.inc: pp-1541.a65 common.i65

pp-1571.inc: pp-1571.a65 common.i65

s1.lo: s1.c s1-1541-1571.inc s1-1581.inc

s2.lo: s2.c s2-1541-1571.inc s2-1581.inc

s3.lo: s3.c s3-1571.inc s3-1581.inc

pp.lo: pp.c pp-1541.inc pp-1571.inc

turbo.lo: turbomain-1541-1571.inc turbomain-1581.inc

.c.o:
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(LIB): $(OBJS)
	$(AR) r $@ $(OBJS)
