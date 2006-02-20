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
** \version $Id: release.c,v 1.5 2006-02-20 12:11:16 strik Exp $ \n
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

 This function releases a specific line on the IEC bus.

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
    FUNC_ENTER();

    FUNC_LEAVE_NTSTATUS(cbmiec_iec_setrelease(Pdx, 0, Line));
}
