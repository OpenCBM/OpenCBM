# $Id: Makefile,v 1.2 2007-04-22 16:15:31 strik Exp $

include ../LINUX/config.make
include dirs

.PHONY: all mrproper clean install uninstall install-files

all:
	@for subdir in $(DIRS); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile all; ) || true; \
	done

mrproper:
	@for subdir in $(DIRS); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

clean:
	@for subdir in $(DIRS); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

install-files: all
	mkdir -p -m 755 $(BINDIR) $(LIBDIR) $(MANDIR) $(INCDIR) $(MODDIR) $(INFODIR)
	@for subdir in $(DIRS); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

install: install-files
	@for subdir in $(DIRS); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

uninstall: 
	@for subdir in $(DIRS); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done
