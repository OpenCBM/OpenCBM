/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2004 Spiro Trikaliotis
 *
 */

/*! ************************************************************** 
** \file instcbm.h \n
** \author Spiro Trikaliotis \n
** \version $Id: instcbm.h,v 1.8 2006-09-13 15:58:33 strik Exp $ \n
** \n
** \brief Header for installation routines
**
****************************************************************/

#ifndef INSTCBM_H
#define INSTCBM_H

/*! Registry key where the opencbm driver is located (under HKLM) */
#define REGKEY_EVENTLOG \
            "System\\CurrentControlSet\\Services\\Eventlog\\System\\opencbm"

extern PCHAR FormatErrorMessage(DWORD Error);

extern BOOL CbmInstall(IN LPCTSTR DriverName, IN LPCTSTR ServiceExe, IN BOOL AutomaticStart);
extern BOOL CbmRemove(IN LPCTSTR DriverName);
extern BOOL CbmCheckPresence(IN LPCTSTR DriverName);

extern BOOL CbmCheckCorrectInstallation(BOOL HaveAdminRights);

extern VOID CbmParportRestart(VOID);

extern BOOL CbmUpdateParameter(IN ULONG DefaultLpt,
                               IN ULONG IecCableType,
                               IN ULONG PermanentlyLock,
                               IN BOOL DebugFlagsDriverPresent, IN ULONG DebugFlagsDriver,
                               IN BOOL DebugFlagsDllPresent, IN ULONG DebugFlagsDll);

extern VOID CbmInstallUpdate(VOID);
extern BOOL CbmTestIrq(VOID);


#if DBG
extern VOID CbmOutputDebuggingBuffer(VOID);
#endif

#endif // #ifndef INSTCBM_H
