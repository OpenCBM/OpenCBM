#!/bin/bash

# $Id: killshll.sh,v 1.2 2010-01-07 00:32:39 wmsr Exp $

if [ -f shelltst.pid ]
then
  UXPID=`cat shelltst.pid`

# The following lines need a special utility for Windows

  WINPID=`ps ux | fgrep $UXPID | tail -n 1 | cut -c26-36`
  echo "Sending SIGINT (CTRL-C) to Windows console process $WINPID"
  sendCTRL-C $WINPID
  sleep 5

  ps ux | fgrep $UXPID > /dev/null
  if [ $? -eq 0 ]
  then
    echo "Doing SIGKILL on process group ID $UXPID"
    kill -9 -$UXPID
    sleep 1
  fi
  rm -f shelltst.pid
fi
# echo "Doing reset on IEC bus"
# cbmctrl reset
echo "Doing reset of the xum1541 firmware including IEC bus"
avrdude -c avrisp2 -P usb -p usb1287 -nuqq
