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

SUBDIRS_OPTIONAL = addon nibtools mnib36 cbmrpm41 cbmlinetester


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

CREATE_TARGET = $(patsubst %,BUILDSYSTEM.%,$(1:=.$2))
CREATE_TARGETS = $(patsubst %,BUILDSYSTEM.%,$(foreach base, $2, $(1:=.$(base))))
GET_TARGET_DIR = $(patsubst BUILDSYSTEM.%,%,$(basename $@))
GET_TARGET =$(patsubst .%,%,$(suffix $@))


all: opencbm plugin

all-doc: all doc

.PHONY: $(call CREATE_TARGETS,$(SUBDIRS_ALL_NON_OPTIONAL),all clean mrproper install install-files install-files-doc install-doc uninstall)
$(call CREATE_TARGETS,$(SUBDIRS_ALL_NON_OPTIONAL),all clean mrproper install install-files install-files-doc install-doc uninstall)::
	$(MAKE) -C $(call GET_TARGET_DIR,$@) -f LINUX/Makefile $(call GET_TARGET,$@)

.PHONY: $(call CREATE_TARGETS,$(SUBDIRS_OPTIONAL),all clean mrproper install install-files install-files-doc install-doc uninstall)
$(call CREATE_TARGETS,$(SUBDIRS_OPTIONAL),all clean mrproper install install-files install-files-doc install-doc uninstall)::
	test ! -e $(call GET_TARGET_DIR,$@)/LINUX/Makefile || $(MAKE) -C $(call GET_TARGET_DIR,$@) -f LINUX/Makefile $(call GET_TARGET,$@)

clean:  $(call CREATE_TARGETS,$(SUBDIRS_ALL_NON_OPTIONAL) $(SUBDIRS_OPTIONAL),clean)

mrproper:  $(call CREATE_TARGETS,$(SUBDIRS_ALL_NON_OPTIONAL) $(SUBDIRS_OPTIONAL),mrproper)

opencbm: $(call CREATE_TARGET,$(SUBDIRS) $(SUBDIRS_OPTIONAL),all)



doc: $(call CREATE_TARGETS,$(SUBDIRS_DOC),all)


dist: all-doc clean
	rm -f build-stamp configure-stamp
	tar czvf ../`basename \`pwd\``.tar.gz --exclude=.git --exclude=debian -C .. `basename \`pwd\``

install-files: $(call CREATE_TARGET,$(SUBDIRS) $(SUBDIRS_OPTIONAL),install-files)
$(call CREATE_TARGET,$(SUBDIRS) $(SUBDIRS_OPTIONAL),install-files):: install-files.prepare

install-files.prepare: opencbm
	mkdir -p -m 755 $(DESTDIR)$(BINDIR) $(DESTDIR)$(LIBDIR) $(DESTDIR)$(MANDIR) $(DESTDIR)$(INCDIR) $(DESTDIR)$(INFODIR)
ifeq "$(OS)" "Linux"
	mkdir -p -m 755 $(DESTDIR)$(MODDIR)
endif 

install: install-files

install-all: install install-plugin

install-plugin-xu1541: $(call CREATE_TARGET,$(SUBDIRS_PLUGIN_XU1541),install)

$(call CREATE_TARGET,$(SUBDIRS_PLUGIN_XU1541),install):: plugin-xu1541

install-plugin-xum1541: $(call CREATE_TARGET,$(SUBDIRS_PLUGIN_XUM1541),install)

$(call CREATE_TARGET,$(SUBDIRS_PLUGIN_XUM1541),install):: plugin-xum1541

install-plugin-xa1541: $(call CREATE_TARGET,$(SUBDIRS_PLUGIN_XA1541),install)

$(call CREATE_TARGET,$(SUBDIRS_PLUGIN_XA1541),install):: plugin-xa1541


install-plugin: $(INSTALL_PLUGINS)

install-files-doc: $(call CREATE_TARGET,$(SUBDIRS_DOC),install-files-doc)
$(call CREATE_TARGET,$(SUBDIRS_DOC),install-files-doc):: install-files all-doc

install-doc: $(call CREATE_TARGET,$(SUBDIRS_DOC),install-doc)
$(call CREATE_TARGET,$(SUBDIRS_DOC),install-doc):: install-files-doc install


plugin-xu1541: $(call CREATE_TARGET,$(SUBDIRS_PLUGIN_XU1541),all)

$(call CREATE_TARGET,$(SUBDIRS_PLUGIN_XU1541),all):: opencbm

plugin-xum1541: $(call CREATE_TARGET,$(SUBDIRS_PLUGIN_XUM1541),all)

$(call CREATE_TARGET,$(SUBDIRS_PLUGIN_XUM1541),all):: opencbm

plugin-xa1541: $(call CREATE_TARGET,$(SUBDIRS_PLUGIN_XA1541),all)

$(call CREATE_TARGET,$(SUBDIRS_PLUGIN_XA1541),all):: opencbm

plugin: $(PLUGINS)

uninstall: $(call CREATE_TARGET,$(SUBDIRS_ALL_NON_OPTIONAL) $(SUBDIRS_ALL_OPTIONAL),uninstall)

dev:
	mkdir -p -m 755 `dirname $(CBMDEV)`
	rm -f $(CBMDEV)
	mknod -m $(CBMPERM) $(CBMDEV) c $(DEVMAJOR) $(DEVMINOR)
