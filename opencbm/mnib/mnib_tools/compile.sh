#! /bin/sh
gcc -o n2d n2d.c ../gcr.c ../prot.c -Wall -I..
gcc -o n2g n2g.c ../gcr.c ../prot.c -Wall -I..
#gcc -o g2d g2d.c ../gcr.c ../prot.c -Wall -I..