@echo off

rem $Id: ddkbuild_start.bat,v 1.1 2004-11-07 11:04:54 strik Exp $

rem These have to be adapted on your environment
rem I'm assuming DDKBUILD.BAT, Version 5.3
rem It can be downloaded from http://www.osronline.com/article.cfm?article=43
rem Furthermore, I have patched it with patch_ddkbuild.diff to show an error summary
rem after doing its job, but this is not necessary if you do not like.

rem First, we have to tell DDKBUILD where all the DDKs are located:

set BASEDIR=c:\WINDDK\nt4
set W2KBASE=
set WXPBASE=c:\WINDDK\2600
set WNETBASE=c:\WINDDK\3790

rem After building the driver, the PDB debugging symbols file will be copied
rem to this location here (leave empty of no copying is to be done):

set COPYSYM=

rem After building the driver, the executable files will be copied
rem to this location here (leave empty of no copying is to be done):

set COPYTARGET=

rem Additionally arguments for DDKBUILD:

set CMDARGUMENTS=



rem --------------------------------------------------------------------------

rem Here, the skript starts. DO NOT CHANGE ANYTHING below this point if you're
rem not totally sure what you're doing.

shift

set CMDLINE=-WNET2K %0 . %1 %2 %3 %4 %5 %6 %7 %8 %9 %CMDARGUMENTS%

rem Make sure no error files are present before starting!
if exist build*.err del build*.err

echo CMDLINE="%CMDLINE%"
echo COPYTARGET="%COPYTARGET%"
rem goto end

call ddkbuild %CMDLINE%

if exist build*.err goto end

rem If we are not called from the root, do not copy!
if not exist ddkbuild_start.bat goto end

if "%COPYTARGET%" == "" goto NOCOPYTARGET
echo.
echo =============== copying files to target =============

xcopy /y bin\i386\*.sys %COPYTARGET%
if errorlevel 1 echo ddkbuild.bat(1) : error : could not copy SYS files to %COPYTARGET%
xcopy /y bin\i386\*.exe %COPYTARGET%
if errorlevel 1 echo ddkbuild.bat(1) : error : could not copy EXE files to %COPYTARGET%
xcopy /y bin\i386\*.dll %COPYTARGET%
if errorlevel 1 echo ddkbuild.bat(1) : error : could not copy DLL files to %COPYTARGET% 

:NOCOPYTARGET

if "%COPYSYM%" == "" goto NOCOPYSYM
echo Copying debugging information
xcopy /y bin\i386\*.pdb %COPYSYM%
if errorlevel 1 echo ddkbuild.bat(1) : error : could not copy PDB files for debugging %COPYSYM%

:NOCOPYSYM

:end
