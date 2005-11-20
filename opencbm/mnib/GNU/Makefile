#
# Makefile for building mnib, n2g, n2d, pshift, and pscan
#
# Build targets are:  dos(djgpp) win32(cygwin) linux
#

# If building for DOS, change this to the path to your DJGPP install
DJGPP_PATH=	c:\djgpp

# If building for Linux or Windows, change this to the base path to your
# opencbm includes and libraries
CBM_LNX_PATH=	/usr/local
#CBM_WIN_PATH=	c:\cbm4win
CBM_WIN_PATH=	../

# Default compile flags.  Be careful with optimization (-O2)
CFLAGS=		${WARNS}

# No user-configurable parts below

usage:
	@echo Please specify a target: dos win32 linux clean distclean

# Arch-specific targets
dos:
	${MAKE} DJGPP="${DJGPP_PATH}\djgpp.env" \
		CC="${DJGPP_PATH}\bin\gcc" \
		ARCH_OBJ="cbm.o kernel.o mnib_rt.o" \
		CFLAGS="-I include/DOS/ $(CFLAGS)" \
		-f GNU/Makefile \
		buildall

linux:
	${MAKE} CFLAGS="-I include/LINUX/ -I ${CBM_LNX_PATH}/include ${CFLAGS}" \
		LDFLAGS="-L${CBM_LNX_PATH}/lib -lopencbm" \
		-f GNU/Makefile \
		buildall

win32:
	${MAKE} CFLAGS="-I include/WINDOWS/ -I ${CBM_WIN_PATH}/include -D WIN32 -mno-cygwin ${CFLAGS}" \
		LDFLAGS="-mno-cygwin -L${CBM_WIN_PATH}/bin/i386/ -lopencbm" \
		-f GNU/Makefile \
		buildall

# Warning level.  Don't reduce, fix your new code instead.
WARNS=	-Werror -W -Wall -Wstrict-prototypes -Wmissing-prototypes \
	-Wpointer-arith -Wno-unused-parameter

# Common objects
OBJ=gcr.o prot.o

# Objects for just mnib
MNIB_OBJ=mnib.o read.o write.o

# All programs to build
PROG=mnib n2d n2g pshift pscan

buildall: ${PROG}

mnib: ${OBJ} ${MNIB_OBJ} ${ARCH_OBJ}
	${CC} -o mnib ${OBJ} ${LDFLAGS} ${MNIB_OBJ} ${ARCH_OBJ}

n2d: ${OBJ} n2d.o
	${CC} -o n2d n2d.o ${OBJ}

n2g: ${OBJ} n2g.o
	${CC} -o n2g n2g.o ${OBJ}

pshift:	${OBJ} pshift.o
	${CC} -o pshift pshift.o ${OBJ}

pscan:	${OBJ} pscan.o
	${CC} -o pscan pscan.o ${OBJ} 

clean:
	${RM} *.o

distclean: clean
	${RM} ${PROG} *.exe
