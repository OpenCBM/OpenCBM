#! /bin/bash
[ -e sys/libcommon/cbmlog.h ] || exit 1

rm sys/libcommon/cbmlog.rc sys/libcommon/cbmlog.h
rm `find . -name \*.inc|grep -iv makefile` `find . -name \*.plg`
rm -r `find  . -name \*chk\*` `find . -name \*fre\*` bin/
