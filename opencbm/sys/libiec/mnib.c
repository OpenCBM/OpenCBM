/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2000-2005 Markus Brenner
 *  Copyright 2000-2005 Pete Rittwage
 *  Copyright 2005      Tim Schürmann
 *  Copyright 2005      Spiro Trikaliotis
 *
 */

/*! ************************************************************** 
** \file sys/libiec/mnib.c \n
** \author Tim Schürmann, Spiro Trikaliotis \n
** \version $Id: mnib.c,v 1.5 2006-03-09 17:31:35 strik Exp $ \n
** \authors Based on code from
**    Markus Brenner
** \n
** \brief Nibble a complete track (for mnib)
**
****************************************************************/

#include <wdm.h>
#include "cbm_driver.h"
#include "i_iec.h"

#define PERF_EVENT_MNIB_PAR_READ_ENTER()           PERF_EVENT(0x5000, 0)
#define PERF_EVENT_MNIB_PAR_READ_DELAY1(_x_)       PERF_EVENT(0x5001, _x_)
#define PERF_EVENT_MNIB_PAR_READ_PP_READ()         PERF_EVENT(0x5002, 0)
#define PERF_EVENT_MNIB_PAR_READ_RELEASED(_x_)     PERF_EVENT(0x5003, _x_)
#define PERF_EVENT_MNIB_PAR_READ_DELAY2(_x_)       PERF_EVENT(0x5004, _x_)
#define PERF_EVENT_MNIB_PAR_READ_TIMEOUT(_x_)      PERF_EVENT(0x5005, _x_)
#define PERF_EVENT_MNIB_PAR_READ_EXIT(_x_)         PERF_EVENT(0x5006, _x_)

#define PERF_EVENT_MNIB_PAR_WRITE_ENTER()          PERF_EVENT(0x5100, 0)
#define PERF_EVENT_MNIB_PAR_WRITE_DELAY1(_x_)      PERF_EVENT(0x5101, _x_)
#define PERF_EVENT_MNIB_PAR_WRITE_PP_WRITE(_x_)    PERF_EVENT(0x5102, _x_)
#define PERF_EVENT_MNIB_PAR_WRITE_RELEASE()        PERF_EVENT(0x5103, 0)
#define PERF_EVENT_MNIB_PAR_WRITE_DELAY2(_x_)      PERF_EVENT(0x5104, _x_)
#define PERF_EVENT_MNIB_PAR_WRITE_DUMMY_READ(_x_)  PERF_EVENT(0x5105, _x_)

#define PERF_EVENT_MNIB_SEND_CMD(_x_)              PERF_EVENT(0x5200, _x_)

#define PERF_EVENT_MNIB_NIB_READ_RELEASE()         PERF_EVENT(0x5300, 0)
#define PERF_EVENT_MNIB_NIB_READ_AFTERDELAY()      PERF_EVENT(0x5301, 0)
#define PERF_EVENT_MNIB_NIB_READ_EXIT(_x_)         PERF_EVENT(0x5302, _x_)

#define PERF_EVENT_MNIB_NIB_WRITE_RELEASE()        PERF_EVENT(0x5400, 0)
#define PERF_EVENT_MNIB_NIB_WRITE_VALUE(_x_)       PERF_EVENT(0x5401, _x_)
#define PERF_EVENT_MNIB_NIB_WRITE_EXIT(_x_)        PERF_EVENT(0x5402, _x_)

#define PERF_EVENT_MNIB_READ_TRACK_ENTER()         PERF_EVENT(0x5500, 0)
#define PERF_EVENT_MNIB_READ_TRACK_STARTLOOP()     PERF_EVENT(0x5500, 0)
#define PERF_EVENT_MNIB_READ_TRACK_VALUE(_x_)      PERF_EVENT(0x5500, _x_)
#define PERF_EVENT_MNIB_READ_TRACK_TIMEOUT(_x_)    PERF_EVENT(0x5500, _x_)
#define PERF_EVENT_MNIB_READ_TRACK_READ_DUMMY(_x_) PERF_EVENT(0x5500, _x_)
#define PERF_EVENT_MNIB_READ_TRACK_EXIT(_x_)       PERF_EVENT(0x5500, _x_)

