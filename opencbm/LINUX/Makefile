# $Id: Makefile,v 1.10.2.4 2007-03-15 06:59:55 strik Exp $

include LINUX/config.make

CBMDEV   = /dev/cbm
CBMPERM  = 666
DEVMAJOR = 10
DEVMINOR = 177
SUBDIRS  = include arch/$(ARCH) lib compat \
	   libtrans \
           cbmctrl cbmformat cbmforng d64copy cbmcopy \
           demo/flash demo/morse demo/rpm1541 \
	   sample/libtrans


SUBDIRS_DOC = docs

SUBDIRS_PLUGIN_XU1541 = lib/plugin/xu1541

SUBDIRS_PLUGIN_XA1541 = lib/plugin/xa1541 sys/linux/

SUBDIRS_OPTIONAL = mnib36 nibtools


SUBDIRS_PLUGIN          = $(SUBDIRS_PLUGIN_XU1541) $(SUBDIRS_PLUGIN_XA1541)

SUBDIRS_ALL_NON_OPTIONAL= $(SUBDIRS) $(SUBDIRS_DOC) $(SUBDIRS_PLUGIN)


.PHONY: all opencbm clean mrproper dist doc install install-doc uninstall dev install-files install-files-doc all-doc plugin-xu1541 plugin-xa1541 plugin install-plugin install-plugin-xu1541 install-plugin-xa1541

all: opencbm plugin

all-doc: all doc

opencbm:
	@for subdir in $(SUBDIRS); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile all || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile all; ) || true; \
	done

doc:
	@for subdir in $(SUBDIRS_DOC); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done

clean:
	@for subdir in $(SUBDIRS_ALL_NON_OPTIONAL); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

mrproper:
	@for subdir in $(SUBDIRS_ALL_NON_OPTIONAL); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

dist: all-doc clean
	rm -f build-stamp configure-stamp
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

install-plugin-xu1541: plugin-xu1541
	@for subdir in $(SUBDIRS_PLUGIN_XU1541); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile install || exit 1; \
	done

install-plugin-xa1541: plugin-xa1541
	@for subdir in $(SUBDIRS_PLUGIN_XA1541); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile install || exit 1; \
	done

install-plugin: install-plugin-xu1541 install-plugin-xa1541

install-files-doc: install-files all-doc
	@for subdir in $(SUBDIRS_DOC); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done

install-doc: install-files-doc install
	@for subdir in $(SUBDIRS_DOC); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done

plugin-xu1541:
	@for subdir in $(SUBDIRS_PLUGIN_XU1541); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile || exit 1; \
	done

plugin-xa1541:
	@for subdir in $(SUBDIRS_PLUGIN_XA1541); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile || exit 1; \
	done

plugin: plugin-xu1541 plugin-xa1541

uninstall: 
	@for subdir in $(SUBDIRS_ALL_NON_OPTIONAL); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

dev:
	mkdir -p -m 755 `dirname $(CBMDEV)`
	rm -f $(CBMDEV)
	mknod -m $(CBMPERM) $(CBMDEV) c $(DEVMAJOR) $(DEVMINOR)
