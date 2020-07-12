KMOD=	opencbm
SRCS=	opencbm.c device_if.h bus_if.h ppbus_if.h
MAN4=	opencbm.4
LOCALBASE?=	/usr/local
MANDIR?=	${LOCALBASE}/man/man

all:	all-man

afterinstall:	maninstall

.include <bsd.kmod.mk>
.include <bsd.man.mk>
