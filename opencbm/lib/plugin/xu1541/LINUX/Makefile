# $Id: Makefile,v 1.1.2.2 2007-03-13 09:59:37 strik Exp $

include ../../../LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

SHLIB   = libopencbm-xu1541.so
SHLIBV  = $(SHLIB).$(MAJ)
SHLIBV3 = $(SHLIBV).$(MIN).$(REL)
LIB     = libopencbm-xu1541.a
SRCS    = archlib.c xu1541.c

OBJS    = $(SRCS:.c=.o)
SHOBJS  = $(SRCS:.c=.lo)

CFLAGS += -I../../../include/LINUX/ -I../../../include/ -I../../ -I $(XU1541DIR)

all: $(SHLIB) $(LIB) $(SHLIBV) $(SHLIBV3)

clean:
	rm -f $(OBJS) $(SHOBJS) $(LIB) $(SHLIB) $(SHLIBV) $(SHLIBV3)
	rm -f $(LIBD64COPY)/*.o65 $(LIBCBMCOPY)/*.o65

mrproper: clean
	rm -f $(LIBCBMCOPY)/*.inc $(LIBD64COPY)/*.inc

install-files:
	install -m 755 $(SHLIBV3) $(LIBDIR)
	install -m 644 $(LIB) $(LIBDIR)
	cd $(LIBDIR) && ln -sf $(SHLIBV3) $(SHLIBV); ln -sf $(SHLIBV) $(SHLIB)

install: install-files
	ldconfig -n $(LIBDIR)

uninstall:
	cd $(LIBDIR) && rm -f $(LIB) $(SHLIB) $(SHLIBV) $(SHLIBV3)

.c.o:
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(SHLIB): $(SHLIBV)
	ln -sf $< $@

$(SHLIBV): $(SHLIBV3)
	ln -sf $< $@

$(SHLIBV3): $(SHOBJS)
	$(CC) -shared -o $@ $(SONAME)$(SHLIBV) $(SHOBJS) -lusb -ldl

$(LIB): $(OBJS)
	$(AR) r $@ $(OBJS)

### dependencies:

archlib.o archlib.lo: ../../archlib.h xu1541.h
xu1541.o xu1541.lo: xu1541.h
