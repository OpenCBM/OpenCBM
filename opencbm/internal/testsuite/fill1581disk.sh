#!/bin/bash

# $Id: fill1581disk.sh,v 1.3 2006-04-12 23:38:20 wmsr Exp $

DRIVENO=`cbmctrl detect | fgrep 1581 | cut -d: -f1`

echo $$ > shelltst.pid
echo using drive $DRIVENO
echo short reformatting disk
cbmctrl command $DRIVENO "N:FASTREFORMAT"
echo beginning transfer
/bin/ls -l 1581filldata/*.prg | /bin/sort -nr | /bin/cut -c45- | /bin/xargs cbmcopy -R -w $* $DRIVENO
rm -f shelltst.pid
echo transfer ended.
