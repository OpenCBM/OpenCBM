/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2004 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Copyright 2001-2004 Spiro Trikaliotis
 *
 */

/*! ************************************************************** 
** \file sys/libiec/reset.c \n
** \author Spiro Trikaliotis \n
** \version $Id: reset.c,v 1.4 2006-02-24 12:21:43 strik Exp $ \n
** \authors Based on code from
**    Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
** \n
** \brief Send a RESET on the IEC bus
**
****************************************************************/

#include <wdm.h>
#include "cbm_driver.h"
#include "i_iec.h"

/*! \brief Send a RESET to the IEC bus

 This function sends a RESET on the IEC bus.

 \param Pdx
   Pointer to the device extension.

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. Otherwise, it
   returns one of the error status values.
*/
NTSTATUS
cbmiec_reset(IN PDEVICE_EXTENSION Pdx)
{
    FUNC_ENTER();

    CBMIEC_RELEASE(PP_DATA_OUT | PP_ATN_OUT | PP_CLK_OUT | PP_LP_BIDIR | PP_LP_IRQ);

    CBMIEC_SET(PP_RESET_OUT);
    cbmiec_schedule_timeout(libiec_global_timeouts.T_holdreset);
    CBMIEC_RELEASE(PP_RESET_OUT);

    DBG_SUCCESS((DBG_PREFIX "sleeping after RESET..."));

/*
    cbmiec_schedule_timeout(libiec_global_timeouts.T_afterreset);
    CBMIEC_SET(PP_CLK_OUT);
*/
    {
        int i=1;

        while (1) {
            IEC_CHECKDEVICE check_device;

            cbmiec_check_device(Pdx, &check_device);

#if 0
            switch (check_device)
            {
            case IEC_CHECKDEVICE_BUSFREE:
                DBG_PRINT((DBG_PREFIX "%u: Bus is free!", i));
                break;

            case IEC_CHECKDEVICE_BUSBUSY:
                DBG_PRINT((DBG_PREFIX "%u: Bus is busy.", i));
                break;

            case IEC_CHECKDEVICE_NODEVICE:
                DBG_PRINT((DBG_PREFIX "%u: No device.", i));
                break;

            default:
                DBG_PRINT((DBG_PREFIX "%u: UNKNOWN", i));
                break;
            }
#endif

            if (check_device == IEC_CHECKDEVICE_BUSFREE)
                break;

            ++i;

            if (i == 1000)
            {
                DBG_PRINT((DBG_PREFIX "Quiting because i has reached %u", i));
                break;
            }
            cbmiec_schedule_timeout(1000);
        }
    }

    FUNC_LEAVE_NTSTATUS_CONST(STATUS_SUCCESS);
}
