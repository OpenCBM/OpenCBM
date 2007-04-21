# $Id: Makefile,v 1.4 2007-04-21 19:17:10 cnvogelg Exp $

include ../../../LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

LIBNAME = libopencbm-xu1541
SRCS    = archlib.c xu1541.c s1_s2_pp.c

OBJS    = $(SRCS:.c=.o)
SHOBJS  = $(SRCS:.c=.lo)

CFLAGS += -I$(LIBUSBDIR)/include -I../../../include/LINUX/ -I../../../include/ -I../../ -I $(XU1541DIR)
LDFLAGS += -L$(LIBUSBDIR)/lib

all: $(SHLIB) $(LIB) $(SHLIBV) $(SHLIBV3)

clean:
	rm -f $(OBJS) $(SHOBJS) $(LIB) $(SHLIB) $(SHLIBV) $(SHLIBV3)
	rm -f $(LIBD64COPY)/*.o65 $(LIBCBMCOPY)/*.o65

mrproper: clean
	rm -f $(LIBCBMCOPY)/*.inc $(LIBD64COPY)/*.inc

install-files:
	install -d $(PLUGINDIR)
	install -m 755 $(SHLIBV3) $(PLUGINDIR)
	install -m 644 $(LIB) $(PLUGINDIR)
	cd $(PLUGINDIR) && ln -sf $(SHLIBV3) $(SHLIBV); ln -sf $(SHLIBV) $(SHLIB)

install: install-files
ifeq "$(OS)" "Linux"
	ldconfig -n $(PLUGINDIR)
endif

uninstall:
	-cd $(PLUGINDIR) && rm -f $(LIB) $(SHLIB) $(SHLIBV) $(SHLIBV3)
	-rmdir -p $(PLUGINDIR)

.c.o:
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(SHLIB): $(SHLIBV)
	ln -sf $< $@

$(SHLIBV): $(SHLIBV3)
	ln -sf $< $@

$(SHLIBV3): $(SHOBJS)
	$(CC) $(LDFLAGS) $(SHLIB_SWITCH) -o $@ $(SONAME)$(SHLIBV) $(SHOBJS) -lusb -ldl

$(LIB): $(OBJS)
	$(AR) r $@ $(OBJS)

### dependencies:

archlib.o archlib.lo: ../../archlib.h xu1541.h
xu1541.o xu1541.lo: xu1541.h