#define PERF_EVENT_MNIB_WRITE_TRACK_ENTER()        PERF_EVENT(0x5500, 0)
#define PERF_EVENT_MNIB_WRITE_TRACK_STARTLOOP()    PERF_EVENT(0x5500, 0)
#define PERF_EVENT_MNIB_WRITE_TRACK_VALUE(_x_)     PERF_EVENT(0x5500, _x_)
#define PERF_EVENT_MNIB_WRITE_TRACK_TIMEOUT(_x_)   PERF_EVENT(0x5500, _x_)
#define PERF_EVENT_MNIB_WRITE_TRACK_EXIT(_x_)      PERF_EVENT(0x5500, _x_)

NTSTATUS
cbmiec_mnib_par_read(IN PDEVICE_EXTENSION Pdx, OUT UCHAR* Byte)
{
    FUNC_ENTER();

    PERF_EVENT_MNIB_PAR_READ_ENTER();

    CBMIEC_RELEASE(PP_DATA_OUT|PP_CLK_OUT);
    CBMIEC_SET(PP_ATN_OUT);

    PERF_EVENT_MNIB_PAR_READ_DELAY1(0);
    cbmiec_udelay(200);
    PERF_EVENT_MNIB_PAR_READ_DELAY1(1);

    while(CBMIEC_GET(PP_DATA_IN))
    {
        if (QueueShouldCancelCurrentIrp(&Pdx->IrpQueue))
        {
            PERF_EVENT_MNIB_PAR_READ_TIMEOUT(0);
            FUNC_LEAVE_NTSTATUS_CONST(STATUS_TIMEOUT);
        }
    }

    PERF_EVENT_MNIB_PAR_READ_PP_READ();
    cbmiec_pp_read(Pdx, Byte);

    cbmiec_udelay(5);
    PERF_EVENT_MNIB_PAR_READ_RELEASED(0);
    CBMIEC_RELEASE(PP_ATN_OUT);
    PERF_EVENT_MNIB_PAR_READ_RELEASED(1);

    PERF_EVENT_MNIB_PAR_READ_DELAY2(0);
    cbmiec_udelay(10);
    PERF_EVENT_MNIB_PAR_READ_DELAY2(1);
    while(!CBMIEC_GET(PP_DATA_IN))
    {
        if (QueueShouldCancelCurrentIrp(&Pdx->IrpQueue))
        {
            PERF_EVENT_MNIB_PAR_READ_TIMEOUT(1);
            FUNC_LEAVE_NTSTATUS_CONST(STATUS_TIMEOUT);
        }
    }

    PERF_EVENT_MNIB_PAR_READ_EXIT(*Byte);
    FUNC_LEAVE_NTSTATUS_CONST(STATUS_SUCCESS);
}

