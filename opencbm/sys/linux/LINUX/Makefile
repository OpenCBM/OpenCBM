# $Id: Makefile,v 1.4.2.1 2007-03-13 09:57:45 strik Exp $

ifneq ($(KERNELRELEASE),)
EXTRA_CFLAGS := $(CBM4LINUX_KERNEL_FLAGS) -I$(here)/../../include -I$(here)/../../include/LINUX
obj-m 	     := cbm.o
cbm-objs     := cbm_module.o

# for 2.2/2.4:
modules:
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c -o $(obj-m) cbm_module.c

else

include ../../LINUX/config.make

all: cbm.o

.PHONY: all clean mrproper install uninstall install-files
clean:
	rm -f cbm.mod.c *.o *.ko .cbm* Modules.symvers
	rm -fr .tmp_versions
	rm -f Makefile
	rm -f Module.symvers

mrproper: clean

install-files: all
	install -m 644 `[ -f cbm.ko ] && echo cbm.ko || echo cbm.o` $(MODDIR)

install: install-files
	-depmod -a

uninstall:
	if [ ! -z "`lsmod|grep '^cbm '`" ]; then rmmod cbm; fi
	rm -f $(MODDIR)/cbm.o

cbm.o: cbm_module.c
	-ln -s LINUX/Makefile Makefile
	$(MAKE) -C $(KERNEL_SOURCE) here=`pwd` CBM4LINUX_KERNEL_FLAGS=$(KERNEL_FLAGS) SUBDIRS=`pwd` modules
	-rm -f Makefile

endif
