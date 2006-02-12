@echo off
cls
echo $Id: endless_reset_status.bat,v 1.1 2006-02-12 13:41:04 strik Exp $

echo.
echo Performing RESET
cbmctrl reset

:repeat
echo.
echo Performing STATUS 8
cbmctrl status 8
goto repeat
