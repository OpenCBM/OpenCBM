# $Id: config.make,v 1.2 2007-04-29 10:46:22 strik Exp $

RELATIVEPATH=../../
include ${RELATIVEPATH}LINUX/config.make

CFLAGS     := $(subst ../,../../,$(CFLAGS))
LINK_FLAGS := $(subst ../,../../,$(LINK_FLAGS))
