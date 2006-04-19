#! /bin/bash
[ -e sys/libcommon/cbmlog.h ] || exit 1

rm sys/libcommon/cbmlog.rc sys/libcommon/cbmlog.h sys/libcommon/msg00001.bin sys/libcommon/msg00002.bin
rm `find . -name \*.inc|grep -iv makefile` `find . -name \*.plg` `find . -name \*.bsc` WINDOWS/cbm4win-vice.ncb WINDOWS/cbm4win-vice.opt WINDOWS/cbm4win.ncb WINDOWS/cbm4win.opt build*.log
rm -r `find  . -name objchk\*` `find . -name objfre\*` bin/ Debug/ Release/
