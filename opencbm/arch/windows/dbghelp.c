/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2005 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file arch/windows/dbghelp.c \n
** \author Spiro Trikaliotis \n
** \version $Id: dbghelp.c,v 1.1 2005-04-17 15:32:17 strik Exp $ \n
** \n
** \brief Some debugging help functions
**
****************************************************************/

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! The name of the executable */
#define DBG_PROGNAME "ARCH.LIB"

#include "debug.h"

#include "arch.h"

#include <windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

// #define DBG_DUMP_RAW_READ
// #define DBG_DUMP_RAW_WRITE

/*-------------------------------------------------------------------*/
/*--------- DEBUGGING FUNCTIONS -------------------------------------*/

#if DBG

void 
dbg_memdump(const char *Where, const unsigned char *InputBuffer, const unsigned int Count)
{
    unsigned i;
    char outputBufferChars[17];
    char outputBuffer[100];
    char *p;

    p = outputBuffer;

    DBG_PRINT((DBG_PREFIX "%s: (0x%04x)", Where, (unsigned) Count));

    for (i=0; i<Count; i++) 
    {
        p += sprintf(p, "%02x ", (unsigned) InputBuffer[i]);

        if (i % 16 == 7)
        {
            p += sprintf(p, "- ");
        }

        outputBufferChars[i % 16] = isprint(InputBuffer[i]) ? InputBuffer[i] : '.';

        if (i % 16 == 15)
        {
            outputBufferChars[(i % 16) + 1] = 0;
            DBG_PRINT((DBG_PREFIX "%04x: %-50s  %s",
                (unsigned) i & 0xfff0, outputBuffer, outputBufferChars));
            p = outputBuffer;
        }
    }

    if (i % 16 != 0)
    {
        outputBufferChars[i % 16] = 0;
        DBG_PRINT((DBG_PREFIX "%04x: %-50s  %s",
            (unsigned) i & 0xfff0, outputBuffer, outputBufferChars));
    }
}

#endif // #if DBG