NTSTATUS
cbmiec_mnib_par_write(IN PDEVICE_EXTENSION Pdx, IN UCHAR Byte)
{
    UCHAR dummy;
    int j,to;

    FUNC_ENTER();

    PERF_EVENT_MNIB_PAR_WRITE_ENTER();

    CBMIEC_RELEASE(PP_DATA_OUT|PP_CLK_OUT);
    CBMIEC_SET(PP_ATN_OUT);
    PERF_EVENT_MNIB_PAR_WRITE_DELAY1(0);
    cbmiec_udelay(200);
    PERF_EVENT_MNIB_PAR_WRITE_DELAY1(1);

    while(CBMIEC_GET(PP_DATA_IN))
    {
        if (QueueShouldCancelCurrentIrp(&Pdx->IrpQueue))
        {
            FUNC_LEAVE_NTSTATUS_CONST(STATUS_TIMEOUT);
        }
    }

    PERF_EVENT_MNIB_PAR_WRITE_PP_WRITE(Byte);
    cbmiec_pp_write(Pdx, Byte);

    cbmiec_udelay(5);
    CBMIEC_RELEASE(PP_ATN_OUT);
    PERF_EVENT_MNIB_PAR_WRITE_DELAY2(0);
    cbmiec_udelay(20);
    PERF_EVENT_MNIB_PAR_WRITE_DELAY2(1);

    while(!CBMIEC_GET(PP_DATA_IN))
    {
        if (QueueShouldCancelCurrentIrp(&Pdx->IrpQueue))
        {
            FUNC_LEAVE_NTSTATUS_CONST(STATUS_TIMEOUT);
        }
    }

    PERF_EVENT_MNIB_PAR_WRITE_DUMMY_READ(0);
    cbmiec_pp_read(Pdx, &dummy);
    PERF_EVENT_MNIB_PAR_WRITE_DUMMY_READ(dummy);

    FUNC_LEAVE_NTSTATUS_CONST(STATUS_SUCCESS);
}

static void
cbm_mnib_send_cmd(PDEVICE_EXTENSION Pdx, UCHAR cmd)
{
    FUNC_ENTER();

    PERF_EVENT_MNIB_SEND_CMD(0);
    cbmiec_mnib_par_write(Pdx, 0x00);
    PERF_EVENT_MNIB_SEND_CMD(1);
    cbmiec_mnib_par_write(Pdx, 0x55);
    PERF_EVENT_MNIB_SEND_CMD(2);
    cbmiec_mnib_par_write(Pdx, 0xaa);
    PERF_EVENT_MNIB_SEND_CMD(3);
    cbmiec_mnib_par_write(Pdx, 0xff);
    PERF_EVENT_MNIB_SEND_CMD(4);
    cbmiec_mnib_par_write(Pdx, cmd);
    PERF_EVENT_MNIB_SEND_CMD(5);

    FUNC_LEAVE();
}

static int
cbm_nib_read(PDEVICE_EXTENSION Pdx, int Toggle)
{
    int to = 0;
    int j;

    FUNC_ENTER();

    PERF_EVENT_MNIB_NIB_READ_RELEASE();
    CBMIEC_RELEASE(PP_DATA_OUT); // @@@ DATA_IN ???

    cbmiec_udelay(2); 
    PERF_EVENT_MNIB_NIB_READ_AFTERDELAY();

    if (!Toggle)
    {
        while (CBMIEC_GET(PP_DATA_IN))
        {
            if (to++ > 1000000)
                break;
        }
    }
    else
    {
        while (!CBMIEC_GET(PP_DATA_IN))
        {
            if (to++ > 1000000)
                break;
        }
    }

    PERF_EVENT_MNIB_NIB_READ_EXIT(to > 1000000 ? -1 : 0);
    return to > 1000000 ? -1 : READ_PORT_UCHAR(PAR_PORT);
}

static int
cbm_nib_write(PDEVICE_EXTENSION Pdx, char Data, int Toggle)
{
    int to=0;
    int retval;

    FUNC_ENTER();

    PERF_EVENT_MNIB_NIB_WRITE_RELEASE();
    CBMIEC_RELEASE(PP_CLK_IN);

    do 
    {
        retval = 1;

        if (!Toggle)
        {
            while (CBMIEC_GET(PP_DATA_IN)) // @@@ CLK_IN ???
            {
                if (to++ > 1000000)
                    break;
            }
        }
        else
        {
            while (!CBMIEC_GET(PP_DATA_IN)) // @@@ CLK_IN ???
            {
                if (to++ > 1000000)
                    break;
            }
        }

        if (to++ <= 1000000)
        {
            PERF_EVENT_MNIB_NIB_WRITE_VALUE(Data);
            cbmiec_pp_write(Pdx, Data);
            retval = 0; // @@@ retval = 1 ???
        }

    } while (0);

    PERF_EVENT_MNIB_NIB_WRITE_EXIT(retval);
    FUNC_LEAVE_INT(retval);
}

