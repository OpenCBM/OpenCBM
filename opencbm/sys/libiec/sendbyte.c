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
** \file sys/libiec/sendbyte.c \n
** \author Spiro Trikaliotis \n
** \version $Id: sendbyte.c,v 1.1 2004-11-07 11:05:14 strik Exp $ \n
** \authors Based on code from
**    Michael Klein <michael.klein@puffin.lb.shuttle.de>
** \n
** \brief Write one byte to the IEC bus
**
****************************************************************/

#include <wdm.h>
#include "cbm4win_common.h"
#include "i_iec.h"

/*! \brief Write one byte to the IEC bus

 \param Pdx
   Pointer to the device extension.

 \param Byte
   The byte to be output

 \return 
   If the routine succeeds - that is, the listener acknowledged 
   the byte -, it returns TRUE. Otherwise, it returns FALSE.
*/
BOOLEAN
cbmiec_send_byte(IN PDEVICE_EXTENSION Pdx, IN UCHAR Byte)
{
    BOOLEAN ack;
    ULONG i;

    FUNC_ENTER();

    DBG_SUCCESS((DBG_PREFIX "send_byte %02x", Byte));
    
    ack = FALSE;

    cbmiec_block_irq();

    for(i = 0; i<8; i++)
    {
        PERF_EVENT_WRITE_BIT_NO(i);

        cbmiec_udelay(libiec_global_timeouts.T_15);

        if(!((Byte>>i) & 1))
        {
            CBMIEC_SET(PP_DATA_OUT);
        }
        CBMIEC_RELEASE(PP_CLK_OUT);
        cbmiec_udelay(libiec_global_timeouts.T_16);
        CBMIEC_SET_RELEASE(PP_CLK_OUT, PP_DATA_OUT);
    }

    cbmiec_release_irq();

    for(i=0; (i<libiec_global_timeouts.T_17_Times) && !(ack=CBMIEC_GET(PP_DATA_IN)); i++)
    {
        cbmiec_udelay(libiec_global_timeouts.T_17);
    }

    DBG_SUCCESS((DBG_PREFIX "ack=%s", ack ? "TRUE" : "FALSE" ));

    FUNC_LEAVE_BOOL(ack);
}
