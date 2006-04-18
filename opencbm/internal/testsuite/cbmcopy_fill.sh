#!/bin/bash

# $Id: cbmcopy_fill.sh,v 1.2 2006-04-18 15:21:23 wmsr Exp $

function error_info {
	echo "cbmcopy_fill.sh <testfileset> <drivetype> [<cbmcopy parameters>]" 1>&2
	echo  1>&2
	echo "testfileset: mass | fill"  1>&2
	echo  1>&2
	echo "drivetype:   1581, 1571, 1541"  1>&2
	rm -f shelltst.pid
	exit 1
	}

if [ $# -lt 2 ]
then
	error_info
fi

case $1 in
	mass)
		checkNcreateMassData.sh
		;;
	file)
		./checkNcreateTestData.sh
		;;
	   *)
		echo 1>&2
        echo testfileset unknown 1>&2
		echo 1>&2
		error_info
        ;;
esac

DRIVENO=`cbmctrl detect | fgrep " $2 " | cut -d: -f1 | tail -n 1 | tr -d "[:space:]"`
if [ _ != _$DRIVENO ]
then
	echo $$ > shelltst.pid
    echo using drive $DRIVENO
    echo short reformatting disk

    case $2 in
        1581)
            cbmctrl command $DRIVENO "N:SHORT1581FORMAT"
            FILESPEC=cbmcopy_files/[0-9][0-9][0-9][0-9]-1581*.prg
            ;;
        1571)
        	cbmctrl command $DRIVENO "U0>M1"
            cbmctrl command $DRIVENO "N:SHORT1571FORMAT"
			FILESPEC=cbmcopy_files/[0-9][0-9][0-9][0-9]-158171*.prg
            ;;
        1541)
            cbmctrl command $DRIVENO "N:SHORT1541FORMAT"
            BLOCKSFREE=`cbmctrl dir $DRIVENO | egrep "^[0-9]+ blocks free[.] *.$" | cut -d" " -f1`
            case $BLOCKSFREE in
		        749)	# 40 tracks format (SpeedDOS and alike)
					FILESPEC=cbmcopy_files/[0-9][0-9][0-9][0-9]-158171414*.prg
            		;;
		        664)	# 35 tracks standard format
					FILESPEC=cbmcopy_files/[0-9][0-9][0-9][0-9]-15817141435.prg
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
    shift
    
    echo beginning transfer
    cbmcopy -R -w $* $DRIVENO $FILESPEC
    rm -f shelltst.pid
    echo transfer ended.
else
	echo drivetype not found or not found beeing active on the bus.
fi