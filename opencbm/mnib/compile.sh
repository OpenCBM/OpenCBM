#! /bin/sh
gcc -o mnib mnib.c read.c write.c gcr.c prot.c -L/usr/lib -L/usr/local/lib -lopencbm -Wall