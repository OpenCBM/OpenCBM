/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2004 Spiro Trikaliotis
*/

/*! ************************************************************** 
** \file sys/vdd/dll/vdd.c \n
** \author Spiro Trikaliotis \n
** \version $Id: vdd.c,v 1.1 2004-12-22 14:43:22 strik Exp $ \n
** \n
** \brief VDD for accessing the driver from DOS
**
****************************************************************/

// #define UNICODE 1

#include <windows.h>
#include <vddsvc.h>

#include <opencbm.h>

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! Mark: We are building a DLL */
#define DBG_DLL

/*! The name of the executable */
#define DBG_PROGNAME "OPENCBMVDD.DLL"

/*! This file is "like" debug.c, that is, define some variables */
#define DBG_IS_DEBUG_C

#include "debug.h"

#include "vdd.h"

#include <stdlib.h>


/*! we are exporting the functions */
#undef EXTERN
#define EXTERN __declspec(dllexport) 

EXTERN BOOL VDDInitialize(IN HANDLE Module, IN DWORD Reason, IN LPVOID Reserved);
EXTERN VOID VDDRegisterInit(VOID);
EXTERN VOID VDDDispatch(VOID);

/*! \brief VDD initialization und unloading

 This function is called whenever the VDD is loaded or unloaded.
 It ensures that the driver is loaded to be able to call its
 functions.

 \param Module
   Handle of the module; this is not used.

 \param Reason
   DLL_PROCESS_ATTACH if the DLL is loaded,
   DLL_PROCESS_DETACH if it is unloaded.

 \param Reserved
   Not used.

 \return 
   Returns TRUE on success, else FALSE.
*/

BOOL
VDDInitialize(IN HANDLE Module, IN DWORD Reason, IN LPVOID Reserved)
{
    FUNC_ENTER();

    DBG_PRINT((DBG_PREFIX "OpencbmVDD.Entry: " __DATE__ " " __TIME__));

    DbgFlags |= DBGF_PARAM;

    switch (Reason) 
    {
        case DLL_PROCESS_ATTACH:
            DBG_PRINT((DBG_PREFIX "ATTACH"));
            break;

        case DLL_PROCESS_DETACH:
            DBG_PRINT((DBG_PREFIX "DETACH"));
            break;

        default:
            break;

    }

    FUNC_LEAVE_BOOL(TRUE);
}

/*! \brief RegisterModule processing

 This function is called when the DOS portion calls the
 REGISTERMODULE function.
*/

VOID
VDDRegisterInit(VOID)
{
    FUNC_ENTER();

    DBG_PRINT((DBG_PREFIX "RegisterInit"));
    setCF(0);

    FUNC_LEAVE();
}

/*! \brief RegisterModule processing

 This function is called when the DOS portion calls the
 DISPATCHCALL function.
*/

