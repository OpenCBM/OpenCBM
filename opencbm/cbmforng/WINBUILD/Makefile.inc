a65:

..\cbmforng.c: ..\cbmforng.inc

..\cbmforng.inc: ..\cbmforng.a65

..\cbmforng.a65: ..\cbmfmacs.i65 ..\cbmfsubs.i65

.SUFFIXES: .a65

{..\}.a65{..\}.inc:
    ..\..\WINDOWS\buildoneinc ..\.. $?
