#!/bin/bash

# $Id: do_1571_repeat_test.sh,v 1.2 2006-04-18 20:45:37 wmsr Exp $

# Before starting this script, do a:
#
#	d64copy -2 tstimg_rcmp_1571_-ts2.d71 $DRIVENO

function numargs {
	if [ -e $1 ]
	then
		return $#
	else
		return 0
	fi
	}

OWNDIRNAME=`pwd | tr "/" "\n" | tail -n 1`
for (( i=99 ; i>10 ; i=i-1 ))
do
	numargs "rcmp_"$OWNDIRNAME"_"$i"_1"[0-9][0-9][0-9].log
	if [ 0 -ne $? ] ; then break ; fi
	
	LOGFILEBASENAME=rcmp_"$OWNDIRNAME"_"$i"_
done
echo Logfile base name is: "$LOGFILEBASENAME"

for (( i=1001 ; i<=9999 ; i=i+1 ))
do
	cbmcopy_rcmp.sh 1571 -ts2 2>&1 | tee $LOGFILEBASENAME$i.log
done
