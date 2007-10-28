# $Id: config.make,v 1.1.10.1 2007-10-28 10:54:25 strik Exp $

include ../../LINUX/config.make

ALL_CFLAGS := $(subst ../,../../,$(ALL_CFLAGS))
LINK_FLAGS := $(subst ../,../../,$(LINK_FLAGS))