VOID
VDDDispatch(VOID)
{
    FUNCTIONCODE functioncode;
    CBM_FILE cbmfile;
    BOOLEAN error;

#if 0
    int port;
    int size;
    UCHAR priaddr_or_line;
    UCHAR secaddr_or_state;
    WORD buffer;
    WORD upload_address;

#endif

    FUNC_ENTER();

    DBG_PRINT((DBG_PREFIX "Dispatch"));
    
    FUNC_PARAM((DBG_PREFIX
        "CALL:\n"
        "EAX = %08x, EBX = %08x, ECX = %08x, EDX = %08x,\n"
        "ESI = %08x, EDI = %08x, ESP = %08x, EBP = %08x,\n"
        "CS = %04x, EIP = %08x,\n"
        "DS = %04x,  ES = %04x,  SS = %04x.\n",
        getEAX(), getEBX(), getECX(), getEDX(),
        getESI(), getEDI(), getESP(), getEBP(),
        getCS(), getEIP(),
        getDS(), getES(), getSS()
        ));

    functioncode = getDL();

#if 0
    /* Process the handle to the file for everything but
     * FC_DRIVER_OPEN and FC_GET_DRIVER_NAME
     */

    switch (functioncode)
    {
    case FC_DRIVER_OPEN:
    case FC_GET_DRIVER_NAME:
        break;

    default:
        cbmfile = (CBM_FILE) getEBX();
        break;
    }

    /* get a port or a size from CX (or DI in the case of FC_CBM_OPEN)
     * for commands which have this parameter
     */
    switch (functioncode)
    {
    case FC_DRIVER_OPEN:
        port = getCX();
        break;

    case FC_GET_DRIVER_NAME:
        port = getCX();
        size = getDI();
        break;

    case FC_RAW_READ:
    case FC_RAW_WRITE:
    case FC_UPLOAD:
    case FC_DEVICE_STATUS:
    case FC_EXEC_COMMAND:
    case FC_IDENTIFY:
        size = getCX();
        break;

    case FC_OPEN:
        size = getDI();
        break;

    default:
        break;
    }

    /* process primary and secondary address for commands 
     * which have these (primary in DH, secondary in CL) */

    switch (functioncode)
    {
    case FC_IEC_RELEASE:

    case FC_LISTEN:
    case FC_TALK:
    case FC_OPEN:
        /* these commands have primary and secondary address.
         * at first, process the secondary address: 
         */
        secaddr_or_state = getCL();

        /* FALL-THROUGH */

    case FC_IEC_GET:
    case FC_IEC_SET:

    case FC_PP_WRITE:

    case FC_UPLOAD:
    case FC_DEVICE_STATUS:
    case FC_EXEC_COMMAND:
    case FC_IDENTIFY:
        priaddr_or_line = getDH();
    }

    switch (functioncode)
    {
    case FC_UPLOAD:
        upload_address = getDI();
        /* FALL THROUGH */

    case FC_OPEN:
    case FC_RAW_READ:
    case FC_RAW_WRITE:
    case FC_DEVICE_STATUS:
    case FC_EXEC_COMMAND:
    case FC_IDENTIFY:
        buffer = getSI();
        break;

    default:
        buffer = 0;
        break;
    }

#else

    error = FALSE;

    // convert to BX value into a CBM_FILE
    switch (functioncode)
    {
    case FC_DRIVER_OPEN:
        // FC_DRIVER_OPEN is special,
	// it does not have a BX input.
        break;

    default:
        cbmfile = vdd_cbmfile_get(getBX());

        if (cbmfile == INVALID_HANDLE_VALUE)
        {
            DBG_ERROR((DBG_PREFIX "invalid BX given: %04x", getBX()));
            error = TRUE;
        }
        break;
    }

    if (!error)
    {
        switch (functioncode)
        {
        case FC_DRIVER_OPEN:     error = vdd_driver_open();          break;
        case FC_DRIVER_CLOSE:    error = vdd_driver_close(cbmfile);  break;
        case FC_LISTEN:          error = vdd_listen(cbmfile);        break;
        case FC_TALK:            error = vdd_talk(cbmfile);          break;
        case FC_OPEN:            error = vdd_open(cbmfile);          break;
        case FC_CLOSE:           error = vdd_close(cbmfile);         break;
        case FC_RAW_READ:        error = vdd_raw_read(cbmfile);      break;
        case FC_RAW_WRITE:       error = vdd_raw_write(cbmfile);     break;
        case FC_UNLISTEN:        error = vdd_unlisten(cbmfile);      break;
        case FC_UNTALK:          error = vdd_untalk(cbmfile);        break;
        case FC_GET_EOI:         error = vdd_get_eoi(cbmfile);       break;
        case FC_CLEAR_EOI:       error = vdd_clear_eoi(cbmfile);     break;
        case FC_RESET:           error = vdd_reset(cbmfile);         break;
        case FC_PP_READ:         error = vdd_pp_read(cbmfile);       break;
        case FC_PP_WRITE:        error = vdd_pp_write(cbmfile);      break;
        case FC_IEC_POLL:        error = vdd_iec_poll(cbmfile);      break;
        case FC_IEC_GET:         error = vdd_iec_get(cbmfile);       break;
        case FC_IEC_SET:         error = vdd_iec_set(cbmfile);       break;
        case FC_IEC_RELEASE:     error = vdd_iec_release(cbmfile);   break;
        case FC_IEC_WAIT:        error = vdd_iec_wait(cbmfile);      break;
        case FC_UPLOAD:          error = vdd_upload(cbmfile);        break;
        case FC_DEVICE_STATUS:   error = vdd_device_status(cbmfile); break; 
        case FC_EXEC_COMMAND:    error = vdd_exec_command(cbmfile);  break;
        case FC_IDENTIFY:        error = vdd_identify(cbmfile);      break;

        default:
            // this function is not implemented:
            DBG_ERROR((DBG_PREFIX "unknown function code in DL: %02x", functioncode));
            error = TRUE;
            break;
        }
    }

#endif

    setCF(error ? 1 : 0);

    FUNC_PARAM((DBG_PREFIX
        "RET:\n"
        "EAX = %08x, EBX = %08x, ECX = %08x, EDX = %08x,\n"
        "ESI = %08x, EDI = %08x, ESP = %08x, EBP = %08x,\n"
        "CS = %04x, EIP = %08x,\n"
        "DS = %04x,  ES = %04x,  SS = %04x.\n",
        getEAX(), getEBX(), getECX(), getEDX(),
        getESI(), getEDI(), getESP(), getEBP(),
        getCS(), getEIP(),
        getDS(), getES(), getSS()
        ));

    FUNC_LEAVE();
}
