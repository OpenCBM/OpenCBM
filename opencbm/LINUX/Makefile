# $Id: Makefile,v 1.5 2006-03-22 09:13:59 strik Exp $

include LINUX/config.make

CBMDEV   = /dev/cbm
CBMPERM  = 666
DEVMAJOR = 10
DEVMINOR = 177
SUBDIRS  = sys/linux include arch/$(ARCH) lib compat \
           cbmctrl cbmformat cbmforng d64copy cbmcopy \
           demo/flash demo/morse docs

SUBDIRS_OPTIONAL = mnib36

.PHONY: all clean mrproper dist install uninstall dev install-files

all: 
	@for subdir in $(SUBDIRS); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	-@for subdir in $(SUBDIRS_OPTIONAL); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@; \
	done

clean:
	@for subdir in $(SUBDIRS); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	-@for subdir in $(SUBDIRS_OPTIONAL); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@; \
	done

mrproper:
	@for subdir in $(SUBDIRS); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	-@for subdir in $(SUBDIRS_OPTIONAL); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@; \
	done

dist: all clean
	-rm -f build-stamp configure-stamp
	tar czvf ../`basename \`pwd\``.tar.gz --exclude=CVS --exclude=debian -C .. `basename \`pwd\``

install-files: all
	mkdir -p -m 755 $(BINDIR) $(LIBDIR) $(MANDIR) $(INCDIR) $(MODDIR) $(INFODIR)
	@for subdir in $(SUBDIRS); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	-@for subdir in $(SUBDIRS_OPTIONAL); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@; \
	done

install: install-files
	@for subdir in $(SUBDIRS); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	-@for subdir in $(SUBDIRS_OPTIONAL); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@; \
	done

uninstall: 
	@for subdir in $(SUBDIRS); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	-@for subdir in $(SUBDIRS_OPTIONAL); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@; \
	done

dev:
	mkdir -p -m 755 `dirname $(CBMDEV)`
	rm -f $(CBMDEV)
	mknod -m $(CBMPERM) $(CBMDEV) c $(DEVMAJOR) $(DEVMINOR)
