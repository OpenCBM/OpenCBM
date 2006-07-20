/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2006 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file arch/windows/ctrlbreak.c \n
** \author Spiro Trikaliotis \n
** \version $Id: ctrlbreak.c,v 1.1 2006-07-20 11:45:28 strik Exp $ \n
** \n
** \brief Helper function for setting a handler for Ctrl+C 
** and Ctrl+Break
**
****************************************************************/

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! The name of the executable */
#define DBG_PROGNAME "ARCH.LIB"

#include "debug.h"

#include "arch.h"

#include <windows.h>

static ARCH_CTRLBREAK_HANDLER handler;

static BOOL WINAPI
ControlHandler(DWORD dwCtrlType)
{
    switch (dwCtrlType)
    {
    case CTRL_BREAK_EVENT:
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        handler(0);
        return TRUE;
        break;
    }
    return FALSE;
}


void
arch_set_ctrlbreak_handler(ARCH_CTRLBREAK_HANDLER Handler)
{
    handler = Handler;
    SetConsoleCtrlHandler(ControlHandler, TRUE); 
}
