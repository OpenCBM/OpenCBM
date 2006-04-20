#!/bin/bash

# $Id: cbmcopy_rcmp.sh,v 1.3 2006-04-20 11:02:34 wmsr Exp $

function error_info {
	echo "cbmcopy_rcmp.sh <testfileset> <drivetype> [<cbmcopy parameters>]" 1>&2
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
	fill)
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


    case $2 in
        1581)
            FILESPEC=`ls cbmcopy_files | egrep "[0-9][0-9][0-9][0-9]-1581.*\.prg" | head -n 1`
            ;;
        1571)
        	cbmctrl command $DRIVENO "U0>M1"
			FILESPEC=`ls cbmcopy_files | egrep "[0-9][0-9][0-9][0-9]-158171.*\.prg" | head -n 1`
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
					FILESPEC=`ls cbmcopy_files | egrep "[0-9][0-9][0-9][0-9]-158171414.*\.prg" | head -n 1`
            		;;
		        664)	# 35 tracks standard format
					FILESPEC=`ls cbmcopy_files | egrep "[0-9][0-9][0-9][0-9]-15817141435\.prg" | head -n 1`
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