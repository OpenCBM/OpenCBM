/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2004 Spiro Trikaliotis <cbm4win@trikaliotis.net>
 *
 */

/*! ************************************************************** 
** \file sys/libcommon/isr.c \n
** \author Spiro Trikaliotis \n
** \version $Id: isr.c,v 1.2 2004-11-21 16:29:09 strik Exp $ \n
** \n
** \brief The Interrupt Service Routine (ISR) for the parallel port
**
****************************************************************/

#include <wdm.h>
#include "cbm4win_common.h"
#include "iec.h"

/*! \brief Interrupt Service Routine (ISR)

 This is the Interrupt Service Routine for the parallel port.

 \param Interrupt:
   Pointer to the interrupt object.

 \param Pdx:
   The device extension 

 \return 
   If we are responsible for this interrupt, we return TRUE.
   If not, we return FALSE and let Windows try other ISRs (if there are any).
*/
BOOLEAN
cbm_isr(IN PKINTERRUPT Interrupt, IN PVOID Pdx)
{
#if 0

    // This implementation might cause erroneous behaviour,
    // as we cannot guard the debugging memory functions
    // against anything running at an IRQL > DISPATCH_LEVEL.

    FUNC_ENTER();

    // let the libiec library do the hard work

    FUNC_LEAVE_BOOL(cbmiec_interrupt(Pdx));

#else

    BOOLEAN result;
    ULONG dbgFlagsOld;

    // Make sure we do not try to write into the debugging memory
    // as long as we are executing in the ISR

    // Remember the old DbgFlags

    dbgFlagsOld = DbgFlags;

    // Now, unset the flag which tells the system to write into
    // the debugging memory

    DbgFlags &= ~DBGF_DBGMEMBUF;

    // let the libiec library do the hard work

    result = cbmiec_interrupt(Pdx);

    // Restore the debugging flags

    DbgFlags = dbgFlagsOld;

    return result;

#endif
}
