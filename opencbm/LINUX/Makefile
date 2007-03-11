# $Id: Makefile,v 1.10.2.2 2007-03-11 13:46:00 strik Exp $

include LINUX/config.make

CBMDEV   = /dev/cbm
CBMPERM  = 666
DEVMAJOR = 10
DEVMINOR = 177
SUBDIRS  = include arch/$(ARCH) lib compat \
	   lib/plugin/xu1541 \
	   libtrans \
           cbmctrl cbmformat cbmforng d64copy cbmcopy \
           demo/flash demo/morse demo/rpm1541 \
	   sample/libtrans
SUBDIRS_DOC = docs

SUBDIRS_OPTIONAL = mnib36 nibtools

.PHONY: all clean mrproper dist doc install install-doc uninstall dev install-files install-files-doc all-doc

all: 
	@for subdir in $(SUBDIRS); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

doc:
	@for subdir in $(SUBDIRS_DOC); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done

all-doc: all doc

clean:
	@for subdir in $(SUBDIRS); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

mrproper:
	@for subdir in $(SUBDIRS); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

dist: all clean
	-rm -f build-stamp configure-stamp
	tar czvf ../`basename \`pwd\``.tar.gz --exclude=CVS --exclude=debian -C .. `basename \`pwd\``

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

install-files-doc: install-files all-doc
	@for subdir in $(SUBDIRS_DOC); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done

install-doc: install-files-doc install
	@for subdir in $(SUBDIRS_DOC); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done

uninstall: 
	@for subdir in $(SUBDIRS) $(SUBDIRS_DOC); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

dev:
	mkdir -p -m 755 `dirname $(CBMDEV)`
	rm -f $(CBMDEV)
	mknod -m $(CBMPERM) $(CBMDEV) c $(DEVMAJOR) $(DEVMINOR)
