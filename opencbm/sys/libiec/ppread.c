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
** \file sys/libiec/ppread.c \n
** \author Spiro Trikaliotis \n
** \version $Id: ppread.c,v 1.3 2005-07-16 17:20:42 strik Exp $ \n
** \authors Based on code from
**    Michael Klein <michael.klein@puffin.lb.shuttle.de>
** \n
** \brief Read a byte from the X[M|A]P1541-cable
**
****************************************************************/

#include <wdm.h>
#include "cbm_driver.h"
#include "i_iec.h"

/*! \brief Read a byte from the X[M|A]P1541 cable

 This function reads a byte from the parallel portion of
 the X[M|A]P1541 cable.

 \param Pdx
   Pointer to the device extension.

 \param Return
   Pointer to an UCHAR where the read byte is written to.

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. Otherwise, it
   returns one of the error status values.
*/
NTSTATUS
cbmiec_pp_read(IN PDEVICE_EXTENSION Pdx, OUT UCHAR *Return)
{
    FUNC_ENTER();

    if (!(Pdx->IecOutBits & PP_LP_BIDIR))
    {
        WRITE_PORT_UCHAR(PAR_PORT, 0xFF);
        CBMIEC_SET(PP_LP_BIDIR);
    }

    *Return = READ_PORT_UCHAR(PAR_PORT);

    FUNC_LEAVE_NTSTATUS_CONST(STATUS_SUCCESS);
}
