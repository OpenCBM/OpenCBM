#!/bin/bash

# $Id: killshll.sh,v 1.1 2006-04-12 23:38:20 wmsr Exp $

if [ -f shelltst.pid ]
then
  UXPID=`cat shelltst.pid`

# The following lines need a special utility for Windows
#
#  WINPID=`ps ux | fgrep $UXPID | tail -n 1 | cut -c26-36`
#  echo "Sending SIGINT (CTRL-C) to Windows console process $WINPID"
#  sendCTRL-C $WINPID
#  sleep 5

  ps ux | fgrep $UXPID > /dev/null
  if [ $? -eq 0 ]
  then
    echo "Doing SIGKILL on process group ID $UXPID"
    kill -9 -$UXPID
    sleep 1
  fi
  rm -f shelltst.pid
fi
echo "Doing reset on IEC bus"
cbmctrl reset
