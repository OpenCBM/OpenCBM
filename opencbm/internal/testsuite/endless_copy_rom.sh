#! /bin/sh

# $Id: endless_copy_rom.sh,v 1.1 2006-02-12 13:41:04 strik Exp $

clear

FC=`which fc`

ROMSTART=0x8000
ROMLEN=0x8000

[ -f COMPARE.ROM.TEST ] && rm COMPARE.ROM.TEST

echo -e "\nDOWNLOADING initial ROM (for compare reasons)"
date
./cbmctrl download 8 $ROMSTART $ROMLEN COMPARE.ROM.TEST
date

while true; do
	[ -f read.rom.test ] && rm read.rom.test
	echo -e "\nDOWNLOADING"
	date
	./cbmctrl download 8 $ROMSTART $ROMLEN read.rom.test
	date
	$FC /b COMPARE.ROM.TEST read.rom.test
	echo -e "\nUPLOADING (does not make sense, but anyway...)"
	date
	./cbmctrl upload 8 $ROMSTART COMPARE.ROM.TEST
	date
done
