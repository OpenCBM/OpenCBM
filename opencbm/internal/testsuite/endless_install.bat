@echo off
cls
echo $Id: endless_install.bat,v 1.1 2006-02-12 13:41:04 strik Exp $

echo.
echo Checking without having installed the driver
instcbm --check

:a
echo.
echo Installing driver with --nocopy
instcbm --nocopy
echo.
echo Checking driver 1
instcbm --check
echo.
echo Checking driver 2
instcbm --check
echo.
echo Checking driver 3
instcbm --check
echo.
echo Removing driver
instcbm --remove
echo.
goto a
