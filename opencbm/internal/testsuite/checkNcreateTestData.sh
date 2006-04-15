#!/bin/bash

# $Id: checkNcreateTestData.sh,v 1.1 2006-04-15 08:56:13 wmsr Exp $

# omitting char that may disturb CVS
# ELIMCHARS="\000\a\b\n\r\\$"

# omitting all control chars as well as the ones disturbig CVS
# ELIMCHARS="[:cntrl:]\\$"

ELIMCHARS='""'


if ! [ -d cbmcopy_files ]
then
    mkdir cbmcopy_files
fi

if [ _12 != _`ls cbmcopy_files/?-1581*-*.prg 2>/dev/null | cat -n | cut -f1 | tail -n 1 | tr -d "[:space:]"` ]
then
	echo "re-creating cbmcopy test fileset for 1581, 1571, 1541 (40 tracks as well as 35)"
	rm -f cbmcopy_files/?-1581*-*.prg

    tr -d $ELIMCHARS < /dev/random | dd ibs=256 count=1817 of=cbmcopy_files/a-1581-890123456.prg 2> /dev/null
    tr -d $ELIMCHARS < /dev/random | dd ibs=256  count=574 of=cbmcopy_files/b-158171-0123456.prg 2> /dev/null
    tr -d $ELIMCHARS < /dev/random | dd ibs=256   count=84 of=cbmcopy_files/c-15817141-40-56.prg 2> /dev/null
    tr -d $ELIMCHARS < /dev/random | dd ibs=256  count=405 of=cbmcopy_files/d-15817141-40-35.prg 2> /dev/null
    
    # for (( i=128, c=1 ; i>0 ; i=i/2, c=c+1 ))
    for (( i=1, c=1 ; i<255 ; i=i*2, c=c+1 ))
    do
        # make a character in the range "e" (octal: 101) to
        # "l" (octal: 110) out of the counter $c
        CCHAR=`echo "obase=8; 100+$c" | bc`
        CCHAR=`echo -e \\\\$CCHAR`
    
        tr -d $ELIMCHARS < /dev/random | dd ibs=$i count=254 of="cbmcopy_files/"$CCHAR"-15817141-40-35.prg" 2> /dev/null
    done
fi
