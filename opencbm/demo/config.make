# $Id: config.make,v 1.1 2005-03-02 18:17:18 strik Exp $

include ../../LINUX/config.make

CFLAGS     := $(subst ../,../../,$(CFLAGS))
LINK_FLAGS := $(subst ../,../../,$(LINK_FLAGS))
