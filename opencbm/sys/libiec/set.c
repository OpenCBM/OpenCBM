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
** \file sys/libiec/set.c \n
** \author Spiro Trikaliotis \n
** \version $Id: set.c,v 1.1 2004-11-07 11:05:14 strik Exp $ \n
** \authors Based on code from
**    Michael Klein <michael.klein@puffin.lb.shuttle.de>
** \n
** \brief Set a specific line on the IEC bus
**
****************************************************************/

#include <wdm.h>
#include "cbm4win_common.h"
#include "i_iec.h"

/*! \brief Set a specific line on the IEC bus

 This functions sets a specific line on the IEC bus.

 \param Pdx
   Pointer to the device extension.

 \param Line
   Which line has to be set (one of IEC_DATA, IEC_CLOCK, IEC_ATN)

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. Otherwise, it
   returns one of the error status values.
*/
NTSTATUS
cbmiec_iec_set(IN PDEVICE_EXTENSION Pdx, IN USHORT Line)
{
    NTSTATUS ntStatus;

    FUNC_ENTER();

    FUNC_PARAM((DBG_PREFIX "line = 0x%02x", Line));

    ntStatus = STATUS_SUCCESS;

    // Set the correct line as given by the call

    switch (Line)
    {
        case IEC_LINE_DATA:
           CBMIEC_SET(PP_DATA_OUT);
           break;

        case IEC_LINE_CLOCK:
           CBMIEC_SET(PP_CLK_OUT);
           break;

        case IEC_LINE_ATN:
           CBMIEC_SET(PP_ATN_OUT);
           break;

        default:
           ntStatus = STATUS_INVALID_PARAMETER;
           break;
    }
    FUNC_LEAVE_NTSTATUS(ntStatus );
}
