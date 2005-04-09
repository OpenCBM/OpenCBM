/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2004 Michael Klein <michael.klein@puffin.lb.shuttle.de>
 *  Copyright 2001-2004 Spiro Trikaliotis <cbm4win@trikaliotis.net>
 *
 */

/*! ************************************************************** 
** \file sys/libiec/rawwrite.c \n
** \author Spiro Trikaliotis \n
** \version $Id: rawwrite.c,v 1.1.2.1 2005-04-09 15:10:57 strik Exp $ \n
** \authors Based on code from
**    Michael Klein <michael.klein@puffin.lb.shuttle.de>
** \n
** \brief Write some bytes to the IEC bus
**
****************************************************************/

#include <wdm.h>
#include "cbm4win_common.h"
#include "i_iec.h"

/*! \brief Write some bytes to the IEC bus

 \param Pdx
   Pointer to the device extension.

 \param Buffer
   Pointer to a buffer where the read bytes are written to.

 \param Size
   Maximum number of characters to read from the bus.

 \param Written
   Pointer to the variable which will hold the number of written bytes.

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. Otherwise, it
   returns one of the error status values.

 ATN is released on return of this routine
*/
NTSTATUS
cbmiec_raw_write(IN PDEVICE_EXTENSION Pdx, 
				 IN const PUCHAR Buffer, IN USHORT Size, 
				 OUT USHORT *Written)
{
    NTSTATUS ntStatus;

#if DBG
    unsigned i;
#endif

    FUNC_ENTER();

    PERF_EVENT_VERBOSE(0x1000, 0);

    FUNC_PARAM((DBG_PREFIX "Buffer = 0x%p, Size = 0x%04x", Buffer, Size));

#if DBG
    for (i=0;i<Size;i++)
    {
        FUNC_PARAM((DBG_PREFIX "   output %2u: 0x%02x '%c'", i, (unsigned) Buffer[i], (UCHAR) Buffer[i]));
    }
#endif

    PERF_EVENT_VERBOSE(0x1001, 0);

    ntStatus = cbmiec_i_raw_write(Pdx, Buffer, Size, Written, 0, 0);

    PERF_EVENT_VERBOSE(0x1002, 0);

    FUNC_LEAVE_NTSTATUS(ntStatus);
}
