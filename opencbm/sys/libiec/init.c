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
** \file sys/libiec/init.c \n
** \author Spiro Trikaliotis \n
** \version $Id: init.c,v 1.1 2004-11-07 11:05:14 strik Exp $ \n
** \authors Based on code from
**    Michael Klein <michael.klein@puffin.lb.shuttle.de>
** \n
** \brief Initialize the IEC bus
**
****************************************************************/

#include <wdm.h>
#include "cbm4win_common.h"
#include "i_iec.h"

/*! timeout values */

IEC_TIMEOUTS libiec_global_timeouts;

/*! Read timeout values from the registry */
#define READ_TIMEOUT_VALUE(_what_, _default_) \
    do { \
        libiec_global_timeouts._what_ = _default_; \
        if (HKey) cbm_registry_read_ulong(*HKey, L#_what_, &libiec_global_timeouts._what_); \
    } while (0)


/*! \internal \brief Initialize the global timeout template

 \param HKey
   Pointer to a handle with holds a registry key from which
   the timeout values are to be read.

   If this is NULL, no access to the registry is performed.
*/
static VOID
cbmiec_timeouts_init(IN PHANDLE HKey)
{
    HANDLE hKey;

    READ_TIMEOUT_VALUE(T_holdreset,   100000); // 100 ms
    READ_TIMEOUT_VALUE(T_afterreset, 2000000); // = 2 s
    READ_TIMEOUT_VALUE(T_1,               20); // us
    READ_TIMEOUT_VALUE(T_2_Times,         80); //! x T_2, \todo 40
    READ_TIMEOUT_VALUE(T_2,               10); // us
    READ_TIMEOUT_VALUE(T_3,               70); // us
    READ_TIMEOUT_VALUE(T_4_Times,        200); //! x T_4, \todo 100
    READ_TIMEOUT_VALUE(T_4,               20); // us
    READ_TIMEOUT_VALUE(T_5_Times,        500); //! x T_5, \todo 200
    READ_TIMEOUT_VALUE(T_5,               10); // us
    READ_TIMEOUT_VALUE(T_6_Times,        100); // x T_6
    READ_TIMEOUT_VALUE(T_6,               20); // us
    READ_TIMEOUT_VALUE(T_7,               70); // us

    READ_TIMEOUT_VALUE(T_8,               20); // us
    READ_TIMEOUT_VALUE(T_9,               10); // us
    READ_TIMEOUT_VALUE(T_9a_Times,       100); // x T_9a
    READ_TIMEOUT_VALUE(T_9a,              10); // us
    READ_TIMEOUT_VALUE(T_10,              20); // us
    READ_TIMEOUT_VALUE(T_11,              50); // us
    READ_TIMEOUT_VALUE(T_12,             100); // us
    READ_TIMEOUT_VALUE(T_13,              20); // us
    READ_TIMEOUT_VALUE(T_14,             100); // us

    // sendbyte related:
    READ_TIMEOUT_VALUE(T_15,              70); // us
    READ_TIMEOUT_VALUE(T_16,              20); // us
    READ_TIMEOUT_VALUE(T_17_Times,        20); // x T_17
    READ_TIMEOUT_VALUE(T_17,             100); // us

    READ_TIMEOUT_VALUE(T_WaitForListener_Granu, 10); // us

}
#undef READ_TIMEOUT_VALUE

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

 \todo
   Do a more sophisticated test
*/
static NTSTATUS
cbmiec_testcable(PDEVICE_EXTENSION Pdx) 
{
    const wchar_t *msgAuto = L"";
    UCHAR in, out;

    FUNC_ENTER();

/*! \todo Do a more sophisticated test for the cable */

    if (Pdx->IecCable > 1)
    {
        in = CBMIEC_GET(PP_ATN_IN);
        out = (READ_PORT_UCHAR(OUT_PORT) & PP_ATN_OUT) ? 1 : 0;
        Pdx->IecCable = (in != out) ? 1 : 0;
        msgAuto = L" (auto)";
    }

    Pdx->IecOutEor = Pdx->IecCable ? 0xcb : 0xc4;

    DBG_SUCCESS((DBG_PREFIX "using %ws cable%ws",
        Pdx->IecCable ? L"active (XA1541)" : L"passive (XM1541)",
        msgAuto));

    LogErrorString(Pdx->Fdo, CBM_IEC_INIT, Pdx->IecCable ? L"XA1541" : L"XM1541", msgAuto);

    Pdx->IecOutBits = (READ_PORT_UCHAR(OUT_PORT) ^ Pdx->IecOutEor) & (PP_DATA_OUT|PP_CLK_OUT|PP_ATN_OUT|PP_RESET_OUT);

/*
    if (Pdx->IecOutBits & PP_RESET_OUT)
    {
       cbmiec_reset(Pdx);
    }
*/

    FUNC_LEAVE_NTSTATUS_CONST(STATUS_SUCCESS);
}


/*! \brief Cleanup the IEC bus

 This function cleans the IEC bus immediately before it is released.

 \param Pdx
   Pointer to the device extension.

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. Otherwise, it
   returns one of the error status values.
*/
NTSTATUS
cbmiec_cleanup(IN PDEVICE_EXTENSION Pdx)
{
    FUNC_ENTER();

    cbmiec_release_bus(Pdx);

    FUNC_LEAVE_NTSTATUS_CONST(STATUS_SUCCESS);
}

/*! \brief Initialize the IEC bus

 This function initializes the IEC bus itself, and sets some
 variables in the device extension. It has to be called before
 any other IEC function is called.

 \param Pdx
   Pointer to the device extension.

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. Otherwise, it
   returns one of the error status values.
*/
NTSTATUS
cbmiec_init(IN PDEVICE_EXTENSION Pdx)
{
    NTSTATUS ntStatus;

    FUNC_ENTER();

    // Initialize the event which is used to wake up the
    // task in wait_for_listener()

    DBG_IRQL( == PASSIVE_LEVEL);
    KeInitializeEvent(&Pdx->EventWaitForListener, SynchronizationEvent, FALSE);

#ifdef USE_DPC

    // Initialize the DPC object which will be used for waking
    // up cbmiec_wait_for_listener() later

    DBG_IRQL( == PASSIVE_LEVEL)
    IoInitializeDpcRequest(Pdx->Fdo, cbmiec_dpc);

#endif // #ifdef USE_DPC

    /* auto-detect the cable */
    Pdx->IecCable = 2;

    ntStatus = cbmiec_testcable(Pdx);

    if (!NT_SUCCESS(ntStatus)) {
        FUNC_LEAVE_NTSTATUS(ntStatus);
    }

    Pdx->IecBusy = FALSE;

    CBMIEC_RELEASE(PP_RESET_OUT | PP_DATA_OUT | PP_ATN_OUT | PP_LP_BIDIR | PP_LP_IRQ);
    CBMIEC_SET(PP_CLK_OUT);

    FUNC_LEAVE_NTSTATUS_CONST(STATUS_SUCCESS);
}

/*! \brief Initialization for libiec which are global in nature

 This function initializes libiec.

 \param HKey
   Pointer to a handle with holds a registry key.
   If this is NULL, no access to the registry is performed.

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. Otherwise, it
   returns one of the error status values.
*/
NTSTATUS
cbmiec_global_init(IN PHANDLE HKey)
{
    FUNC_ENTER();

    //Initialize the timeout values

    cbmiec_timeouts_init(HKey);

    FUNC_LEAVE_NTSTATUS_CONST(STATUS_SUCCESS);
}
