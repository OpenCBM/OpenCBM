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
** \file sys/libiec/set.c \n
** \author Spiro Trikaliotis \n
** \version $Id: setrelease.c,v 1.2 2005-03-02 18:17:22 strik Exp $ \n
** \authors Based on code from
**    Michael Klein <michael.klein@puffin.lb.shuttle.de>
** \n
** \brief Set a specific line on the IEC bus
**
****************************************************************/

#include <wdm.h>
#include "cbm_driver.h"
#include "i_iec.h"

/*! \brief Set a specific line on the IEC bus

 This functions sets a specific line on the IEC bus.

 \param Pdx
   Pointer to the device extension.

 \param Mask
   The mask of which lines have to be altered at all. Any line
   not mentioned here is left untouched. This has to be a bitwise
   OR between the constants IEC_DATA, IEC_CLOCK, IEC_ATN, and IEC_RESET

 \param Line
   If a line has been set in Mask, the corresponding bit here decides
   if that line is to be set (in this case, it is ORed to this value)
   or released (in this case, the corresponding bit here is 0).

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. Otherwise, it
   returns one of the error status values.
*/
NTSTATUS
cbmiec_iec_setrelease(IN PDEVICE_EXTENSION Pdx, IN USHORT Mask, IN USHORT Line)
{
    NTSTATUS ntStatus;

    FUNC_ENTER();

    FUNC_PARAM((DBG_PREFIX "line = 0x%02x, mask = 0x%02x", Line, Mask));

    ntStatus = STATUS_SUCCESS;

    // Set the correct line as given by the call

    if ( (Line & ~(IEC_LINE_DATA | IEC_LINE_CLOCK | IEC_LINE_ATN | IEC_LINE_RESET))
        || (Mask & ~(IEC_LINE_DATA | IEC_LINE_CLOCK | IEC_LINE_ATN | IEC_LINE_RESET)))
    {
        // there was some bit set that is not recognized, return
        // with an error
        ntStatus = STATUS_INVALID_PARAMETER;
    }
    else
    {

#define SET_RELEASE_LINE(_LineName, _PPName) \
        if (Mask & IEC_LINE_##_LineName) \
        { \
            if (Line & IEC_LINE_##_LineName) \
            { \
                CBMIEC_SET(PP_##_PPName##_OUT); \
            } \
            else \
            { \
                CBMIEC_RELEASE(PP_##_PPName##_OUT); \
            } \
        }

        SET_RELEASE_LINE(DATA,  DATA);
        SET_RELEASE_LINE(CLOCK, CLK);
        SET_RELEASE_LINE(ATN,   ATN);
        SET_RELEASE_LINE(RESET, RESET);

#ifdef TEST_BIDIR

        #define PP_BIDIR_OUT   PP_LP_BIDIR
        #define IEC_LINE_BIDIR PP_BIDIR_OUT

        SET_RELEASE_LINE(BIDIR, BIDIR);

        #undef PP_BIDIR_OUT
        #undef IEC_LINE_BIDIR

#endif // #ifdef TEST_BIDIR

#undef SET_RELEASE_LINE

    }

    FUNC_LEAVE_NTSTATUS(ntStatus );
}
