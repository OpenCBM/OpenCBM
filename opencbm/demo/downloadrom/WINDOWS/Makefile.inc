a65:

..\downloadrom.c: ..\downloadrom.inc

..\downloadrom.inc: ..\downloadrom.a65

.SUFFIXES: .a65

{..\}.a65{..\}.inc:
    ..\..\..\WINDOWS\buildoneinc ..\..\.. $?
