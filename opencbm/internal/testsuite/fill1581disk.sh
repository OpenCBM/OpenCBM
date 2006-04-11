#!/bin/bash

# $Id: fill1581disk.sh,v 1.2 2006-04-11 15:57:20 wmsr Exp $

DRIVENO=`cbmctrl detect | fgrep 1581 | cut -d: -f1`

echo using drive $DRIVENO
echo short reformatting disk
cbmctrl command $DRIVENO "N:FASTREFORMAT"
echo beginning transfer
ls -l 1581filldata/*.prg | sort -nr | cut -c45- | xargs cbmcopy -R -w $DRIVENO
echo transfer ended.
