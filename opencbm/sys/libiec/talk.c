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
** \file sys/libiec/talk.c \n
** \author Spiro Trikaliotis \n
** \version $Id: talk.c,v 1.1 2004-11-07 11:05:14 strik Exp $ \n
** \authors Based on code from
**    Michael Klein <michael.klein@puffin.lb.shuttle.de>
** \n
** \brief Send a TALK to the IEC bus
**
****************************************************************/

#include <wdm.h>
#include "cbm4win_common.h"
#include "i_iec.h"

/*! \brief Send a TALK over the IEC bus

 This function sends a TALK to the IEC bus.

 \param Pdx
   Pointer to the device extension.

 \param Device
   Device (primary) address

 \param Secaddr
   Secondary address

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. Otherwise, it
   returns one of the error status values.
*/
NTSTATUS
cbmiec_talk(IN PDEVICE_EXTENSION Pdx, IN UCHAR Device, IN UCHAR Secaddr)
{
    NTSTATUS ntStatus;
    USHORT sent;
    UCHAR buffer[2];

    FUNC_ENTER();

    FUNC_PARAM((DBG_PREFIX "Device = 0x%02x, Secaddr = 0x%02x", (int)Device, (int)Secaddr));

    // send a 0x4x / 0x6y (talk device x, secaddr y) under control of ATN

    buffer[0] = 0x40 | Device;
    buffer[1] = 0x60 | Secaddr;
    ntStatus = cbmiec_i_raw_write(Pdx, buffer, 2, &sent, 1, 1);

    FUNC_LEAVE_NTSTATUS(ntStatus);
}
