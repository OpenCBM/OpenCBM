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
** \file lib/WINVICEBUILD/vice_comm.c \n
** \author Spiro Trikaliotis \n
** \version $Id: vice_comm.c,v 1.2 2006-02-22 09:52:09 strik Exp $ \n
** \n
** \brief Library functions for communicating with VICE.
**
****************************************************************/

#include <windows.h>
#include <windowsx.h>

#include "viceremote.h"

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! The name of the executable */
#define DBG_PROGNAME "OPENCBM.DLL"

#include "debug.h"

#include <winioctl.h>
#include "cbmioctl.h"

#include <stdlib.h>

//! mark: We are building the DLL */
#define DLL
#include "i_opencbm.h"
#include "archlib.h"

#include "arch.h"
#include "vice_comm.h"

static REMOTECONTROL *ptr_mappedfile = 0;
static HANDLE handle_mappedfile = 0;

static UINT buffernotowrite = -1;

static REMOTECONTROL_ONEMEMBUFFER *membuffertowrite = 0;
static REMOTECONTROL_ONEREGBUFFER *regbuffertowrite = 0;

static REMOTECONTROL_ONEMEMBUFFER *membuffertoread = 0;
static REMOTECONTROL_ONEREGBUFFER *regbuffertoread = 0;

int vicereadregister(viceregs which)
{
    unsigned int value;

    switch (which)
    {
        case reg_pc:    value = regbuffertoread->OutRegs.PC;    break;
        case reg_a:     value = regbuffertoread->OutRegs.AC;    break;
        case reg_x:     value = regbuffertoread->OutRegs.XR;    break;
        case reg_y:     value = regbuffertoread->OutRegs.YR;    break;
        case reg_sp:    value = regbuffertoread->OutRegs.SP;    break;
        case reg_flags: value = regbuffertoread->OutRegs.FLAGS; break;
    }

    return value;
}

void vicewriteregister(viceregs which, unsigned int value)
{
    switch (which)
    {
        case reg_pc:    regbuffertowrite->InRegsValid.PC    = 1; regbuffertowrite->InRegs.PC    = value; break;
        case reg_a:     regbuffertowrite->InRegsValid.AC    = 1; regbuffertowrite->InRegs.AC    = value; break;
        case reg_x:     regbuffertowrite->InRegsValid.XR    = 1; regbuffertowrite->InRegs.XR    = value; break;
        case reg_y:     regbuffertowrite->InRegsValid.YR    = 1; regbuffertowrite->InRegs.YR    = value; break;
        case reg_sp:    regbuffertowrite->InRegsValid.SP    = 1; regbuffertowrite->InRegs.SP    = value; break;
        case reg_flags: regbuffertowrite->InRegsValid.FLAGS = 1; regbuffertowrite->InRegs.FLAGS = value; break;
    }
}

void vicewriteregister_when_at(unsigned int value)
{
    ptr_mappedfile->regupdateaddress = value;
}

void vicereadmemory(unsigned int address, unsigned int size, char *buffer)
{
    if (ptr_mappedfile)
    {
        DBG_ASSERT(membuffertoread->read.perform == 0); // make sure the read was executed
        DBG_ASSERT(membuffertoread->read.address == address);
        DBG_ASSERT(membuffertoread->read.size    == size);

        memcpy(buffer, ptr_mappedfile->data, size);
    }
}

void vicepreparereadmemory(unsigned int address, unsigned int size)
{
    if (ptr_mappedfile)
    {
        DBG_ASSERT(membuffertowrite->read.perform == 0);

        membuffertowrite->read.perform = 1;
        membuffertowrite->read.address = address;
        membuffertowrite->read.size    = size;
    }
}

void vicewritememory(unsigned int address, unsigned int size, const char *buffer)
{
    if (ptr_mappedfile)
    {
        DBG_ASSERT(membuffertowrite->write.perform == 0);

        membuffertowrite->write.perform = 1;
        membuffertowrite->write.address = address;
        membuffertowrite->write.size    = size;

        memcpy(ptr_mappedfile->data, buffer, size);
    }
}

void vicepause()
{
    if (ptr_mappedfile)
    {
        membuffertoread = 0;
        regbuffertoread = 0;

        buffernotowrite = (ptr_mappedfile->remotecontrollerbuffer + 1) & 1;
        membuffertowrite = &ptr_mappedfile->memorybuffer[buffernotowrite];
        regbuffertowrite = &ptr_mappedfile->regbuffer[buffernotowrite];

        // delete the complete data which we might want to set
        memset(membuffertowrite, 0, sizeof(*membuffertowrite));
        memset(regbuffertowrite, 0, sizeof(*regbuffertowrite));

        ptr_mappedfile->trapaddress = -1;
        ptr_mappedfile->regupdateaddress = -1;
    }
}

