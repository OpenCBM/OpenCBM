#!/bin/bash

# $Id: do_1571_repeat_test.sh,v 1.4 2006-04-21 15:30:13 wmsr Exp $

# Before starting this script, do a:
#
#	d64copy -2 tstimg_rcmp_1571_-ts2.d71 $DRIVENO
#
# not needed anymore, it does it on its own now
#

DRIVETYPE=1571

which c1541 2> /dev/null | fgrep c1541 > /dev/null
if [ 0 -ne $? ]
then
	echo "VICE's c1541 is a prerequisite needed (in the search path)"
	echo "for generating the test set as a D71 image file"
	exit 1
fi

	# create the test fileset which is needed to generate the D71 image
	# ./checkNcreateTestData.sh
./checkNcreateMassData.sh

# Doesn't work currently
# cbmconvert -n -D7o tstimg_rcmp_1571.d71 cbmcopy_files/1[0-9][0-9][0-9]-15817141435.prg


DRIVENO=`cbmctrl detect | fgrep " $DRIVETYPE " | cut -d: -f1 | tail -n 1 | tr -d "[:space:]"`

# Doesn't work currently
# d64copy -2 tstimg_rcmp_1571.d71 $DRIVENO

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
	cbmcopy_rcmp.sh fill $DRIVETYPE -ts2 2>&1 | tee $LOGFILEBASENAME$i.log
done
