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
** \file sys/libiec/i_rawwrite.c \n
** \author Spiro Trikaliotis \n
** \version $Id: i_rawwrite.c,v 1.2 2004-11-15 16:11:52 strik Exp $ \n
** \authors Based on code from
**    Michael Klein <michael.klein@puffin.lb.shuttle.de>
** \n
** \brief Write some bytes to the IEC bus, internal version
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

 \param Count
   Maximum number of characters to read from the bus.

 \param Sent
   Pointer to the variable which will hold the number of written bytes.

 \param Atn
   If true: Sent the bytes with set ATN; else, with released ATN

 \param Talk
   If true: A talk command is to be sent (some special care has to 
   be taken at the end of the transmission).

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. Otherwise, it
   returns one of the error status values.

 ATN is released on return of this routine
*/
NTSTATUS
cbmiec_i_raw_write(PDEVICE_EXTENSION Pdx, const UCHAR *Buffer, USHORT Count, USHORT *Sent, BOOLEAN Atn, BOOLEAN Talk)
{
    NTSTATUS ntStatus;
    USHORT sent;
    UCHAR c;
    ULONG i;
    LONG ret;

    FUNC_ENTER();

    ntStatus = STATUS_SUCCESS;
    sent = 0;

    Pdx->Eoi = 0;

    ret = InterlockedExchange(&Pdx->IrqCount, 0);
    DBG_ASSERT(ret == 0);

    DBG_IEC((DBG_PREFIX "About to send %d bytes%s", Count, Atn ? " with ATN" : ""));

    if (Atn)
    {
        CBMIEC_SET(PP_ATN_OUT);
    }

    // Signal: We have something to send

    CBMIEC_SET(PP_CLK_OUT);
    CBMIEC_RELEASE(PP_DATA_OUT);

    // Wait for DATA be set by the drive(s)

    for(i=0; (i<libiec_global_timeouts.T_9_Times) && !CBMIEC_GET(PP_DATA_IN); i++)
    {
        cbmiec_udelay(libiec_global_timeouts.T_9_SEND_WAIT_DEVICES_T_AT);
    }

    // cbmiec_show_state(Pdx,"!GET(PP_DATA_IN)");

    // If DATA was not set, there is no device present

    if(!CBMIEC_GET(PP_DATA_IN))
    {
        DBG_ERROR((DBG_PREFIX "no devices found!"));
        CBMIEC_RELEASE(PP_CLK_OUT | PP_ATN_OUT);
        ntStatus = STATUS_NO_SUCH_DEVICE;
    }
    else
    {
        cbmiec_schedule_timeout(libiec_global_timeouts.T_10_SEND_BEFORE_1ST_BYTE);
    }

    while(sent < Count && ntStatus == STATUS_SUCCESS)
    {
        c = *Buffer++;

        PERF_EVENT_WRITE_BYTE_NO(sent);
        PERF_EVENT_WRITE_BYTE(c);

        cbmiec_udelay(libiec_global_timeouts.T_11_SEND_BEFORE_BYTE_DELAY);

        if(CBMIEC_GET(PP_DATA_IN))
        {
            // Wait for the listener
            // We might have to signal an EOI. That is signaled
            // *before* the last byte has been sent.
            // Anyway, if we are sending with ATN asserted, then
            // no EOI signaling is allowed.

            cbmiec_wait_for_listener(Pdx, 
                ((sent == (Count-1)) && (Atn == 0)) ? TRUE : FALSE);

            if(QueueShouldCancelCurrentIrp(&Pdx->IrpQueue))
            {
                ntStatus = STATUS_CANCELLED;
            }
            else
            {
                if(cbmiec_send_byte(Pdx,c))
                {
                    sent++;
                    cbmiec_udelay(libiec_global_timeouts.T_12_SEND_AFTER_BYTE_DELAY);
                }
                else
                {
                    DBG_ERROR((DBG_PREFIX "I/O error on cbmiec_send_byte()"));
                    ntStatus = STATUS_NO_SUCH_DEVICE;
                }
            }
        }
        else
        {
            DBG_ERROR((DBG_PREFIX "device not present"));
            ntStatus = STATUS_NO_SUCH_DEVICE;
        }
    }

#if DBG
    if (ntStatus == STATUS_SUCCESS)
    {
        DBG_SUCCESS((DBG_PREFIX "%d bytes sent", sent));
    }
    else
    {
        DBG_ERROR((DBG_PREFIX "%d bytes sent, Status=%s", sent, DebugNtStatus(ntStatus)));
    }
#endif

    if (ntStatus == STATUS_SUCCESS && Talk)
    {
        // Talk-attention turn around (reverse Talker and Listener)

        cbmiec_block_irq();

        CBMIEC_SET(PP_DATA_OUT);
        CBMIEC_RELEASE(PP_ATN_OUT);

        cbmiec_udelay(libiec_global_timeouts.T_13_SEND_TURN_AROUND_LISTENER_TALKER_T_TK);

        CBMIEC_RELEASE(PP_CLK_OUT);

        cbmiec_release_irq();
    }
    else
    {
        CBMIEC_RELEASE(PP_ATN_OUT);
    }
    cbmiec_udelay(libiec_global_timeouts.T_14_SEND_AT_END_DELAY);

    DBG_ASSERT(Sent != NULL);
    *Sent = sent;

    return ntStatus;
}
