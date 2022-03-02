@echo off
cls

set OC_DESTINATION=%ProgramFiles%\opencbm

set OC_VARIANT_DISPLAY_DEFAULT=ZoomFloppy
set OC_VARIANT_DEFAULT=xum1541
set OC_VARIANT_DEFAULT_INSTALL_DRIVER=OC_INSTALL_DRIVER_ZOOMFLOPPY
set OC_VARIANT_DEFAULT_INSTALL_IS_USB=1

rem ---------------------------------------

setlocal enabledelayedexpansion

set OC_SOURCE_PATH=%~dp0

set OC_VERSION=0.4.99.104
set OC_INSTALLED_SIZE_IN_KB_AMD64=1500
set OC_INSTALLED_SIZE_IN_KB_I386=2000

set OC_VARIANT_DISPLAY=
set OC_VARIANT=

if [%PROCESSOR_ARCHITECTURE%] == [AMD64] (
	set OC_BINDIR_LOCAL=amd64
	set OC_INSTALLED_SIZE_IN_KB=%OC_INSTALLED_SIZE_IN_KB_AMD64%
) else if [%PROCESSOR_ARCHITECTURE%] == [x86] (
	set OC_BINDIR_LOCAL=i386
	set OC_INSTALLED_SIZE_IN_KB=%OC_INSTALLED_SIZE_IN_KB_I386%
) else (
	echo unknown architecture, exiting...
	exit
)


set OC_INSTALL_DRIVER_ZOOMFLOPPY=0
set OC_INSTALL_DRIVER_USB=0
set OC_INSTALL_DRIVER_XUM1541=0
set OC_INSTALL_DRIVER_XU1541=0
set OC_INSTALL_DRIVER_XA1541=0
set OC_INSTALL_ELEVATED=0
set OC_IS_XP_OR_OLDER=0


rem Check Windows version

for /f "tokens=4-5 delims=. " %%i in ('ver') do set VERSION=%%i.%%j
for /f "tokens=5-6 delims=. " %%i in ('ver') do set VERSION2=%%i.%%j

rem echo.

if "%version%" == "10.0" (
	rem echo Windows 10
	set OC_INSTALL_WIN=10
)
if "%version%" == "6.3" (
	rem echo Windows 8.1
	set OC_INSTALL_WIN=8.1
)
if "%version%" == "6.2" (
	rem echo Windows 8
	set OC_INSTALL_WIN=8
)
if "%version%" == "6.1" (
	rem echo Windows 7
	if "%version2%" == "1.7600]" (
		rem echo . Windows 7 w/o SP
		set OC_INSTALL_WIN=7
	)
	if "%version2%" == "1.7601]" (
		rem echo . Windows 7 SP1
		set OC_INSTALL_WIN=7SP1
	)
)
if "%version%" == "6.0" (
	rem echo Windows Vista.
	set OC_INSTALL_WIN=Vista
	echo I do not know how to install on Windows Vista. If this fails, try again with --xp as option.
	pause
)
if "%version2%" == "5.1" (
	rem echo Windows XP
	set OC_INSTALL_WIN=XP
	set OC_IS_XP_OR_OLDER=1
)
if "%version2%" == "5.00" (
	rem echo Windows 2000
	set OC_INSTALL_WIN=2000
	set OC_IS_XP_OR_OLDER=1
)
if "%version2%" == "4.0" (
	rem echo Windows NT 4.0
	set OC_INSTALL_WIN=4
	set OC_IS_XP_OR_OLDER=1
)

rem Process command line parameter

for /d %%p in (%*) do (
	if [%%~p] == [zoomfloppy] (
		set OC_VARIANT_DISPLAY=!OC_VARIANT_DISPLAY! ZoomFloppy
		set OC_VARIANT=!OC_VARIANT! xum1541
		set OC_INSTALL_DRIVER_ZOOMFLOPPY=1
		set OC_INSTALL_DRIVER_USB=1
	) else if [%%~p] == [xum1541] (
		set OC_VARIANT_DISPLAY=!OC_VARIANT_DISPLAY! xum1541
		set OC_VARIANT=!OC_VARIANT! xum1541
		set OC_INSTALL_DRIVER_XUM1541=1
		set OC_INSTALL_DRIVER_USB=1
	) else if [%%~p] == [xu1541] (
		set OC_VARIANT_DISPLAY=!OC_VARIANT_DISPLAY! xu1541
		set OC_VARIANT=!OC_VARIANT! xu1541
		set OC_INSTALL_DRIVER_XU1541=1
		set OC_INSTALL_DRIVER_USB=1
	) else if [%%~p] == [xa1541] (
		set OC_VARIANT_DISPLAY=!OC_VARIANT_DISPLAY! xa1541
		set OC_VARIANT=!OC_VARIANT! xa1541
		set OC_INSTALL_DRIVER_XA1541=1
	) else if [%%~p] == [--xp] (
		set OC_IS_XP_OR_OLDER=1
	) else if [%%~p] == [--admin] (
		set OC_IS_XP_OR_OLDER=1
	) else if [%%~p] == [--internal_call_elevated] (
		set OC_INSTALL_ELEVATED=1
	) else (
		echo Unknown parameter %%~p, ignoring ...
		pause
	)
)

