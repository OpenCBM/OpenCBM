echo off
echo $Id: a7.bat,v 1.3 2006-03-14 21:11:56 wmsr Exp $
:a
if exist test7.bin del test7.bin
cbmctrl download 8 0xc000 0x0001 test7.bin
set RETCODE=%ERRORLEVEL%
if exist test7.bin (
	if not "%RETCODE%" == "0" (
		echo ERROR-Condition: Unsuccessful command created a file of size:
		dir | find "test7.bin"
		) else (
		echo single MP test was completely successful
		)
	) else (
	if "%RETCODE%" == "0" (
		echo ERROR-Condition: Successful command did not create a file
		) else (
		echo unsuccessful single MP test did not create a file, which is correct
		)
	)
goto :a