void viceresume()
{
    if (ptr_mappedfile)
    {
        DBG_ASSERT(ptr_mappedfile->remotecontrollerbuffer == ptr_mappedfile->vicebuffer);

        // make the new buffer active
        InterlockedIncrement(&ptr_mappedfile->remotecontrollerbuffer);

        buffernotowrite = -1;

        membuffertoread = membuffertowrite;
        regbuffertoread = regbuffertowrite;

        membuffertowrite = 0;
        regbuffertowrite = 0;
    }
}

void vicetrap(UINT address)
{
    if (ptr_mappedfile)
    {
        ptr_mappedfile->trapaddress = address;
    }
}

void vicereset(void)
{
    if (ptr_mappedfile)
    {
        ptr_mappedfile->reset = 1;
    }
}

void vicewaittrap(void)
{
    if (ptr_mappedfile)
    {
        while (InterlockedExchangeAdd(&ptr_mappedfile->vicebuffer, 0)
            != InterlockedExchangeAdd(&ptr_mappedfile->remotecontrollerbuffer, 0))
            arch_usleep(1);
    }
}

void vicerelease(void)
{
    if (ptr_mappedfile)
    {
        vicepause();
        vicewriteregister(reg_pc, 0xe39d); // better than 0xa474, as it restores the SP
        viceresume();

        vicewaittrap();

        if (InterlockedExchangeAdd(&ptr_mappedfile->version, 0) == 1)
            InterlockedDecrement(&ptr_mappedfile->controllerAvailable);

        UnmapViewOfFile(ptr_mappedfile);
        ptr_mappedfile = 0;
    }

    if (handle_mappedfile)
    {
        CloseHandle(handle_mappedfile);
        handle_mappedfile = 0;
    }
}

BOOLEAN viceinit(void)
{
    static BOOLEAN success = FALSE;

    do {
#if 0
        handle_mappedfile =
            CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
            0, sizeof(*ptr_mappedfile), "VICE_REMOTE_CONTROL");
#else
        handle_mappedfile =
            OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "VICE_REMOTE_CONTROL");
#endif

        if (!handle_mappedfile || handle_mappedfile == INVALID_HANDLE_VALUE)
        {
            handle_mappedfile = 0;
            break;
        }

        ptr_mappedfile =
            MapViewOfFile(handle_mappedfile, FILE_MAP_ALL_ACCESS,
            0, 0, sizeof(*ptr_mappedfile));

        if (!ptr_mappedfile)
        {
            break;
        }
    } while (0);

    do {
        if (!ptr_mappedfile)
            break;

#ifdef MSVC_COMPILE
/*
 * Why this #ifdef and the #define?
 * - With older headers (for example, as given with MSVC6), there
 *   was only an 
 *   PVOID InterlockedCompareExchange(PVOID *Dest, PVOID Exch, PVOID Comperand);
 * - With ne^wer SDKs (and the DDK), the above function has been renamed to
 *   InterlockedCompareExchangePointer(), and a new function
 *   LONG InterlockedCompareExchange(PLONG Dest, LONG Exch, LONG Comperand);
 *   has been added.
 *   (This is due to the fact that a pointer and an ULONG are not necessarily
 *   the same length anymore with the introduction of WIN64, cf. ULONG_PTR,
 *   InterlockedCompareExchange64(), etc.)
 *   Thus, depending upon the headers we are including, the one or the other
 *   use gets some warning. This #ifdef (both paths only differ in the casts)
 *   avoids any warnings.
 * -@srt.20060221
 */

# define InterlockedCompareExchange(_a, _b, _c) \
    (LONG) InterlockedCompareExchange(_a, (PVOID)_b, (PVOID)_c)

#endif

        if (InterlockedCompareExchange(
                  (PVOID)&ptr_mappedfile->version, 1, 0)
                  > 1)
            break;

#undef InterlockedCompareExchange

        if (InterlockedIncrement(&ptr_mappedfile->controllerAvailable) > 1)
            break;

        success = TRUE;        
    } while (0);

    if (!success)
    {
        vicerelease();
    }

    return success;
}
