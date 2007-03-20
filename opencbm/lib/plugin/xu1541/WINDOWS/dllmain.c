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
** \version $Id: dllmain.c,v 1.1.2.3 2007-03-20 18:43:20 strik Exp $ \n
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


BOOL WINAPI
DllMain(IN HANDLE Module, IN DWORD Reason, IN LPVOID Reserved)
{
    return TRUE;
}
