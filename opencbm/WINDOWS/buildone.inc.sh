#! /bin/sh

CC65PATH=/usr/local/bin/

CL65=$CC65PATH\cl65
LD65=$CC65PATH\ld65
OD=/usr/bin/od
SED=/usr/bin/sed
RM=/usr/bin/rm

CA65_FLAGS="--feature labels_without_colons --feature pc_assignment --feature loose_char_term --asm-include-dir .."

funcbuildinc()
{
WHICHFILE=`echo $1|$SED 's/\.\(a\|A\)65//'`

$CL65 -c $CA65_FLAGS -o $WHICHFILE.tmp $WHICHFILE.a65
test -s $WHICHFILE.tmp && $LD65 --target none -o $WHICHFILE.o65 $WHICHFILE.tmp && $RM $WHICHFILE.tmp
test -s $WHICHFILE.o65 && $OD -w8 -txC -v -An $WHICHFILE.o65|$SED 's/\([0-9a-f]\{2\}\) */0x\1,/g; $s/,$//' > $WHICHFILE.inc && $RM $WHICHFILE.o65
}

funcbuildvice()
{
WHICHFILE=`echo $1|$SED 's/\.\(a\|A\)65//'`

$CL65 -c $CA65_FLAGS -g -l -o $WHICHFILE.tmp $WHICHFILE.a65
test -s $WHICHFILE.tmp && $LD65 --target none -Ln $WHICHFILE.sym65 -o $WHICHFILE.o65 $WHICHFILE.tmp && $RM $WHICHFILE.tmp
#test -s $WHICHFILE.o65 && $OD -w8 -txC -v -An $WHICHFILE.o65|$SED 's/\([0-9a-f]\{2\}\) */0x\1,/g; $s/,$//' > $WHICHFILE.inc && $RM $WHICHFILE.o65
}
