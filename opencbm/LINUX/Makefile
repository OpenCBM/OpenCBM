# $Id: Makefile,v 1.6.2.3 2007-11-04 11:05:39 strik Exp $

include LINUX/config.make

CBMDEV   = /dev/cbm
CBMPERM  = 666
DEVMAJOR = 10
DEVMINOR = 177
SUBDIRS  = sys/linux include arch/$(ARCH) lib compat \
           cbmctrl cbmformat cbmforng d64copy cbmcopy \
           demo/flash demo/morse demo/rpm1541 docs

SUBDIRS_OPTIONAL = mnib36

.PHONY: all clean mrproper dist install uninstall dev install-files

all: 
	@for subdir in $(SUBDIRS); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

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

uninstall: 
	@for subdir in $(SUBDIRS); do \
	  $(MAKE) -C $$subdir -f LINUX/Makefile $@ || exit 1; \
	done
	@for subdir in $(SUBDIRS_OPTIONAL); do \
	  (test -e $$subdir/LINUX/Makefile && $(MAKE) -C $$subdir -f LINUX/Makefile $@; ) || true; \
	done

dev:
	mkdir -p -m 755 `dirname $(CBMDEV)`
	rm -f $(CBMDEV)
	mknod -m $(CBMPERM) $(CBMDEV) c $(DEVMAJOR) $(DEVMINOR)

showvars:
	echo CBMDEV=$(CBMDEV)
	echo CBMPERM=$(CBMPERM)
	echo DEVMAJOR=$(DEVMAJOR)
	echo SUBDIRS=$(SUBDIRS)
	echo SUBDIRS_OPTIONAL=$(SUBDIRS_OPTIONAL)
	echo
	echo XASS=$(XASS)
	echo PREFIX=$(PREFIX)
	echo BINDIR=$(BINDIR)
	echo LIBDIR=$(LIBDIR)
	echo MANDIR=$(MANDIR)
	echo INFODIR$(INFODIR)
	echo INCDIR=$(INCDIR)
	echo MODDIR=$(MODDIR)
	echo UDEV_RULES=$(UDEV_RULES)

	echo ARCH=$(ARCH)
	echo CFLAGS=$(CFLAGS)
	echo ALL_CFLAGS=$(ALL_CFLAGS)
	echo LIB_CFLAGS=$(LIB_CFLAGS)
	echo SHLIB_CFLAGS=$(SHLIB_CFLAGS)
	echo LINK_FLAGS=$(LINK_FLAGS)
	echo SONAME=$(SONAME)
	echo CC=$(CC)
	echo AR=$(AR)
	echo LDCONFIG=$(LDCONFIG)
	echo KERNEL_SOURCE=$(KERNEL_SOURCE)

	echo KERNEL_INCLUDE_CONFIG=$(KERNEL_INCLUDE_CONFIG)

	echo KERNEL_DEFINE=$(KERNEL_DEFINE)

	echo KERNEL_FLAGS=$(KERNEL_FLAGS)

	echo MAJ=$(MAJ)
	echo MIN=$(MIN)
	echo REL=$(REL)

	echo VERSION=$(VERSION)

	echo XA=$(XA)

	echo CL65=$(CL65)
	echo LD65=$(LD65)
	echo CA65_FLAGS=$(CA65_FLAGS)

	echo
	echo EXTRA:
	echo MODDIR ALL = `for d in /lib/modules/\`uname -r\`/{kernel/drivers/char,extra,misc}; do test -d $$d && echo $$d; done`
	echo KERNEL_SOURCE ALL = ${shell for d in {/lib/modules/`uname -r`/build,/usr/src/linux}; do test -d $$d && echo $$d; done}

	echo KERNEL_INCLUDE_CONFIG ALL = ${shell for c in ${KERNEL_SOURCE}/include/linux/autoconf.h ${KERNEL_SOURCE}/include/linux/config.h; do test -f $$c && echo $$c; done}


showvars-file:
	make -f LINUX/Makefile showvars|grep -v "^echo" > showvars.txt
