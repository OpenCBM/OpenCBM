# $Id: librules.make,v 1.1 2007-04-28 17:18:33 cnvogelg Exp $
# common rules for shared lib creation
#

# you need to define the following variables to use the rules:
# LIBNAME   name of library
# SRCS      source files for library
# LIBS      link libs for library (optional)

#
# library naming definitions
#
SHLIB   = $(LIBNAME).$(SHLIB_EXT)
SHLIBV  = $(SHLIB).$(MAJ)
SHLIBV3 = $(SHLIBV).$(MIN).$(REL)
LIB     = $(LIBNAME).a

ifeq "$(OS)" "Darwin"
SHLIBV  = $(LIBNAME).$(MAJ).$(SHLIB_EXT)
SHLIBV3 = $(LIBNAME).$(MAJ).$(MIN).$(REL).$(SHLIB_EXT)
endif

# define object files
OBJS    = $(SRCS:.c=.o)
SHOBJS  = $(SRCS:.c=.lo)

.PHONY: build-lib clean-lib install-lib uninstall-lib update-libcache
.PHONY: install-plugin uninstall-plugin

# build lib rule
build-lib: $(LIB) $(SHLIB) $(SHLIBV) $(SHLIBV3)

# clean up lib files
clean-lib:
	rm -f $(OBJS) $(SHOBJS) $(LIB) $(SHLIB) $(SHLIBV) $(SHLIBV3)

# install lib
install-lib:
	install -d $(LIBDIR)
	install -m 755 $(SHLIBV3) $(LIBDIR)
	install -m 644 $(LIB) $(LIBDIR)
	cd $(LIBDIR) && ln -sf $(SHLIBV3) $(SHLIBV); ln -sf $(SHLIBV) $(SHLIB)
	
# update lib
update-libcache:
ifeq "$(OS)" "Linux"
	ldconfig -n $(LIBDIR)
endif

# uninstall lib
uninstall-lib:
	cd $(LIBDIR) && rm -f $(LIB) $(SHLIB) $(SHLIBV) $(SHLIBV3)

# install plugin
install-plugin:
	install -d $(PLUGINDIR)
	install -m 755 $(SHLIBV3) $(PLUGINDIR)
	install -m 644 $(LIB) $(PLUGINDIR)
	cd $(PLUGINDIR) && ln -sf $(SHLIBV3) $(SHLIBV); ln -sf $(SHLIBV) $(SHLIB)

# uninstall plugin
uninstall-plugin:
	-cd $(PLUGINDIR) && rm -f $(LIB) $(SHLIB) $(SHLIBV) $(SHLIBV3)
	-rmdir -p $(PLUGINDIR)

# compile rule
.c.o:
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

# link lib
$(SHLIB): $(SHLIBV)
	ln -sf $< $@

$(SHLIBV): $(SHLIBV3)
	ln -sf $< $@

# build shared lib
$(SHLIBV3): $(SHOBJS)
	$(CC) $(LDFLAGS) $(SHLIB_SWITCH) -o $@ $(SONAME)$(SHLIBV) $(SHOBJS) $(LIBS)

# build static lib
$(LIB): $(OBJS)
	$(AR) r $@ $(OBJS)
