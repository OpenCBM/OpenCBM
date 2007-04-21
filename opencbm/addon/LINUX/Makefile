# $Id: Makefile,v 1.1 2007-04-21 09:05:47 wmsr Exp $

include ../LINUX/config.make

.PHONY: all mrproper clean install uninstall install-files

SUBDIRS  = frm_analyzer weaktest

all:
	@for subdir in $(SUBDIRS); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile all; ) || true; \
	done

mrproper:
	@for subdir in $(SUBDIRS); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

clean:
	@for subdir in $(SUBDIRS); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

install-files: all
	mkdir -p -m 755 $(BINDIR) $(LIBDIR) $(MANDIR) $(INCDIR) $(MODDIR) $(INFODIR)
	@for subdir in $(SUBDIRS); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

install: install-files
	@for subdir in $(SUBDIRS); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

uninstall: 
	@for subdir in $(SUBDIRS); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done