rem If no variant was given, use default

if "%OC_VARIANT_DISPLAY%" == "" (
	set OC_VARIANT_DISPLAY=%OC_VARIANT_DISPLAY_DEFAULT%
	set OC_VARIANT=%OC_VARIANT_DEFAULT%
	set %OC_VARIANT_DEFAULT_INSTALL_DRIVER%=1
	set OC_INSTALL_DRIVER_USB=%OC_VARIANT_DEFAULT_INSTALL_IS_USB%
)

if %OC_INSTALL_ELEVATED% EQU 0 (
	rem Check if we already have administrative rights.
	rem Idea taken from https://stackoverflow.com/questions/4051883/batch-script-how-to-check-for-admin-rights
	rem and https://stackoverflow.com/questions/4051883/batch-script-how-to-check-for-admin-rights#21295806
	rem
	rem fsutil should work on XP onwards, but not PE
	rem fsutil dirty query %SYSTEMDRIVE% >nul 2>&1
	rem
	rem sfc should work up to 2000, and also on PE
	sfc 2>&1 | find /i "/" >nul

	if !errorlevel! EQU 0 (
		rem echo We have administrative rights
		set OC_INSTALL_ELEVATED=1
	) else (
		rem echo We do not have administrative rights
	)
)

if not exist "%OC_SOURCE_PATH%\uninstall.cmd.add" (
	echo del "%USERPROFILE%\Desktop\OpenCBM.lnk" > "%OC_SOURCE_PATH%\uninstall.cmd.add"
)

rem Check if we are running elevated. If not, restart after elevating privileges
if %OC_INSTALL_ELEVATED% == 0 (
	echo OpenCBM installation script
	echo ===========================
	echo.
	echo Elevating privileges in order to install OpenCBM.
	echo Please grant the rights on the UAC prompt,
	echo or I will not be able to continue!
	echo.
	if %OC_IS_XP_OR_OLDER% EQU 0 (
		powershell -Command "Start-Process -FilePath \"%~dpnx0\" -ArgumentList \"--internal_call_elevated %*\" -Verb runAs"
	) else (
		runas /user:administrator "\"%~dpnx0\" --internal_call_elevated %*"
	)
	echo.
	if errorlevel 1 (
		echo ERROR
		echo =====
		echo.
		echo I could not get administrative rights - aborting!
		echo.
		del "%OC_SOURCE_PATH%\uninstall.cmd.add"
	) else (
		echo Your installation will continue in another window.

		if %OC_IS_XP_OR_OLDER% EQU 1 if %OC_INSTALL_DRIVER_USB% EQU 1 (
			echo.
			echo ===========================================================
			echo = Please do not forget to install the drivers for OpenCBM =
			echo ===========================================================
			echo.
			echo I am opening the path to them, so you know from where to install.
			echo.
			start xp_drv
		)
		rem create Shortcut
		.\tools\genShortCut.vbs "%USERPROFILE%" "%OC_DESTINATION%" "%OC_VERSION%"
	)
	pause
	goto EXIT
)

echo Continuing...

rem Install by copying everything in place

xcopy "%OC_SOURCE_PATH%\%OC_BINDIR_LOCAL%\*.exe" "%OC_DESTINATION%\"     /q /i /y /c
xcopy "%OC_SOURCE_PATH%\%OC_BINDIR_LOCAL%\*.dll" "%OC_DESTINATION%\"     /q /i /y /c
rem The \.\ is needed, or cmd.exe will not process the '*' correctly. Don't ask me why...
xcopy "%OC_SOURCE_PATH%\.\*.pdf"                 "%OC_DESTINATION%\doc\" /q /i /y /c
xcopy "%OC_SOURCE_PATH%\tools"                   "%OC_DESTINATION%\tools\" /q /i /y /c

mkdir "%OC_DESTINATION%\installer"

if [%OC_INSTALL_DRIVER_XA1541%] == [1] (
	xcopy "%OC_SOURCE_PATH%\%OC_BINDIR_LOCAL%\*.sys" "%OC_DESTINATION%\" /q /i /y /c
)

pushd "%OC_DESTINATION%"
instcbm.exe %OC_VARIANT%
popd

