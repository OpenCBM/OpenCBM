/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2001-2004 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file install/i_opencbm.c \n
** \author Spiro Trikaliotis \n
** \version $Id: i_opencbm.c,v 1.3 2004-12-22 14:43:21 strik Exp $ \n
** \n
** \brief Functions for accessing the driver
**
****************************************************************/

/*! The name of the executable */
#ifndef DBG_PROGNAME
    #define DBG_PROGNAME "INSTCBM.EXE"
#endif // #ifndef DBG_PROGNAME

/*! Mark: We are in user-space (for debug.h) */
// #define DBG_USERMODE

// Include the functionality of the DLL into the install
// Application

#include "../lib/WINBUILD/i_opencbm.c"


#if DBG

#include <stdio.h>

/*! \brief Output contents of the debugging buffer

 This functions outputs the contents of the kernel-mode
 debugging buffer to the screen.

 This function is for use of the installation routines only!
*/

VOID
CbmOutputDebuggingBuffer(VOID)
{
    CHAR buffer[0x20000];

    CBM_FILE HandleDevice;

    FUNC_ENTER();

    if (cbm_i_driver_open(&HandleDevice, 0) == 0)
    {
        PCHAR p = buffer;
        PCHAR endLine;

        cbm_ioctl(HandleDevice, CBMCTRL(I_READDBG), NULL, 0, buffer, sizeof(buffer));
        cbm_i_driver_close(HandleDevice);

        printf("Output of the debugging buffer:\n\n");

        do {

            endLine = strchr(p, 13);
            if (endLine)
            {
                *endLine = 0;
            }
            printf("%s", p);

            p = endLine + 1;

        } while (endLine);
    }

    FUNC_LEAVE();
}

#endif // #if DBG
