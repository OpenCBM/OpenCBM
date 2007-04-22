# $Id: Makefile,v 1.3 2007-04-22 18:46:50 wmsr Exp $

include ../LINUX/config.make
include ./dirs

.PHONY: all mrproper clean install uninstall install-files

SUBDIRS          = $(DIRS)
SUBDIRS_OPTIONAL = $(OPTIONAL_DIRS)

all:
	@for subdir in $(SUBDIRS); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile all || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile all; ) || true; \
	done

mrproper:
	@for subdir in $(SUBDIRS_ALL_NON_OPTIONAL); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

clean:
	@for subdir in $(SUBDIRS_ALL_NON_OPTIONAL); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

install-files: all
	mkdir -p -m 755 $(BINDIR) $(LIBDIR) $(MANDIR) $(INCDIR) $(MODDIR) $(INFODIR)
	@for subdir in $(SUBDIRS); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

install: install-files
	@for subdir in $(SUBDIRS); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

uninstall: 
	@for subdir in $(SUBDIRS_ALL_NON_OPTIONAL); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done
