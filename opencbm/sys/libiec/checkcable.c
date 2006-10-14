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
** \file sys/libiec/checkcable.c \n
** \author Spiro Trikaliotis \n
** \version $Id: checkcable.c,v 1.4 2006-10-14 16:50:57 strik Exp $ \n
** \n
** \brief Check and test the cable type
**
****************************************************************/

#include <wdm.h>
#include "cbm_driver.h"
#include "i_iec.h"

/*! \internal \brief Check if the cable works at all

 This function tries to find out if there is a cable connected,
 and if the type given is the right one.

 \param Pdx
   Pointer to the device extension.

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. Otherwise, it
   returns one of the error status values.

 \todo
   Do a more sophisticated test
*/
static NTSTATUS
cbmiec_testcable(PDEVICE_EXTENSION Pdx) 
{
    NTSTATUS ntStatus = STATUS_PORT_DISCONNECTED;
    UCHAR ch;

    FUNC_ENTER();

    /* check if the state of all lines is correct */

    ch = READ_PORT_UCHAR(IN_PORT);

    DBG_PRINT((DBG_PREFIX "############ Status: out: $%02x, in: $%02x ($%02x ^ $%02x)",
        READ_PORT_UCHAR(OUT_PORT), ch, ch ^ Pdx->IecOutEor, Pdx->IecOutEor));

#define READ(_x) ((((READ_PORT_UCHAR(OUT_PORT) ^ Pdx->IecOutEor)) & (_x)) ? 1 : 0)

#define SHOW(_x, _y) 
    // DBG_PRINT((DBG_PREFIX "CBMIEC_GET(" #_x ") = $%02x, READ(" #_y ") = $%02x", CBMIEC_GET(_x), READ(_y) ));

#define SHOW1() \
    DBG_PRINT((DBG_PREFIX "############ ATN OUT = %u, CLOCK OUT = %u, DATA OUT = %u, RESET OUT = %u", \
        READ(PP_ATN_OUT), READ(PP_CLK_OUT), READ(PP_DATA_OUT), READ(PP_RESET_OUT) )); \
\
    DBG_PRINT((DBG_PREFIX "############ ATN IN  = %u, CLOCK IN  = %u, DATA IN  = %u, RESET IN  = %u", \
        CBMIEC_GET(PP_ATN_IN), CBMIEC_GET(PP_CLK_IN), CBMIEC_GET(PP_DATA_IN), CBMIEC_GET(PP_RESET_IN) ));

    do {
        /* 
         * Do some tests
         */

        /* First of all: If a line is set by me, it must be set when reading, too. */

        if (READ(PP_RESET_OUT) && CBMIEC_GET(PP_RESET_IN) == 0)
        {
            DBG_PRINT((DBG_PREFIX "RESET does not follow"));
            break;
        }

        if (READ(PP_ATN_OUT) && CBMIEC_GET(PP_ATN_IN) == 0)
        {
            DBG_PRINT((DBG_PREFIX "ATN does not follow"));
            break;
        }

        if (READ(PP_DATA_OUT) && CBMIEC_GET(PP_DATA_IN) == 0)
        {
            DBG_PRINT((DBG_PREFIX "DATA does not follow"));
            break;
        }

        if (READ(PP_CLK_OUT) && CBMIEC_GET(PP_CLK_IN) == 0)
        {
            DBG_PRINT((DBG_PREFIX "CLOCK does not follow"));
            break;
        }


        if (Pdx->DoNotReleaseBus)
        {
            DBG_PRINT((DBG_PREFIX "Pdx->DoNotReleaseBus set, skipping extra tests."));

            ntStatus = STATUS_SUCCESS;
            break;
        }

        /* Release all lines */

        CBMIEC_RELEASE(PP_ATN_OUT | PP_RESET_OUT | PP_CLK_OUT | PP_DATA_OUT);
        cbmiec_schedule_timeout(1000); /* wait 1 ms */

DBG_PRINT((DBG_PREFIX " --- Release all lines" ));
SHOW1();
        cbmiec_wait_for_drives_ready(Pdx);
DBG_PRINT((DBG_PREFIX " --- Release all lines - 1" ));
SHOW1();

        /* Now, check if all lines are unset */

        if (CBMIEC_GET(PP_RESET_IN))
        {
            DBG_PRINT((DBG_PREFIX "RESET is set, but it should not"));
            break;
        }

        if (CBMIEC_GET(PP_ATN_IN))
        {
            DBG_PRINT((DBG_PREFIX "ATN is set, but it should not"));
            break;
        }

        if (CBMIEC_GET(PP_DATA_IN))
        {
            DBG_PRINT((DBG_PREFIX "DATA is set, but it should not"));
            break;
        }

        if (CBMIEC_GET(PP_CLK_IN))
        {
            DBG_PRINT((DBG_PREFIX "CLOCK is set, but it should not"));
            break;
        }

        /* Set ATN and wait 1ms for the drive to react */

        CBMIEC_SET(PP_ATN_OUT);

DBG_PRINT((DBG_PREFIX " --- ATN set" ));
SHOW1();

        if (CBMIEC_GET(PP_RESET_IN))
        {
            DBG_PRINT((DBG_PREFIX "RESET is reacting to ATN - most probably, it is an XE1541 cable!"));
            Pdx->IecCable = IEC_CABLETYPE_XE;
        }
        else
        {
            if (CBMIEC_GET(PP_DATA_IN) == 0)
            {
                DBG_PRINT((DBG_PREFIX "DATA does not react to ATN."));
            }
        }

        /* Release ATN again */

        CBMIEC_RELEASE(PP_ATN_OUT);

#undef SHOW
#undef SHOW1

        ntStatus = STATUS_SUCCESS;

    } while (0);

#undef READ

    FUNC_LEAVE_NTSTATUS(ntStatus);
}

