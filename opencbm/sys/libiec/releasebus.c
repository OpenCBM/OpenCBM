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
** \file sys/libiec/releasebus.c \n
** \author Spiro Trikaliotis \n
** \version $Id: releasebus.c,v 1.3 2005-07-16 17:20:42 strik Exp $ \n
** \authors Based on code from
**    Michael Klein <michael.klein@puffin.lb.shuttle.de>
** \n
** \brief Release the IEC bus
**
****************************************************************/

#include <wdm.h>
#include "cbm_driver.h"
#include "i_iec.h"

/*! \brief Release the IEC bus

 This function releases the IEC bus.

 \param Pdx
   Pointer to the device extension.

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. Otherwise, it
   returns one of the error status values.
*/
VOID
cbmiec_release_bus(IN PDEVICE_EXTENSION Pdx)
{
    FUNC_ENTER();

    CBMIEC_RELEASE(PP_RESET_OUT | PP_ATN_OUT | PP_DATA_OUT | PP_CLK_OUT);

    FUNC_LEAVE();
}
