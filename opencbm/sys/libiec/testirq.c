/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2006 Spiro Trikaliotis
 *
 */

/*! ************************************************************** 
** \file sys/libiec/testirq.c \n
** \author Spiro Trikaliotis \n
** \version $Id: testirq.c,v 1.1 2006-05-05 08:19:25 strik Exp $ \n
** \n
** \brief Test for IRQ capabilities
**
****************************************************************/

#include <wdm.h>
#include "cbm_driver.h"
#include "iec.h"
#include "i_iec.h"

/*! \brief Test for IRQ capabilities

 This function tests if the parallel port is able to
 generate interrupts.

 \param Pdx
   Pointer to the device extension of the device to be
   tested.
*/

NTSTATUS
cbmiec_test_irq(IN PDEVICE_EXTENSION Pdx)
{
    NTSTATUS ntStatus;
    LONG ret;
    LONG oldDbgFlags = DbgFlags;

    FUNC_ENTER();

    ntStatus = STATUS_SUCCESS;

    do {
        DbgFlags |= 0x7;
        DbgFlags |= 0x7fffffff;

        //
        // Did we get a an interrupt at all? If not, no need
        // to do ANY test!
        //

        if (!Pdx->ParallelPortAllocatedInterrupt)
        {
            ntStatus = STATUS_BIOS_FAILED_TO_CONNECT_INTERRUPT;
            break;
        }

        //
        // Now, do the test
        //

        DBG_PRINT((DBG_PREFIX "Release all lines"));
        CBMIEC_RELEASE(PP_ATN_OUT | PP_CLK_OUT | PP_DATA_OUT | PP_RESET_OUT);

        DBG_PRINT((DBG_PREFIX "Pdx->IrqCount = 100"));
        ret = InterlockedExchange(&Pdx->IrqCount, 100);
        DBG_ASSERT(ret==0);

        DBG_PRINT((DBG_PREFIX "Allow Interrupts"));
        CBMIEC_SET(PP_LP_IRQ);

        DBG_PRINT((DBG_PREFIX "Set all lines"));
        CBMIEC_SET(PP_ATN_OUT | PP_CLK_OUT | PP_DATA_OUT);

        DBG_PRINT((DBG_PREFIX "Wait 1ms"));
        cbmiec_udelay(1000);

        ret = InterlockedExchange(&Pdx->IrqCount, 100);
        DBG_PRINT((DBG_PREFIX "Pdx->IrqCount = 100, old Value = %u", ret));

        if (ret != 100)
        {
            DBG_ERROR((DBG_PREFIX "Interrupt generated when SETTING"));
            ntStatus = STATUS_UNSUCCESSFUL;
        }

        DBG_PRINT((DBG_PREFIX "Release all lines"));
        CBMIEC_RELEASE(PP_ATN_OUT | PP_CLK_OUT | PP_DATA_OUT);

        DBG_PRINT((DBG_PREFIX "Wait 1ms"));
        cbmiec_udelay(1000);

        DBG_PRINT((DBG_PREFIX "Disallow Interrupts"));
        CBMIEC_RELEASE(PP_LP_IRQ);

        ret = InterlockedExchange(&Pdx->IrqCount, 0);
        DBG_PRINT((DBG_PREFIX "Pdx->IrqCount = 0, old Value = %u", ret));

        if (ret != 99)
        {
            DBG_ERROR((DBG_PREFIX "No interrupt generated when RELEASING"));
            ntStatus = STATUS_NO_SUCH_DEVICE;
        }

    } while (0);

    DbgFlags = oldDbgFlags;

    FUNC_LEAVE_NTSTATUS(ntStatus);
}
