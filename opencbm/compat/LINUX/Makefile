# $Id: Makefile,v 1.1 2005-03-02 18:17:18 strik Exp $

include ../LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

LIBS = libcbm4linux.a
OBJS = cbm.o upload.o
HDRS = cbm4linux.h

all: $(LIBS)

clean:
	rm -f $(LIBS) $(OBJS)

mrproper: clean

install-files:
	install -m 644 $(HDRS) $(INCDIR)
	install -m 644 $(LIBS) $(LIBDIR)

install: install-files

uninstall:
	rm -f $(INCDIR)/$(HDRS) $(LIBDIR)/$(LIBS)

.c.o:
	$(CC) $(LIB_CFLAGS) -DCBM4LINUX_OLDAPI -c $< -o $@

$(LIBS): $(OBJS)
	$(AR) r $@ $(OBJS)
