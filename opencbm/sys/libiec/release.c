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
** \file sys/libiec/release.c \n
** \author Spiro Trikaliotis \n
** \version $Id: release.c,v 1.3 2005-03-02 18:17:22 strik Exp $ \n
** \authors Based on code from
**    Michael Klein <michael.klein@puffin.lb.shuttle.de>
** \n
** \brief Release a specific line on the IEC bus
**
****************************************************************/

#include <wdm.h>
#include "cbm_driver.h"
#include "i_iec.h"

/*! \brief Release a specific line on the IEC bus

 This functions releases a specific line on the IEC bus.

 \param Pdx
   Pointer to the device extension.

 \param Line
   Which line has to be released (an OR between IEC_DATA, IEC_CLOCK, IEC_ATN,
   and IEC_RESET)


 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. Otherwise, it
   returns one of the error status values.
*/
NTSTATUS
cbmiec_iec_release(IN PDEVICE_EXTENSION Pdx, IN USHORT Line)
{
    NTSTATUS ntStatus;

    FUNC_ENTER();
    FUNC_PARAM((DBG_PREFIX "line = 0x%02x", Line));

    ntStatus = STATUS_SUCCESS;

    // Set the correct line as given by the call

    if (Line & ~(IEC_LINE_DATA | IEC_LINE_CLOCK | IEC_LINE_ATN | IEC_LINE_RESET))
    {
        // there was some bit set that is not recognized, return
        // with an error
        ntStatus = STATUS_INVALID_PARAMETER;
    }
    else
    {
        if (Line & IEC_LINE_DATA)  CBMIEC_RELEASE(PP_DATA_OUT);
        if (Line & IEC_LINE_CLOCK) CBMIEC_RELEASE(PP_CLK_OUT);
        if (Line & IEC_LINE_ATN)   CBMIEC_RELEASE(PP_ATN_OUT);
        if (Line & IEC_LINE_RESET) CBMIEC_RELEASE(PP_RESET_OUT);
    }

    FUNC_LEAVE_NTSTATUS(ntStatus );
}
