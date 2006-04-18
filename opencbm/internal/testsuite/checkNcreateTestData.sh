#!/bin/bash

# $Id: checkNcreateTestData.sh,v 1.2 2006-04-18 15:21:23 wmsr Exp $

# omitting char that may disturb CVS
# ELIMCHARS="\000\a\b\n\r\\$"

# omitting all control chars as well as the ones disturbig CVS
# ELIMCHARS="[:cntrl:]\\$"

ELIMCHARS='""'
TESTFILEDIR=cbmcopy_files


if ! [ -d $TESTFILEDIR ]
then
    mkdir $TESTFILEDIR
fi

if [ _12 != _`ls $TESTFILEDIR/[0-9][0-9][0-9][0-9]-1581*.prg 2>/dev/null | cat -n | tail -n 1 | cut -f1 | tr -d "[:space:]"` ]
then
	echo "re-creating cbmcopy test fileset for 1581, 1571, 1541 (40 tracks as well as 35)"
	rm -f $TESTFILEDIR/[0-9][0-9][0-9][0-9]-1581*.prg

	echo creating 1581 only file
    tr -d $ELIMCHARS < /dev/random | dd ibs=256 count=1817 of=$TESTFILEDIR/1001-1581-abcdef.prg 2> /dev/null
	echo creating 1571 only file
    tr -d $ELIMCHARS < /dev/random | dd ibs=256  count=574 of=$TESTFILEDIR/1002-158171-cdef.prg 2> /dev/null
	echo creating 1541/40 only file
    tr -d $ELIMCHARS < /dev/random | dd ibs=256   count=84 of=$TESTFILEDIR/1003-158171414-f.prg 2> /dev/null
	echo creating 1541/35 only files
    tr -d $ELIMCHARS < /dev/random | dd ibs=256  count=405 of=$TESTFILEDIR/1004-15817141435.prg 2> /dev/null
    
    for (( i=1, c=1005 ; i<255 ; i*=2, c++ ))
    do
        tr -d $ELIMCHARS < /dev/random | dd ibs=$i count=254 of="$TESTFILEDIR/"$c"-15817141435.prg" 2> /dev/null
    done
fi
