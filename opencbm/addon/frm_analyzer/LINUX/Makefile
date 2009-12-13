# $Id: Makefile,v 1.5 2009-12-13 12:04:18 cnvogelg Exp $

RELATIVEPATH=../../
include ${RELATIVEPATH}LINUX/config.make
CFLAGS     := $(subst ../,../../,$(CFLAGS))
LINK_FLAGS := $(subst ../,../../,$(LINK_FLAGS))

PROG = frm_analyzer
INC  = frm_analyzer.inc

include ${RELATIVEPATH}LINUX/prgrules.make
