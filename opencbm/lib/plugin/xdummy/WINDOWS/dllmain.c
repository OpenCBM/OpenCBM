/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2007, 2009 Spiro Trikaliotis
 *
*/

/*! **************************************************************
** \file lib/plugin/xdummy/WINDOWS/dllmain.c \n
** \author Michael Klein, Spiro Trikaliotis \n
** \n
** \brief Shared library / DLL for accessing the driver, windows specific code
**
****************************************************************/

#include <windows.h>
#include <windowsx.h>

#include "dynlibusb.h"


/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! Mark: We are building the DLL */
#define DBG_DLL

/*! The name of the executable */
#define DBG_PROGNAME "OPENCBM-XDUMMY.DLL"

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

int CBMAPIDECL
opencbm_plugin_init(void)
{
#if DBG

    // Read the debugging flags from the registry

    cbm_get_debugging_flags("xdummy");

#endif

	return 0;
}

void CBMAPIDECL
opencbm_plugin_uninit(void)
{
}
