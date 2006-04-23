#!/bin/bash

# $Id: checkNcreateMassData.sh,v 1.2 2006-04-23 15:22:10 wmsr Exp $

# omitting char that may disturb CVS
# ELIMCHARS="\000\a\b\n\r\\$"

# omitting all control chars as well as the ones disturbig CVS
# ELIMCHARS="[:cntrl:]\\$"

ELIMCHARS='""'
TESTFILEDIR=cbmcopy_files


function createXblocksFile {
	# Params: <pathNfilename> <numofblocks>

	# this doesn't work, sometimes files get to small
	# COUNT=`echo "  $2 * 254 - 1 " | bc`
	# tr -d $ELIMCHARS < /dev/random | dd bs=$COUNT count=1 > "$1" 2> /dev/null

	COUNT=`echo " ( $2 * 254 - 1 ) / 512 " | bc`
	tr -d $ELIMCHARS < /dev/random | dd bs=512 count=$COUNT  > "$1" 2> /dev/null
	COUNT=`echo " ( $2 * 254 - 1 ) % 512 " | bc`
	tr -d $ELIMCHARS < /dev/random | dd   bs=1 count=$COUNT >> "$1" 2> /dev/null
	}

if ! [ -d $TESTFILEDIR ]
then
    mkdir $TESTFILEDIR
fi

if [ _296 != _`ls $TESTFILEDIR/[0-9][0-9][0-9][0-9]-1581*.prg 2>/dev/null | cat -n | tail -n 1 | cut -f1 | tr -d "[:space:]"` ]
then
	echo "re-creating cbmcopy test fileset for 1581, 1571, 1541 (40 tracks as well as 35)"
	rm -f $TESTFILEDIR/[0-9][0-9][0-9][0-9]-1581*.prg

	# Availability: DIRENTRIES BLOCKS     BLOCKS/DIRENTRIES
	# -----------------------------------------------
	# D81                  296   3160 = ((83+10+27+75)*8 + 5 + 11 + (22+77)*16) BLOCKS
	#                                 ==>   3160/296
	#
	# D71                  144   1328 = ((83+10+27)   *8 + 5 + 11 +  22    *16) BLOCKS
	#                                 ==>   1328/144
	#
	# D64-40               144    749 = ((83+10)      *8 + 5) BLOCKS
	#                                 ==>    749/94
	#
	# D64-35               144    664 = ( 83          *8) BLOCKS
	#                                 ==>    664/83


	# Precreate a 16 blocks and a 8 blocks file
	# 1581 fill names: 1001-1581-abcdef
	echo creating 1581 only files
	SIXTEENBLOCKS=$TESTFILEDIR/1001-1581-abcdef.prg
	EIGHT__BLOCKS=$TESTFILEDIR/1002-1581-abcdef.prg
	createXblocksFile $SIXTEENBLOCKS 16
	createXblocksFile $EIGHT__BLOCKS  8

		# 74 *  8 blocks for 1581 fillup
	for (( i=1003; i<1077; i++ )) ; do cp $EIGHT__BLOCKS $TESTFILEDIR/$i-1581-abcdef.prg ; done
		# 77 * 16 blocks for 1581 fillup
	for (( i=1077; i<1153; i++ )) ; do cp $SIXTEENBLOCKS $TESTFILEDIR/$i-1581-abcdef.prg ; done

	# 1571 fill names: 1153-158171-cdef
	echo creating 1571 only files
	createXblocksFile $TESTFILEDIR/1153-158171-cdef.prg 11
		# 27 *  8 blocks for 1571 fillup
	for (( i=1154; i<1181; i++ )) ; do cp $EIGHT__BLOCKS $TESTFILEDIR/$i-158171-cdef.prg ; done
		# 22 * 16 blocks for 1571 fillup
	for (( i=1181; i<1203; i++ )) ; do cp $SIXTEENBLOCKS $TESTFILEDIR/$i-158171-cdef.prg ; done


	# 1541 40 tracks fill names: 1001-158171414-f
	echo creating 1541/40 only files
	createXblocksFile $TESTFILEDIR/1203-158171414-f.prg 5
		# 10 *  8 blocks for 1541/40 fillup
	for (( i=1204; i<1214; i++ )) ; do cp $EIGHT__BLOCKS $TESTFILEDIR/$i-158171414-f.prg ; done

	# 1541 35 tracks fill names: 1001-15817141435
	echo creating 1541/35 only files
		# 83 *  8 blocks for 1541/35 fillup
	for (( i=1214; i<1297; i++ )) ; do cp $EIGHT__BLOCKS $TESTFILEDIR/$i-15817141435.prg ; done
fi
