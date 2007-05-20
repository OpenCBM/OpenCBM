/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2007      Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file lib/plugin/xu1541/WINDOWS/dllmain.c \n
** \author Michael Klein, Spiro Trikaliotis \n
** \version $Id: dllmain.c,v 1.3 2007-05-20 17:32:47 strik Exp $ \n
** \n
** \brief Shared library / DLL for accessing the driver, windows specific code
**
****************************************************************/

#include <windows.h>
#include <windowsx.h>


/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! Mark: We are building the DLL */
#define DBG_DLL

/*! The name of the executable */
#define DBG_PROGNAME "OPENCBM-XU1541.DLL"

/*! This file is "like" debug.c, that is, define some variables */
//#define DBG_IS_DEBUG_C

#include "debug.h"

//! mark: We are building the DLL */
#define OPENCBM_PLUGIN
#include "archlib.h"


/*! \brief Dummy DllMain

 This function is a dummy DllMain(). Without it, the DLL
 is not completely initialized, which breaks us.

 \param Module
   A handle to the DLL.

 \param Reason
   Specifies a flag indicating why the DLL entry-point function is being called. 

 \param Reserved
   Specifies further aspects of DLL initialization and cleanup

 \return
   FALSE if the DLL load should be aborted, else TRUE

 \remark
   For details, look up any documentation on DllMain().
*/

BOOL WINAPI
DllMain(IN HANDLE Module, IN DWORD Reason, IN LPVOID Reserved)
{
    return TRUE;
}

/*! \brief Complete driver installation, "external version"

 This function performs anything that is needed to successfully
 complete the driver installation.

 \param Buffer
   Pointer to a buffer which will return the install information

 \param BufferLen
   The length of the buffer Buffer points to (in bytes).

 \return
   FALSE on success, TRUE on error

 This function is for use of the installation routines only!

 This version of this function is for exporting out of the DLL.
*/

BOOL CBMAPIDECL
cbm_install_complete(OUT PULONG Buffer, IN ULONG BufferLen)
{
    FUNC_ENTER();
    FUNC_LEAVE_BOOL(FALSE);
}
