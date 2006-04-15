#!/bin/bash

# $Id: cbmcopy_rcmp.sh,v 1.1 2006-04-15 08:56:13 wmsr Exp $

function error_info {
	echo "cbmcopy_fill.sh <drivetype> [<cbmcopy parameters>]" 1>&2
	echo  1>&2
	echo "drivetype: 1581, 1571, 1541"  1>&2
	rm -f shelltst.pid
	exit 1
	}

if [ $# -lt 1 ]
then
	error_info
fi

./checkNcreateTestData.sh


DRIVENO=`cbmctrl detect | fgrep " $1 " | cut -d: -f1 | tail -n 1 | tr -d "[:space:]"`
if [ _ != _$DRIVENO ]
then
	echo $$ > shelltst.pid
    echo using drive $DRIVENO

    case $1 in
        1581)
            FILESPEC=a-1581-890123456.prg
            ;;
        1571)
        	cbmctrl command $DRIVENO "U0>M1"
            FILESPEC=b-158171-0123456.prg
            ;;
        1541)
            BLOCKSFREE=`cbmctrl dir $DRIVENO | \
                        egrep '^[0-9]+ *(.["].+["].*)|(blocks free\.) *.$' | \
                        cut -d" " -f1 | \
                        tr "\r\n" " +" | \
                        sed "s/+$/\n/g" | \
                        bc`

            case $BLOCKSFREE in
		        749)	# 40 tracks format (SpeedDOS and alike)
		            FILESPEC=c-15817141-40-56.prg
            		;;
		        664)	# 35 tracks standard format
		            FILESPEC=d-15817141-40-35.prg
            		;;
		          *)	# unknown 1541 disk format
					echo 1>&2
		           	echo 1541 drivetype disk format unkown with $BLOCKSFREE blocks free 1>&2
					rm -f shelltst.pid
					exit 1
            		;;
            esac
            ;;
           *)
			echo 1>&2
           	echo drivetype unknown 1>&2
			echo 1>&2
			error_info
            ;;
    esac

    shift

	echo beginning transfer
	TMPCBMCOPYDIR=`mktemp -d -p ./ cbmcopy_rcmp.XXXXXXXXXXXX`
	cd $TMPCBMCOPYDIR
	cbmctrl dir $DRIVENO | egrep 'prg *.?$' | cut -d'"' -f2 | xargs cbmcopy -r $* $DRIVENO
	cd ..
	echo transfer ended
	echo beginning comparison
	diff -nqrs --binary -S $FILESPEC cbmcopy_files $TMPCBMCOPYDIR
	echo comparison ended
	echo cleaning up temporary directory
	rm -f $TMPCBMCOPYDIR/*.prg
	rmdir $TMPCBMCOPYDIR
	echo cleaning finished
	rm -f shelltst.pid
else
	echo drivetype not found or not found beeing active on the bus.
fi