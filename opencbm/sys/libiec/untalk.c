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
** \file sys/libiec/untalk.c \n
** \author Spiro Trikaliotis \n
** \version $Id: untalk.c,v 1.1 2004-11-07 11:05:14 strik Exp $ \n
** \authors Based on code from
**    Michael Klein <michael.klein@puffin.lb.shuttle.de>
** \n
** \brief Send an UNTALK to the IEC bus
**
****************************************************************/

#include <wdm.h>
#include "cbm4win_common.h"
#include "i_iec.h"

/*! \brief Send an UNTALK over the IEC bus

 This function sends an UNTALK to the IEC bus.

 \param Pdx
   Pointer to the device extension.

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. Otherwise, it
   returns one of the error status values.
*/
NTSTATUS
cbmiec_untalk(IN PDEVICE_EXTENSION Pdx)
{
    NTSTATUS ntStatus;
    USHORT sent;
    UCHAR buffer;

    FUNC_ENTER();

    // send a 0x5F (untalk) under control of ATN

    buffer = 0x5f;
    ntStatus = cbmiec_i_raw_write(Pdx, &buffer, 1, &sent, 1, 0);

    FUNC_LEAVE_NTSTATUS(ntStatus);
}
