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
** \file lib/WINBUILD/i_opencbm.h \n
** \author Spiro Trikaliotis \n
** \version $Id: i_opencbm.h,v 1.5 2006-04-10 14:08:09 strik Exp $ \n
** \n
** \brief Internal API for opencbm installation
**
****************************************************************/

#ifndef I_OPENCBM_H
#define I_OPENCBM_H

#include "opencbm.h"

extern BOOL cbm_ioctl(IN CBM_FILE HandleDevice, IN DWORD ControlCode, 
#if DBG
          IN char *TextControlCode, 
#endif // #if DBG
          IN PVOID InBuffer, IN ULONG InBufferSize,
          OUT PVOID OutBuffer, IN ULONG OutBufferSize);

/*! A macro for the call to cbm_ioctl()
 * Remember, I'm lazy...
 */

#if DBG
    #define CBMCTRL( _x_ ) CBMCTRL_##_x_, "CBMCTRL_" #_x_
#else  // #if DBG
    #define CBMCTRL( _x_ ) CBMCTRL_##_x_
#endif // #if DBG

extern BOOL cbm_i_driver_stop(VOID);
extern BOOL cbm_i_driver_start(VOID);

extern BOOL cbm_i_i_driver_install(OUT PULONG Buffer, IN ULONG BufferLen);

EXTERN BOOL CBMAPIDECL cbm_i_driver_install(OUT PULONG Buffer, IN ULONG BufferLen);
/*! Function pointer for the cbm_i_driver_install() function */
typedef BOOL (*P_CBM_I_DRIVER_INSTALL)(OUT PULONG Buffer, IN ULONG BufferLen);

extern LONG RegGetDWORD(IN HKEY RegKey, IN char *SubKey, OUT LPDWORD Value);
extern BOOL IsDriverStartedAutomatically(VOID);

#if DBG
    extern VOID cbm_i_get_debugging_flags(VOID);
#endif

extern VOID WaitForIoCompletionInit(VOID);
extern VOID WaitForIoCompletionDeinit(VOID);
extern VOID WaitForIoCompletionCancelAll(VOID);
extern VOID WaitForIoCompletionConstruct(LPOVERLAPPED Overlapped);
extern BOOL WaitForIoCompletion(BOOL Result, CBM_FILE HandleDevice, LPOVERLAPPED Overlapped, DWORD *BytesTransferred);


#endif /* I_OPENCBM_H */
