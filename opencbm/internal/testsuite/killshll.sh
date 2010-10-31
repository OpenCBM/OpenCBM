#!/bin/bash

# $Id: killshll.sh,v 1.3 2010-10-31 17:17:53 wmsr Exp $

if [ -f shelltst.pid ]
then
  UXPID=`cat shelltst.pid`

# The following lines need a special utility for Windows

  WINPID=`ps ux | fgrep $UXPID | tail -n 1 | cut -c26-36`
  if [ -n $WINPID ]
  then
    echo "Sending SIGINT (CTRL-C) to Windows console process $WINPID (as background process)"
    cmd.exe /c "sendCTRL-C-1.0rb16.exe $WINPID"
    sleep 5
  fi

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
# avrdude -c dragon_isp -p usb162 -P usb -B 8 -nuqq &
avrdude -c dragon_isp -p usb647 -P usb -B 8 -nuqq &
avrdude -c xu1541asp -p usb1287 -P usb -B 8 -nuqq &


# let the xum1541 resurrect, it may not answer for the next command otherweise
sleep 3