echo @echo off> "%OC_DESTINATION%\installer\uninstall.cmd"
echo @cd /d "%OC_DESTINATION%">> "%OC_DESTINATION%\installer\uninstall.cmd"
echo echo Removing %OC_VARIANT%>> "%OC_DESTINATION%\installer\uninstall.cmd"
echo instcbm --remove %OC_VARIANT%>> "%OC_DESTINATION%\installer\uninstall.cmd"
echo del "%SystemRoot%\System32\opencbm.conf">> "%OC_DESTINATION%\installer\uninstall.cmd"
echo cd ..>> "%OC_DESTINATION%\installer\uninstall.cmd"
IF EXIST "%OC_SOURCE_PATH%\uninstall.cmd.add" (
	type "%OC_SOURCE_PATH%\uninstall.cmd.add" >> "%OC_DESTINATION%\installer\uninstall.cmd"
	del "%OC_SOURCE_PATH%\uninstall.cmd.add"
)
echo %%SYSTEMROOT%%\System32\reg delete "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenCBM" /f>> "%OC_DESTINATION%\installer\uninstall.cmd"
echo rd /s /q "%OC_DESTINATION%">> "%OC_DESTINATION%\installer\uninstall.cmd"

set day=%date:~0,2%
set month=%date:~3,2%
set year=%date:~6%

%SYSTEMROOT%\System32\reg delete "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenCBM" /f > NUL 2> NUL

%SYSTEMROOT%\System32\reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenCBM" /t REG_SZ    /v "UninstallString" /d "%SYSTEMROOT%\System32\cmd.exe /c \"%OC_DESTINATION%\installer\uninstall.cmd\"" > NUL
%SYSTEMROOT%\System32\reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenCBM" /t REG_SZ    /v "Comments"        /d "OpenCBM v%OC_VERSION% %OC_VARIANT_DISPLAY%" > NUL
%SYSTEMROOT%\System32\reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenCBM" /t REG_SZ    /v "DisplayName"     /d "OpenCBM v%OC_VERSION%" > NUL
%SYSTEMROOT%\System32\reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenCBM" /t REG_SZ    /v "DisplayVersion"  /d "%OC_VERSION%" > NUL
%SYSTEMROOT%\System32\reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenCBM" /t REG_SZ    /v "InstallDate"     /d "%year%%month%%day%" > NUL
%SYSTEMROOT%\System32\reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenCBM" /t REG_SZ    /v "Publisher"       /d "The OpenCBM team" > NUL
%SYSTEMROOT%\System32\reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenCBM" /t REG_SZ    /v "HelpLink"        /d "http://opencbm.sourceforge.net/" > NUL
%SYSTEMROOT%\System32\reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenCBM" /t REG_DWORD /v "NoModify"        /d 1 > NUL
%SYSTEMROOT%\System32\reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenCBM" /t REG_DWORD /v "NoRepair"        /d 1 > NUL
%SYSTEMROOT%\System32\reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenCBM" /t REG_DWORD /v "EstimatedSize"   /d %OC_INSTALLED_SIZE_IN_KB% > NUL


echo.
echo =================================================
echo === OpenCBM v%OC_VERSION% for %OC_VARIANT_DISPLAY%
echo === installation done!
echo ===
echo === Add %OC_DESTINATION% to your PATH to use the command line tools there.
echo =================================================
echo.

if %OC_IS_XP_OR_OLDER% EQU 1 goto EXIT

set INFER_PATH="%OC_SOURCE_PATH%\tools"
set INFER_EXENAME=INFer.exe
set INFER_EXE="%INFER_PATH%\%INFER_EXENAME%"

if not "%OC_INSTALL_DRIVER_ZOOMFLOPPY% %OC_INSTALL_DRIVER_XUM1541% %OC_INSTALL_DRIVER_XU1541%" == "0 0 0" (

	If exist "%INFER_EXE%" (

		echo.
		echo I could install the necessary USB drivers if you like.
		echo If you want this, then please
		echo 1. attach your device, and then
		echo 2. answer the following question with "y", without parenthesis:
		echo.
		set OC_INSTALL_USB=
		set /p OC_INSTALL_USB="Do you want me to install the necessary USB drivers for you? (y/n) "

		set OC_INSTALL_USB_RESULT=
		if [!OC_INSTALL_USB!] == [y] set OC_INSTALL_USB_RESULT=1
		if [!OC_INSTALL_USB!] == [Y] set OC_INSTALL_USB_RESULT=1

		if [!OC_INSTALL_USB_RESULT!] == [1] (
			echo.
			pushd "%INFER_PATH%"
			"%INFER_EXENAME%" -f opencbm.inf
			popd
		) else (
			echo NO.
		)
	)
)

echo.
echo That's it, I am finished.
pause

:EXIT
