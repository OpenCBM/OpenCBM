#!/bin/bash

DEVICE=8

if [ ! -z "$1" ]; then
	COUNT=$1
else
	COUNT=10
fi

clear

if [ 1 == 0 ]; then
	ROMFILE_COMPARE=rom1541p.bin

	ROMFILE_TEMPLATE=rom1541p-auto

	ROM_START=0xc000
	ROM_LENGTH=0x4000
fi

if [ 1 == 0 ]; then
	ROMFILE_COMPARE=rom2031lp.bin

	ROMFILE_TEMPLATE=rom2031lp-auto

	ROM_START=0xc000
	ROM_LENGTH=0x4000
fi

if [ 1 == 0 ]; then
	ROMFILE_COMPARE=rom1001.bin

	ROMFILE_TEMPLATE=rom1001-auto

	ROM_START=0xc000
	ROM_LENGTH=0x4000
fi

if [ 1 == 0 ]; then
	ROMFILE_COMPARE=rom1570.bin

	ROMFILE_TEMPLATE=rom1570-auto

	ROM_START=0x8000
	ROM_LENGTH=0x8000
fi

if [ 1 == 1 ]; then
	ROMFILE_COMPARE=rom1541-05.bin

	ROMFILE_TEMPLATE=rom1541-auto

	ROM_START=0xc000
	ROM_LENGTH=0x4000
fi

DIR_GENERATED=GENERATED
DIR_GENERATED_TXT=$DIR_GENERATED/TXT

[ -d $DIR_GENERATED ]     || mkdir $DIR_GENERATED
[ -d $DIR_GENERATED_TXT ] || mkdir $DIR_GENERATED_TXT

LAST_EXISTING=0

for i in $DIR_GENERATED/$ROMFILE_TEMPLATE-*.*; do
	if [ -f "$i" ]; then
		NUMBER=`echo $i|sed -e "s/^$DIR_GENERATED\/$ROMFILE_TEMPLATE-\([0-9]*\).*$/\1/"`
		[ ! -z "$NUMBER" ] && [ $NUMBER -gt $LAST_EXISTING ] && LAST_EXISTING=$NUMBER
	fi
done

FIRST=$(($LAST_EXISTING + 1))
LAST=$(($FIRST + $COUNT))

echo Running from $FIRST to $(($LAST-1))

cbmctrl reset
sleep 2

if [ ! -e $ROMFILE_COMPARE ]; then
	echo The ROM file to compare with '$ROMFILE_COMPARE' does not exist.
	echo Should I create it from the ROM contents?
	read
	case "${REPLY,,}" in
		"y"|"yes")
			echo Creating file $ROMFILE_COMPARE
			timeout 2m cbmctrl download $DEVICE $ROM_START $ROM_LENGTH $ROMFILE_COMPARE; RET=$?
			echo
			if [ $RET -gt 0 ]; then
				echo Failed to create $ROMFILE_COMPARE, aborting.
				exit 1
			fi
			;;
		*)
			echo Without the ROM file, I cannot proceed. Aborting.
			exit 2
			;;
	esac
fi

xxd -g1 < $ROMFILE_COMPARE > GENERATED/TXT/$ROMFILE_COMPARE.c.txt

for ((A=$FIRST; A<$LAST; A++)); do
	echo
	echo -n "Run $A: "
	ROMFILE_TO_CREATE=GENERATED/$ROMFILE_TEMPLATE-$A
	date
	date > $ROMFILE_TO_CREATE.time 2>&1
	timeout 2m time -a -o $ROMFILE_TO_CREATE.time cbmctrl download $DEVICE $ROM_START $ROM_LENGTH $ROMFILE_TO_CREATE.bin; RET=$?
	[ $RET -gt 0 ] && echo $RET > $ROMFILE_TO_CREATE.wd
	date >> $ROMFILE_TO_CREATE.time 2>&1
	diff $ROMFILE_TO_CREATE.bin $ROMFILE_COMPARE > /dev/null 2>&1; RET=$?
	if [ $RET -gt 0 ]; then
		echo "  FAILED  "
		xxd -g1 < $ROMFILE_TO_CREATE.bin > GENERATED/TXT/$ROMFILE_TEMPLATE-$A.txt
		diff -u GENERATED/TXT/$ROMFILE_COMPARE.c.txt GENERATED/TXT/$ROMFILE_TEMPLATE-$A.txt > GENERATED/TXT/$ROMFILE_TEMPLATE-$A.diff
	fi
done