/*! \brief Determine the type of cable (XA1541/XM1541) on the IEC bus

 This function tries to determine the type of cable with which
 the IEC bus is connected to the PC's parallel port. Afterwards,
 some variables in the device extension are initialized to reflect
 the type.

 \param Pdx
   Pointer to the device extension.

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. Otherwise, it
   returns one of the error status values.
*/
NTSTATUS
cbmiec_checkcable(PDEVICE_EXTENSION Pdx) 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    const wchar_t *msgAuto = L"";
    const wchar_t *msgCable;
    UCHAR in, out;
    IEC_CABLETYPE iecCableType;

    FUNC_ENTER();

DBG_PRINT((DBG_PREFIX "IecCableUserSet = %d, IecCable = %d", Pdx->IecCableUserSet, Pdx->IecCable));
    do {
        CABLESTATE newCableState = CABLESTATE_UNKNOWN;

        /*
         * If the cabletype is still tested, do not retest again
         */

        DBG_PRINT((DBG_PREFIX "*****************" ));
        DBG_PRINT((DBG_PREFIX "cbmiec_checkcable" ));
        DBG_PRINT((DBG_PREFIX "*****************" ));

        switch (Pdx->IecCableUserSet)
        {
        case IEC_CABLETYPE_XE:
            /* FALL THROUGH */

        case IEC_CABLETYPE_XM:
            /* FALL THROUGH */

        case IEC_CABLETYPE_XA:

            /* the user specified a cable type, use this 
             * without questioning:
             */

            iecCableType = Pdx->IecCableUserSet;

            newCableState = CABLESTATE_TESTED;
            break;

        default:
            in = CBMIEC_GET(PP_ATN_IN);
            out = (READ_PORT_UCHAR(OUT_PORT) & PP_ATN_OUT) ? 1 : 0;
            iecCableType = (in != out) ? IEC_CABLETYPE_XA : IEC_CABLETYPE_XM;
            msgAuto = L" (auto)";
            newCableState = CABLESTATE_UNKNOWN;
            break;
        }

        /*
         * If the cable type recognized does not match the cable
         * type we think we have, the cable is set to "unknown".
         */

        if (iecCableType != Pdx->IecCable)
        {
            cbmiec_setcablestate(Pdx, CABLESTATE_UNKNOWN);
            Pdx->DoNotReleaseBus = FALSE;
        }

        /*
         * If the cable was at least tested before (and we have no
         * contradiction to think it is still in this state),
         * do not do any more tests.
         */
        if (Pdx->IecCableState >= CABLESTATE_TESTED)
            break;

        /*
         * Remember the cable type we recognized, and the state
         */

        Pdx->IecCable = iecCableType;
        cbmiec_setcablestate(Pdx, newCableState);


        switch (Pdx->IecCable)
        {
        case IEC_CABLETYPE_XE:
            /* \todo XE and XM are distinguished later */

            /* FALL THROUGH */

        case IEC_CABLETYPE_XM:
            Pdx->IecOutEor = 0xc4;
            msgCable = L"passive (XM1541)";
            break;

        case IEC_CABLETYPE_XA:
            Pdx->IecOutEor = 0xcb;
            msgCable = L"active (XA1541)";
            break;
        }

        Pdx->IecInEor = 0x80;

        /* remember the current state of the output bits */

        Pdx->IecOutBits = (READ_PORT_UCHAR(OUT_PORT) ^ Pdx->IecOutEor) 
                          & (PP_DATA_OUT|PP_CLK_OUT|PP_ATN_OUT|PP_RESET_OUT);

        /* Now, test if the cable really works */

        if (Pdx->IecCableState > CABLESTATE_UNKNOWN)
            break;

DBG_PRINT((DBG_PREFIX "using %ws cable%ws", msgCable, msgAuto)); // @@@@TODO

        ntStatus = cbmiec_testcable(Pdx);

        if (NT_SUCCESS(ntStatus))
        {
            const wchar_t *msgExtra = L"";

            if (Pdx->IecCable == IEC_CABLETYPE_XE)
            {
                /* the test found out that we have an XE cable, not an XM! */

                msgCable = L"passive (XE1541)";

                msgExtra = L" (THIS IS NOT SUPPORTED YET!)";
            }

            DBG_SUCCESS((DBG_PREFIX "using %ws cable%ws%s",
                msgCable, msgAuto, msgExtra));

            LogErrorString(Pdx->Fdo, 
                (Pdx->IecCable == IEC_CABLETYPE_XE) ? CBM_IEC_INIT_XE1541 : CBM_IEC_INIT,
                msgCable, msgAuto);

            cbmiec_setcablestate(Pdx, CABLESTATE_TESTED);
        }
        else
        {
            DBG_ERROR((DBG_PREFIX "could not validate that the cable "
                       "used is really a %ws cable%ws",
                       msgCable, msgAuto));

            LogErrorString(Pdx->Fdo, CBM_IEC_INIT_FAIL, msgCable, msgAuto);
        }

/*
        if (Pdx->IecOutBits & PP_RESET_OUT)
        {
           cbmiec_reset(Pdx);
        }
*/

    } while (0);

    /*
     * will be printed even if "start" was not printed!
     */

    DBG_PRINT((DBG_PREFIX "*********************" ));
    DBG_PRINT((DBG_PREFIX "end cbmiec_checkcable" ));
    DBG_PRINT((DBG_PREFIX "*********************" ));

    FUNC_LEAVE_NTSTATUS(ntStatus);
}


/*! \brief Set the current state of the cable detection
*/
VOID
cbmiec_setcablestate(PDEVICE_EXTENSION Pdx, CABLESTATE State)
{
    FUNC_ENTER();

    Pdx->IecCableState = State;

    FUNC_LEAVE();
}
