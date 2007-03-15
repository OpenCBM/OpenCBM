# $Id: Makefile,v 1.1.2.3 2007-03-15 11:19:27 strik Exp $

include ../../../LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

SHLIB   = libopencbm-xa1541.so
SHLIBV  = $(SHLIB).$(MAJ)
SHLIBV3 = $(SHLIBV).$(MIN).$(REL)
LIB     = libopencbm-xa1541.a
SRCS    = LINUX/iec.c LINUX/parburst.c

OBJS    = $(SRCS:.c=.o)
SHOBJS  = $(SRCS:.c=.lo)

CFLAGS += -I../../../include/LINUX/ -I../../../include/ -I../../

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
	ldconfig -n $(PLUGINDIR)

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
	$(CC) -shared -o $@ $(SONAME)$(SHLIBV) $(SHOBJS) -lusb -ldl

$(LIB): $(OBJS)
	$(AR) r $@ $(OBJS)

### dependencies:

LINUX/iec.o LINUX/iec.lo: LINUX/iec.c ../../archlib.h
LINUX/parburst.o LINUX/parburst.lo: LINUX/parburst.c ../../archlib.h
