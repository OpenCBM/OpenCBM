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
** \version $Id: mnib.c,v 1.1.2.1 2005-09-16 12:39:54 strik Exp $ \n
** \authors Based on code from
**    Markus Brenner
** \n
** \brief Nibble a complete track (for mnib)
**
****************************************************************/

#include <wdm.h>
#include "cbm_driver.h"
#include "i_iec.h"

NTSTATUS
cbmiec_mnib_par_read(IN PDEVICE_EXTENSION Pdx, OUT UCHAR* Byte)
{
    FUNC_ENTER();

    CBMIEC_RELEASE(PP_DATA_OUT|PP_CLK_OUT);
    CBMIEC_SET(PP_ATN_OUT);

    cbmiec_udelay(20); /* 200? */
    while(CBMIEC_GET(PP_DATA_IN))
    {
        if (QueueShouldCancelCurrentIrp(&Pdx->IrpQueue))
        {
            FUNC_LEAVE_NTSTATUS_CONST(STATUS_TIMEOUT);
        }
    }

    cbmiec_pp_read(Pdx, Byte);

    cbmiec_udelay(5);
    CBMIEC_RELEASE(PP_ATN_OUT);

    cbmiec_udelay(10);
    while(!CBMIEC_GET(PP_DATA_IN))
    {
        if (QueueShouldCancelCurrentIrp(&Pdx->IrpQueue))
        {
            FUNC_LEAVE_NTSTATUS_CONST(STATUS_TIMEOUT);
        }
    }

    FUNC_LEAVE_NTSTATUS_CONST(STATUS_SUCCESS);
}

NTSTATUS
cbmiec_mnib_par_write(IN PDEVICE_EXTENSION Pdx, IN UCHAR Byte)
{
    UCHAR dummy;
    int j,to;

    FUNC_ENTER();

    CBMIEC_RELEASE(PP_DATA_OUT|PP_CLK_OUT);
    CBMIEC_SET(PP_ATN_OUT);
    cbmiec_udelay(20);

    while(CBMIEC_GET(PP_DATA_IN))
    {
        if (QueueShouldCancelCurrentIrp(&Pdx->IrpQueue))
        {
            FUNC_LEAVE_NTSTATUS_CONST(STATUS_TIMEOUT);
        }
    }

    cbmiec_pp_write(Pdx, Byte);

    cbmiec_udelay(5);
    CBMIEC_RELEASE(PP_ATN_OUT);
    cbmiec_udelay(20);
    while(!CBMIEC_GET(PP_DATA_IN))
    {
        if (QueueShouldCancelCurrentIrp(&Pdx->IrpQueue))
        {
            FUNC_LEAVE_NTSTATUS_CONST(STATUS_TIMEOUT);
        }
    }

    cbmiec_pp_read(Pdx, &dummy);

    FUNC_LEAVE_NTSTATUS_CONST(STATUS_SUCCESS);
}

static void
send_par_cmd(PDEVICE_EXTENSION Pdx, UCHAR cmd)
{
    FUNC_ENTER();

    cbmiec_mnib_par_write(Pdx, 0x00);
    cbmiec_mnib_par_write(Pdx, 0x55);
    cbmiec_mnib_par_write(Pdx, 0xaa);
    cbmiec_mnib_par_write(Pdx, 0xff);
    cbmiec_mnib_par_write(Pdx, cmd);

    FUNC_LEAVE();
}

static int
cbm_nib_read1(PDEVICE_EXTENSION Pdx)
{
    int to;
    int j;

    FUNC_ENTER();

    to = 0;

    CBMIEC_RELEASE(PP_DATA_OUT);

    for (j=0; j < 2; j++)
    {
        CBMIEC_GET(PP_DATA_IN);
    }

    while (CBMIEC_GET(PP_DATA_IN))
    {
        if (to++ > 1000000)
            break;
    }

    return to > 1000000 ? -1 : READ_PORT_UCHAR(PAR_PORT);
}

static int
cbm_nib_read2(PDEVICE_EXTENSION Pdx)
{
    int to;
    int j;

    FUNC_ENTER();

    to = 0;

    CBMIEC_RELEASE(PP_DATA_OUT);

    for (j=0; j < 2; j++)
        CBMIEC_GET(PP_DATA_IN);

    while (!CBMIEC_GET(PP_DATA_IN))
    {
        if (to++ > 1000000)
            break;
    }
    return to > 1000000 ? -1 : READ_PORT_UCHAR(PAR_PORT);
}

static int
cbm_nib_write(PDEVICE_EXTENSION Pdx, char Data, int Toggle)
{
    int to=0;
    int retval;

    FUNC_ENTER();

    CBMIEC_RELEASE(PP_CLK_IN);

    do 
    {
        retval = 1;

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

        cbmiec_pp_write(Pdx, Data);
        retval = 0;

    } while (0);

	FUNC_LEAVE_INT(retval);
}

#define enable()  cbmiec_release_irq()
#define disable() cbmiec_block_irq()

NTSTATUS
cbmiec_mnib_read_track(IN PDEVICE_EXTENSION Pdx, IN UCHAR Mode, OUT UCHAR* Buffer, IN ULONG ReturnLength)
{
    NTSTATUS ntStatus;
	ULONG i;
    UCHAR dummy;

	int	timeout = 0;
	int byte;

    FUNC_ENTER();

	disable();

	send_par_cmd(Pdx, Mode);
	cbmiec_mnib_par_read(Pdx, &dummy);

	for (i = 0; i < ReturnLength; i += 2)
	{
		byte = cbm_nib_read1(Pdx);
		if (byte <= 0)
		{
			timeout = 1;
			break;
		}
		Buffer[i] = (UCHAR) byte;

	    byte = cbm_nib_read2(Pdx);
	    if (byte <= 0)
	    {
			timeout = 1;
			break;
		}
		Buffer[i+1] = (UCHAR) byte;

        if (QueueShouldCancelCurrentIrp(&Pdx->IrpQueue))
        {
            timeout = 1; // FUNC_LEAVE_NTSTATUS_CONST(STATUS_TIMEOUT);
        }
	}

    if(!timeout)
    {
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

    FUNC_LEAVE_NTSTATUS(ntStatus);
}

NTSTATUS
cbmiec_mnib_write_track(IN PDEVICE_EXTENSION Pdx, IN UCHAR Mode, IN UCHAR* Buffer, IN ULONG BufferLength)
{
    NTSTATUS ntStatus;
    UCHAR dummy;

	ULONG i = 0;

	int timeout = 0;

    FUNC_ENTER();

	disable();

  	// send write command
   	send_par_cmd(Pdx, Mode);
	cbmiec_mnib_par_write(Pdx, 0);

	for (i = 0; i < BufferLength; i++)
	{
		if(cbm_nib_write(Pdx, Buffer[i], i&1))
		{
			timeout = 1;
			break;
		}

        if (QueueShouldCancelCurrentIrp(&Pdx->IrpQueue))
        {
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

    FUNC_LEAVE_NTSTATUS(ntStatus);
}
