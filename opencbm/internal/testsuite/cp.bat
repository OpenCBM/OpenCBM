@echo off
cls
echo $Id: cp.bat,v 1.1 2006-02-12 13:41:04 strik Exp $

echo.
echo Stopping opencbm (if necessary)
net stop opencbm

echo.
echo Removing driver (if necessary)
if exist instcbm.exe instcbm --remove

echo.
echo Copying files from VMWXP
copy \\vmwxp\opencbm\*.sys
copy \\vmwxp\opencbm\*.dll
copy \\vmwxp\opencbm\*.exe
copy \\vmwxp\opencbm\*.bat
copy \\vmwxp\opencbm\*.sh

if exist setdate.bat del setdate.bat

echo.
echo DONE
