@echo off

setlocal

rem $Id: ddkbuild_start.bat,v 1.1 2005-03-02 18:17:16 strik Exp $

rem These have to be adapted on your environment
rem I'm assuming DDKBUILD.BAT, Version 5.3
rem It can be downloaded from http://www.osronline.com/article.cfm?article=43
rem Furthermore, I have patched it with patch_ddkbuild.diff to show an error summary
rem after doing its job, but this is not necessary if you do not like.
rem
rem Another option is to use DDKBUILD from Hollies Technologies,
rem downloadable from ...
rem To use that one, just define DDKBUILD_HOLLIS to be 1 on startup.

rem Set this to 1 if you are using DDKBUILD.BAT from Hollis Technology
rem Solutions.  Do not set if using DDKBUILD from OSR
if not defined DDKBUILD_HOLLIS set DDKBUILD_HOLLIS=0

rem First, we have to tell DDKBUILD where all the DDKs are located:

if not defined BASEDIR  set BASEDIR=c:\WINDDK\nt4.ddk
if not defined W2KBASE  set W2KBASE=
if not defined WXPBASE  set WXPBASE=c:\WINDDK\2600
if not defined WNETBASE set WNETBASE=c:\WINDDK\3790

rem After building the driver, the PDB debugging symbols file will be copied
rem to this location here (leave empty of no copying is to be done):

if not defined COPYSYM set COPYSYM=

rem After building the driver, the executable files will be copied
rem to this location here (leave empty of no copying is to be done):

if not defined COPYTARGET set COPYTARGET=

rem Additional arguments for DDKBUILD:
if not defined CMDARGUMENTS set CMDARGUMENTS=


rem Some files which might be useful
if not defined DDKBUILD_CMD_HOLLIS set DDKBUILD_CMD_HOLLIS=ddkbuild_hollis.bat
if not defined DDKBUILD_CMD_OSR    set DDKBUILD_CMD_OSR=ddkbuild_osr.bat


rem --------------------------------------------------------------------------

rem Here, the skript starts. DO NOT CHANGE ANYTHING below this point if you're
rem not totally sure what you're doing.

shift

rem first, check if we want to use a specific version of ddkbuild

if /I "%0" EQU "-hollis" (
	set DDKBUILD=%DDKBUILD_CMD_HOLLIS%
	set DDKBUILD_HOLLIS=1
	shift
) else if /I "%0" EQU "-osr" (
	set DDKBUILD=%DDKBUILD_CMD_OSR%
	set DDKBUILD_HOLLIS=0
	shift
)

rem Now, adjust the parameters for the DDKBUILD
rem version we are using
if %DDKBUILD_HOLLIS% EQU 1 (
	set DDKBUILD=%DDKBUILD_CMD_HOLLIS%
	set TARGETSPEC=-WNETW2K
	set CHECKEDFREE=checked
	if /i "%0" EQU "fre" set CHECKEDFREE=free
) else (
	set DDKBUILD=%DDKBUILD_CMD_OSR%
	set TARGETSPEC=-WNET2K
	set CHECKEDFREE=chk
	if /i "%0" EQU ="fre" set CHECKEDFREE=fre
)

shift

set CMDLINE=%TARGETSPEC% %CHECKEDFREE% .. %0 %1 %2 %3 %4 %5 %6 %7 %8 %9 %CMDARGUMENTS%

rem Make sure no error files are present before starting!
if exist build*.err del build*.err

echo CMDLINE="%CMDLINE%"
echo COPYTARGET="%COPYTARGET%"

call %DDKBUILD% %CMDLINE%

if not exist build*.err (

	rem If we are not called from the root, do not copy!
	if exist ddkbuild_start.bat (

		if "%COPYTARGET%" NEQ "" (
			echo.
			echo =============== copying files to target =============

			xcopy /y ..\bin\i386\*.sys %COPYTARGET%
			if errorlevel 1 echo "ddkbuild.bat(1) : error : could not copy SYS files to %COPYTARGET%"
			xcopy /y ..\bin\i386\*.exe %COPYTARGET%
			if errorlevel 1 echo "ddkbuild.bat(1) : error : could not copy EXE files to %COPYTARGET%"
			xcopy /y ..\bin\i386\*.dll %COPYTARGET%
			if errorlevel 1 echo "ddkbuild.bat(1) : error : could not copy DLL files to %COPYTARGET%"
		)

		if "%COPYSYM%" NEQ "" (
			echo Copying debugging information
			xcopy /y ..\bin\i386\*.pdb %COPYSYM%
			if errorlevel 1 echo "ddkbuild.bat(1) : error : could not copy PDB files for debugging %COPYSYM%"
		)
	)
)

endlocal
