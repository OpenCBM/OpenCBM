# $Id: Makefile,v 1.10 2007-04-28 17:18:33 cnvogelg Exp $

include ../LINUX/config.make

CA65_FLAGS += --asm-include-dir ../libd64copy/

CFLAGS += -I ./

.PHONY: all clean mrproper install uninstall install-files

LIBCBMCOPY = ../libcbmcopy
LIBD64COPY = ../libd64copy
LIBARCH    = ../arch/linux/

# specify lib
LIBNAME = libopencbm
SRCS    = cbm.c detect.c detectxp1541.c petscii.c gcr_4b5b.c upload.c \
	  configuration.c LINUX/configuration_name.c \
	  LINUX/getpluginaddress.c \
	  $(foreach t,cbmcopy pp s1 s2, $(LIBCBMCOPY)/$(t).c) \
	  $(foreach t,d64copy fs gcr pp s1 s2 std, $(LIBD64COPY)/$(t).c)
LIBS = $(LIBARCH)/libarch.a -ldl

all: build-lib

clean: clean-lib
	rm -f $(LIBD64COPY)/*.o65 $(LIBCBMCOPY)/*.o65

mrproper: clean
	rm -f $(LIBCBMCOPY)/*.inc $(LIBD64COPY)/*.inc

install-files: install-lib

install: install-files update-libcache

uninstall: uninstall-lib

include ../LINUX/librules.make

### dependencies:

$(LIBCBMCOPY)/cbmcopy.o $(LIBCBMCOPY)/cbmcopy.lo: \
  $(LIBCBMCOPY)/cbmcopy.c ../include/opencbm.h \
  ../include/cbmcopy.h $(LIBCBMCOPY)/cbmcopy_int.h \
  $(LIBCBMCOPY)/turboread1541.inc $(LIBCBMCOPY)/turboread1571.inc \
  $(LIBCBMCOPY)/turboread1581.inc $(LIBCBMCOPY)/turbowrite1541.inc \
  $(LIBCBMCOPY)/turbowrite1571.inc $(LIBCBMCOPY)/turbowrite1581.inc
$(LIBCBMCOPY)/pp.o $(LIBCBMCOPY)/pp.lo: \
  $(LIBCBMCOPY)/pp.c ../include/opencbm.h $(LIBCBMCOPY)/cbmcopy_int.h \
  $(LIBCBMCOPY)/ppr-1541.inc $(LIBCBMCOPY)/ppr-1571.inc \
  $(LIBCBMCOPY)/ppw-1541.inc $(LIBCBMCOPY)/ppw-1571.inc
$(LIBCBMCOPY)/s1.o $(LIBCBMCOPY)/s1.lo: \
  $(LIBCBMCOPY)/s1.c ../include/opencbm.h $(LIBCBMCOPY)/cbmcopy_int.h \
  $(LIBCBMCOPY)/s1r.inc $(LIBCBMCOPY)/s1w.inc $(LIBCBMCOPY)/s1r-1581.inc \
  $(LIBCBMCOPY)/s1w-1581.inc
$(LIBCBMCOPY)/s2.o $(LIBCBMCOPY)/s2.lo: \
  $(LIBCBMCOPY)/s2.c ../include/opencbm.h $(LIBCBMCOPY)/cbmcopy_int.h \
  $(LIBCBMCOPY)/s2r.inc $(LIBCBMCOPY)/s2w.inc $(LIBCBMCOPY)/s2r-1581.inc \
  $(LIBCBMCOPY)/s2w-1581.inc
$(LIBD64COPY)/d64copy.o $(LIBD64COPY)/d64copy.lo: \
  $(LIBD64COPY)/d64copy.c $(LIBD64COPY)/d64copy_int.h \
  ../include/opencbm.h ../include/d64copy.h $(LIBD64COPY)/gcr.h \
  $(LIBD64COPY)/warpread1541.inc $(LIBD64COPY)/warpwrite1541.inc \
  $(LIBD64COPY)/warpread1571.inc $(LIBD64COPY)/warpwrite1571.inc \
  $(LIBD64COPY)/turboread1541.inc $(LIBD64COPY)/turbowrite1541.inc \
  $(LIBD64COPY)/turboread1571.inc $(LIBD64COPY)/turbowrite1571.inc
$(LIBD64COPY)/fs.o $(LIBD64COPY)/fs.lo: \
  $(LIBD64COPY)/fs.c $(LIBD64COPY)/d64copy_int.h ../include/opencbm.h \
  ../include/d64copy.h $(LIBD64COPY)/gcr.h
$(LIBD64COPY)/gcr.o $(LIBD64COPY)/gcr.lo: \
  $(LIBD64COPY)/gcr.c $(LIBD64COPY)/gcr.h
$(LIBD64COPY)/pp.o $(LIBD64COPY)/pp.lo: \
  $(LIBD64COPY)/pp.c ../include/opencbm.h $(LIBD64COPY)/d64copy_int.h \
  ../include/d64copy.h $(LIBD64COPY)/gcr.h $(LIBD64COPY)/pp1541.inc \
  $(LIBD64COPY)/pp1571.inc
$(LIBD64COPY)/s1.o $(LIBD64COPY)/s1.lo: \
  $(LIBD64COPY)/s1.c ../include/opencbm.h $(LIBD64COPY)/d64copy_int.h \
  ../include/d64copy.h $(LIBD64COPY)/gcr.h $(LIBD64COPY)/s1.inc
$(LIBD64COPY)/s2.o $(LIBD64COPY)/s2.lo: \
  $(LIBD64COPY)/s2.c ../include/opencbm.h $(LIBD64COPY)/d64copy_int.h \
  ../include/d64copy.h $(LIBD64COPY)/gcr.h $(LIBD64COPY)/s2.inc
$(LIBD64COPY)/std.o $(LIBD64COPY)/std.lo: \
  $(LIBD64COPY)/std.c ../include/opencbm.h \
  $(LIBD64COPY)/d64copy_int.h ../include/d64copy.h $(LIBD64COPY)/gcr.h
detect.o detect.lo: detect.c ../include/opencbm.h
detectxp1541.o detectxp1541.lo: detectxp1541.c ../include/opencbm.h
petscii.o petscii.lo: petscii.c ../include/opencbm.h
gcr_4b5b.o gcr_4b5b.lo: gcr_4b5b.c ../include/opencbm.h
upload.o upload.lo: upload.c ../include/opencbm.h
cbm.o cbm.lo: cbm.c ../include/opencbm.h ../include/LINUX/cbm_module.h
