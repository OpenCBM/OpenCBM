a65: ..\pp1541.inc ..\pp1571.inc ..\s1.inc ..\s2.inc ..\turboread1541.inc ..\turboread1571.inc ..\turbowrite1541.inc ..\turbowrite1571.inc ..\warpread1541.inc ..\warpread1571.inc ..\warpwrite1541.inc ..\warpwrite1571.inc

.SUFFIXES: .a65

{..\}.a65{..\}.inc:
    ..\..\buildoneinc ..\.. $?
