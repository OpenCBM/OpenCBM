# Microsoft Developer Studio Project File - Name="opencbm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=opencbm - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "opencbm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "opencbm.mak" CFG="opencbm - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "opencbm - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "opencbm - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "opencbm - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../Release"
# PROP Intermediate_Dir "../../Release/opencbm"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DLL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../" /I "../../include" /I "../../include/WINDOWS/" /I "../WINBUILD/" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DLL_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /i "../../include" /i "../../include/WINDOWS/" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib arch.lib winmm.lib /nologo /dll /machine:I386 /libpath:"../../Release"

!ELSEIF  "$(CFG)" == "opencbm - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../Debug"
# PROP Intermediate_Dir "../../Debug/opencbm"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DLL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../" /I "../../include" /I "../../include/WINDOWS/" /I "../WINBUILD/" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DLL_EXPORTS" /D "MSVC_COMPILE" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /i "../../include" /i "../../include/WINDOWS/" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib arch.lib winmm.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"../../Debug"

!ENDIF 

# Begin Target

# Name "opencbm - Win32 Release"
# Name "opencbm - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\archlib_vice.c
# End Source File
# Begin Source File

SOURCE=.\archmnib_vice.c
# End Source File
# Begin Source File

SOURCE=..\cbm.c
# End Source File
# Begin Source File

SOURCE=..\detect.c
# End Source File
# Begin Source File

SOURCE=..\detectxp1541.c
# End Source File
# Begin Source File

SOURCE=..\gcr_4b5b.c
# End Source File
# Begin Source File

SOURCE=.\i_opencbm_vice.c
# End Source File
# Begin Source File

SOURCE=.\opencbmvice.def
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\petscii.c
# End Source File
# Begin Source File

SOURCE=..\upload.c
# End Source File
# Begin Source File

SOURCE=.\vice_comm.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\archlib.h
# End Source File
# Begin Source File

SOURCE=..\..\include\WINDOWS\cbmioctl.h
# End Source File
# Begin Source File

SOURCE=..\..\include\debug.h
# End Source File
# Begin Source File

SOURCE=..\WINBUILD\i_opencbm.h
# End Source File
# Begin Source File

SOURCE=..\..\include\opencbm.h
# End Source File
# Begin Source File

SOURCE=.\vice_comm.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\opencbmvice.rc
# End Source File
# End Group
# Begin Source File

SOURCE=.\makefile
# End Source File
# Begin Source File

SOURCE=.\sources
# End Source File
# Begin Source File

SOURCE=..\..\include\WINDOWS\version.common.h
# End Source File
# End Target
# End Project
