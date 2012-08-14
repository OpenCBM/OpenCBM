RELATIVEPATH=./
include ${RELATIVEPATH}LINUX/config.make

CBMDEV   = /dev/cbm
CBMPERM  = 666
DEVMAJOR = 10
DEVMINOR = 177
SUBDIRS  = include arch/$(OS_ARCH) libmisc lib \
	   libtrans \
           cbmctrl cbmformat cbmforng d64copy cbmcopy \
	   d82copy \
           demo/flash demo/morse demo/rpm1541 \
	   sample/libtrans
ifeq "$(OS)" "Linux"
SUBDIRS += compat
endif

SUBDIRS_DOC = docs

SUBDIRS_PLUGIN_XU1541 = lib/plugin/xu1541

SUBDIRS_PLUGIN_XUM1541 = lib/plugin/xum1541

SUBDIRS_PLUGIN_XA1541 = lib/plugin/xa1541 sys/linux/

SUBDIRS_OPTIONAL = addon nibtools mnib36 cbmrpm41


SUBDIRS_PLUGIN          = $(SUBDIRS_PLUGIN_XUM1541) $(SUBDIRS_PLUGIN_XU1541) $(SUBDIRS_PLUGIN_XA1541)

SUBDIRS_ALL_NON_OPTIONAL= $(SUBDIRS) $(SUBDIRS_DOC) $(SUBDIRS_PLUGIN)

ifeq "$(OS)" "Darwin"
PLUGINS=plugin-xum1541 plugin-xu1541
INSTALL_PLUGINS=install-plugin-xum1541 install-plugin-xu1541
else
ifeq "$(OS)" "FreeBSD"
PLUGINS=plugin-xum1541 plugin-xu1541
INSTALL_PLUGINS=install-plugin-xum1541 install-plugin-xu1541
else
PLUGINS=plugin-xum1541 plugin-xu1541 plugin-xa1541
INSTALL_PLUGINS=install-plugin-xum1541 install-plugin-xu1541 install-plugin-xa1541
endif
endif

.PHONY: all opencbm clean mrproper dist doc install install-doc uninstall dev install-files install-files-doc all-doc plugin-xum1541 plugin-xu1541 plugin-xa1541 plugin install-plugin install-plugin-xum1541 install-plugin-xu1541 install-plugin-xa1541

all: opencbm plugin

all-doc: all doc

opencbm:
	@for subdir in $(SUBDIRS); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile all || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile all || exit 1) || true; \
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
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1) || true; \
	done

mrproper:
	@for subdir in $(SUBDIRS_ALL_NON_OPTIONAL); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1) || true; \
	done

dist: all-doc clean
	rm -f build-stamp configure-stamp
	tar czvf ../`basename \`pwd\``.tar.gz --exclude=.git --exclude=debian -C .. `basename \`pwd\``

install-files: all
	mkdir -p -m 755 $(DESTDIR)$(BINDIR) $(DESTDIR)$(LIBDIR) $(DESTDIR)$(MANDIR) $(DESTDIR)$(INCDIR) $(DESTDIR)$(INFODIR)
ifeq "$(OS)" "Linux"
	mkdir -p -m 755 $(DESTDIR)$(MODDIR)
endif 
	@for subdir in $(SUBDIRS); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1 ) || true; \
	done

install: install-files

install-all: install install-plugin

install-plugin-xu1541: plugin-xu1541
	@for subdir in $(SUBDIRS_PLUGIN_XU1541); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile install || exit 1; \
	done

install-plugin-xum1541: plugin-xum1541
	@for subdir in $(SUBDIRS_PLUGIN_XUM1541); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile install || exit 1; \
	done

install-plugin-xa1541: plugin-xa1541
	@for subdir in $(SUBDIRS_PLUGIN_XA1541); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile install || exit 1; \
	done

install-plugin: $(INSTALL_PLUGINS)

install-files-doc: install-files all-doc
	@for subdir in $(SUBDIRS_DOC); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done

install-doc: install-files-doc install
	@for subdir in $(SUBDIRS_DOC); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done

plugin-xu1541: opencbm
	@for subdir in $(SUBDIRS_PLUGIN_XU1541); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile || exit 1; \
	done

plugin-xum1541: opencbm
	@for subdir in $(SUBDIRS_PLUGIN_XUM1541); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile || exit 1; \
	done

plugin-xa1541: opencbm
	@for subdir in $(SUBDIRS_PLUGIN_XA1541); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile || exit 1; \
	done

plugin: $(PLUGINS)

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