#define enable()  cbmiec_release_irq(Pdx)
#define disable() cbmiec_block_irq(Pdx)

NTSTATUS
cbmiec_mnib_read_track(IN PDEVICE_EXTENSION Pdx, OUT UCHAR* Buffer, IN ULONG ReturnLength)
{
    NTSTATUS ntStatus;
    ULONG i;
    UCHAR dummy;

    int timeout = 0;
    int byte;

    FUNC_ENTER();

    PERF_EVENT_MNIB_READ_TRACK_ENTER();

    disable();

    PERF_EVENT_MNIB_READ_TRACK_STARTLOOP();

    for (i = 0; i < ReturnLength; i ++)
    {
        byte = cbm_nib_read(Pdx, i&1);
        PERF_EVENT_MNIB_READ_TRACK_VALUE(byte);
        if (byte == -1)
        {
            PERF_EVENT_MNIB_READ_TRACK_TIMEOUT(0);
            timeout = 1;
            break;
        }
        Buffer[i] = (UCHAR) byte;

        if (QueueShouldCancelCurrentIrp(&Pdx->IrpQueue))
        {
            PERF_EVENT_MNIB_READ_TRACK_TIMEOUT(1);
            timeout = 1; // FUNC_LEAVE_NTSTATUS_CONST(STATUS_TIMEOUT);
        }
    }

    if(!timeout)
    {
        cbmiec_mnib_par_read(Pdx, &dummy);
        PERF_EVENT_MNIB_READ_TRACK_READ_DUMMY(dummy);
        enable();
        ntStatus = STATUS_SUCCESS;
    }
    else
    {
        enable();
        DBG_PRINT((DBG_PREFIX "timeout failure!"));
        ntStatus = STATUS_TIMEOUT;
    }

    PERF_EVENT_MNIB_READ_TRACK_EXIT(ntStatus);

    FUNC_LEAVE_NTSTATUS(ntStatus);
}

NTSTATUS
cbmiec_mnib_write_track(IN PDEVICE_EXTENSION Pdx, IN UCHAR* Buffer, IN ULONG BufferLength)
{
    NTSTATUS ntStatus;
    UCHAR dummy;

    ULONG i = 0;

    int timeout = 0;

    FUNC_ENTER();

    PERF_EVENT_MNIB_WRITE_TRACK_ENTER();

    disable();

    PERF_EVENT_MNIB_WRITE_TRACK_STARTLOOP();

    for (i = 0; i < BufferLength; i++)
    {
        PERF_EVENT_MNIB_WRITE_TRACK_VALUE(Buffer[i]);
        if(cbm_nib_write(Pdx, Buffer[i], i&1))
        {
            PERF_EVENT_MNIB_WRITE_TRACK_TIMEOUT(0);
            timeout = 1;
            break;
        }

        if (QueueShouldCancelCurrentIrp(&Pdx->IrpQueue))
        {
            PERF_EVENT_MNIB_WRITE_TRACK_TIMEOUT(1);
            timeout = 1; // FUNC_LEAVE_NTSTATUS_CONST(STATUS_TIMEOUT);
        }
    }

    if(!timeout)
    {
        cbm_nib_write(Pdx, 0, i&1);
        cbmiec_mnib_par_read(Pdx, &dummy);
        enable();
        ntStatus = STATUS_SUCCESS;
    }
    else
    {
        enable();
        DBG_PRINT((DBG_PREFIX "timeout failure!"));
        ntStatus = STATUS_TIMEOUT;
    }

    PERF_EVENT_MNIB_WRITE_TRACK_EXIT(ntStatus);

    FUNC_LEAVE_NTSTATUS(ntStatus);
}
