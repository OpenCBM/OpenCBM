@echo off
cls
echo $Id: $

echo.
echo Performing RESET
cbmctrl reset

:repeat
echo.
echo Performing STATUS 8
cbmctrl status 8
goto repeat
