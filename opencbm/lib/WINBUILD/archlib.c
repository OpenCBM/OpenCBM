/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2001 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Copyright 2001-2005,2007 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file lib/WINBUILD/archlib.c \n
** \author Michael Klein, Spiro Trikaliotis \n
** \version $Id: archlib.c,v 1.16.2.2 2007-03-20 18:43:19 strik Exp $ \n
** \n
** \brief Shared library / DLL for accessing the driver, windows specific code
**
****************************************************************/

#include <windows.h>

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! Mark: We are building the DLL */
#define DBG_DLL

/*! The name of the executable */
#define DBG_PROGNAME "OPENCBM.DLL"

/*! This file is "like" debug.c, that is, define some variables */
#define DBG_IS_DEBUG_C

#include "debug.h"


//! mark: We are building the DLL */
#define DLL
#include "i_opencbm.h"
#include "archlib.h"


/*! \brief DLL initialization und unloading

 This function is called whenever the DLL is loaded or unloaded.
 It ensures that the driver is loaded to be able to call its
 functions.

 \param Module
   Handle of the module; this is not used.

 \param Reason
   DLL_PROCESS_ATTACH if the DLL is loaded,
   DLL_PROCESS_DETACH if it is unloaded.

 \param Reserved
   Not used.

 \return 
   Returns TRUE on success, else FALSE.

 If this function returns FALSE, windows reports that loading the DLL
 was not successful. If the DLL is linked statically, the executable
 refuses to load with STATUS_DLL_INIT_FAILED (0xC0000142)
*/

BOOL WINAPI
DllMain(IN HANDLE Module, IN DWORD Reason, IN LPVOID Reserved)
{
    static BOOL bIsOpen = FALSE;

    FUNC_ENTER();

#if DBG

    if (Reason == DLL_PROCESS_ATTACH)
    {
        // Read the debugging flags from the registry

        cbm_i_get_debugging_flags();
    }

#endif

    FUNC_LEAVE_BOOL(TRUE);
}
