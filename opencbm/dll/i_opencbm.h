/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2004 Spiro Trikaliotis <cbm4win@trikaliotis.net>
 *
 */

/*! ************************************************************** 
** \file i_opencbm.h \n
** \author Spiro Trikaliotis \n
** \version $Id: i_opencbm.h,v 1.4 2004-11-24 20:08:18 strik Exp $ \n
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

extern const char * cbm_i_get_driver_name(int PortNumber);
extern int  cbm_i_driver_open(CBM_FILE *HandleDevice, int PortNumber);
extern void cbm_i_driver_close(CBM_FILE HandleDevice);

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

#endif /* I_OPENCBM_H */
