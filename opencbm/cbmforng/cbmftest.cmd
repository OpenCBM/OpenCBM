@ECHO OFF
SET DRIVENO=10


REM transfer mode can be set to `auto' since some versions now
SET TRANSFEROPT=-ta

REM if a parallel cable is available
REM SET TRANSFEROPT=-tp

REM if no parallel cable is available
REM SET TRANSFEROPT=-ts2

REM if this cannot be done with only one drive connected
REM SET TRANSFEROPT=-ts1



REM number of loops with formatting and transferring
SET LOOPCOUNT=10



REM the test image file (35 tracks)
SET SOURCEFILE=filleddk.d64

REM the read back image
SET DESTFILE=destfile.d64

REM logging of the script
SET SCRDEBUGLOG=cbmf_script.log

REM explicit next generation format debug output log file
SET CBMFNGLOG=cbmf_ng_debug.log


if "%1"=="LOGGING" goto LOGGINGENABLED
del %CBMFNGLOG% 2> NUL
del %SCRDEBUGLOG% 2> NUL

ECHO J | CALL %0 LOGGING > %SCRDEBUGLOG%
REM CALL %0 LOGGING | tee %SCRDEBUGLOG%
GOTO EXIT
:LOGGINGENABLED


@ECHO ON

cbmctrl reset
ECHO (Re-) Insert disk for formatting test (it gets overwritten!) 1>&2
cbmctrl change %DRIVENO%
cbmctrl command %DRIVENO% "N0:originalformat,of"
cbmctrl status %DRIVENO% 1>&2

d64copy -v -w %TRANSFEROPT% %SOURCEFILE% %DRIVENO% 1>&2
d64copy -v -w %TRANSFEROPT% %DRIVENO% %DESTFILE% 1>&2
fc /b %SOURCEFILE% %DESTFILE% | find /v "000" 1>&2

for /l %%i in (1,1,%LOOPCOUNT%) do (
    echo Doing next generation formatter test loop No. %%i 1>&2
    echo Doing next generation formatter test loop No. %%i
	cbmctrl reset
	cbmctrl status %DRIVENO%
    echo "Formatting (with verify)..." 1>&2
	cbmforng -o -v -s %DRIVENO% "cbm-ng-format,nf" >> %CBMFNGLOG%
    echo Exporting... 1>&2
	d64copy -v -w %TRANSFEROPT% %SOURCEFILE% %DRIVENO% 1>&2
    echo Importing... 1>&2
	d64copy -v -w %TRANSFEROPT% %DRIVENO% %DESTFILE% 1>&2
    echo Comparing... 1>&2
	fc /b %SOURCEFILE% %DESTFILE% | find /v "000" 1>&2
	)

:EXIT
