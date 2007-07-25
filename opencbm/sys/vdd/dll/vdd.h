/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2004 Spiro Trikaliotis
*/

/*! **************************************************************
** \file sys/vdd/dll/vdd.h \n
** \author Spiro Trikaliotis \n
** \version $Id: vdd.h,v 1.7 2007-07-25 16:37:52 strik Exp $ \n
** \n
** \brief Function prototypes for the VDD
**
****************************************************************/

#ifndef VDD_H
#define VDD_H

#include <opencbm.h>

typedef
enum FUNCTIONCODE
{
    FC_DRIVER_OPEN,
    FC_DRIVER_CLOSE,
    FC_LISTEN,
    FC_TALK,
    FC_OPEN,
    FC_CLOSE,
    FC_RAW_READ,
    FC_RAW_WRITE,
    FC_UNLISTEN,
    FC_UNTALK,
    FC_GET_EOI,
    FC_CLEAR_EOI,
    FC_RESET,
    FC_PP_READ,
    FC_PP_WRITE,
    FC_IEC_POLL,
    FC_IEC_GET,
    FC_IEC_SET,
    FC_IEC_RELEASE,
    FC_IEC_WAIT,
    FC_UPLOAD,
    FC_DEVICE_STATUS,
    FC_EXEC_COMMAND,
    FC_IDENTIFY,
    FC_GET_DRIVER_NAME,
    FC_VDD_INSTALL_IOHOOK,
    FC_VDD_UNINSTALL_IOHOOK,
    FC_VDD_USLEEP,
    FC_IEC_SETRELEASE,
    FC_IDENTIFY_XP1541
} FUNCTIONCODE;

extern HANDLE vdd_handle;

extern BOOLEAN vdd_driver_open(VOID);
extern BOOLEAN vdd_driver_close(CBM_FILE);
extern BOOLEAN vdd_raw_write(CBM_FILE);
extern BOOLEAN vdd_raw_read(CBM_FILE);
extern BOOLEAN vdd_listen(CBM_FILE);
extern BOOLEAN vdd_talk(CBM_FILE);
extern BOOLEAN vdd_open(CBM_FILE);
extern BOOLEAN vdd_close(CBM_FILE);
extern BOOLEAN vdd_unlisten(CBM_FILE);
extern BOOLEAN vdd_untalk(CBM_FILE);
extern BOOLEAN vdd_get_eoi(CBM_FILE);
extern BOOLEAN vdd_clear_eoi(CBM_FILE);
extern BOOLEAN vdd_reset(CBM_FILE);
extern BOOLEAN vdd_pp_read(CBM_FILE);
extern BOOLEAN vdd_pp_write(CBM_FILE);
extern BOOLEAN vdd_iec_poll(CBM_FILE);
extern BOOLEAN vdd_iec_set(CBM_FILE);
extern BOOLEAN vdd_iec_release(CBM_FILE);
extern BOOLEAN vdd_iec_setrelease(CBM_FILE);
extern BOOLEAN vdd_iec_wait(CBM_FILE);
extern BOOLEAN vdd_iec_get(CBM_FILE);
extern BOOLEAN vdd_upload(CBM_FILE);
extern BOOLEAN vdd_device_status(CBM_FILE);
extern BOOLEAN vdd_exec_command(CBM_FILE);
extern BOOLEAN vdd_identify(CBM_FILE);
extern BOOLEAN vdd_identify_xp1541(CBM_FILE);
extern BOOLEAN vdd_get_driver_name(VOID);

extern BOOLEAN vdd_install_iohook(CBM_FILE);
extern BOOLEAN vdd_uninstall_iohook(CBM_FILE);
extern USHORT  vdd_uninstall_iohook_internal(VOID);

extern BOOLEAN vdd_usleep(VOID);

extern CBM_FILE vdd_cbmfile_get(WORD);
extern WORD     vdd_cbmfile_store(CBM_FILE);
extern CBM_FILE vdd_cbmfile_delete(WORD);
extern VOID     vdd_cbmfile_closeall(VOID);

#endif /* #ifndef VDD_H */
