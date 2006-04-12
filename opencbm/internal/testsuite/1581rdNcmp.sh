#!/bin/bash

# $Id: 1581rdNcmp.sh,v 1.1 2006-04-12 23:38:20 wmsr Exp $

DRIVENO=`cbmctrl detect | fgrep 1581 | cut -d: -f1`

echo $$ > shelltst.pid
echo using drive $DRIVENO
echo beginning transfer
TMP1581DIR=`mktemp -d -p ./ 1581rdNcmp.XXXXXXXXXXXX`
cd $TMP1581DIR
cbmctrl dir $DRIVENO | /bin/egrep 'prg *.?$' | /bin/cut -d'"' -f2 | /bin/xargs cbmcopy -r $* $DRIVENO
cd ..
echo transfer ended
echo beginning comparison
diff -nqrs 1581filldata $TMP1581DIR
echo comparison ended
echo cleaning up temporary directory
rm -f $TMP1581DIR/*.prg
rmdir $TMP1581DIR
echo cleaning finished
rm -f shelltst.pid
