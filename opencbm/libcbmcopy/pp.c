/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *  Copyright 2001 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 */

#ifdef SAVE_RCSID
static char *rcsid =
    "@(#) $Id: pp.c,v 1.9 2006-05-22 08:34:19 wmsr Exp $";
#endif

#include "opencbm.h"
#include "cbmcopy_int.h"

#include <stdlib.h>

#include "arch.h"

static const unsigned char ppr1541[] = {
#include "ppr-1541.inc"
};

static const unsigned char ppr1571[] = {
#include "ppr-1571.inc"
};

static const unsigned char ppw1541[] = {
#include "ppw-1541.inc"
};

static const unsigned char ppw1571[] = {
#include "ppw-1571.inc"
};


static struct drive_prog
{
    const unsigned char *prog;
    size_t size;
} drive_progs[] =
{
    { ppr1541, sizeof(ppr1541) },
    { ppw1541, sizeof(ppw1541) },
    { ppr1571, sizeof(ppr1571) },
    { ppw1571, sizeof(ppw1571) }
};


static int write_byte(CBM_FILE fd, unsigned char c)
{
                                                                        SETSTATEDEBUG((void)0);
    cbm_pp_write(fd, c);
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_release(fd, IEC_CLOCK);
                                                                        SETSTATEDEBUG((void)0);
#ifndef USE_CBM_IEC_WAIT
    while(cbm_iec_get(fd, IEC_DATA));
#else
    cbm_iec_wait(fd, IEC_DATA, 0);
#endif

                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_set(fd, IEC_CLOCK);
                                                                        SETSTATEDEBUG((void)0);
#ifndef USE_CBM_IEC_WAIT
    while(!cbm_iec_get(fd, IEC_DATA));
#else
    cbm_iec_wait(fd, IEC_DATA, 1);
#endif

                                                                        SETSTATEDEBUG((void)0);
    return 0;
}

static unsigned char read_byte(CBM_FILE fd)
{
    unsigned char c;

                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_release(fd, IEC_CLOCK);
                                                                        SETSTATEDEBUG((void)0);
#ifndef USE_CBM_IEC_WAIT
    while(cbm_iec_get(fd, IEC_DATA));
#else
    cbm_iec_wait(fd, IEC_DATA, 0);
#endif
                                                                        SETSTATEDEBUG((void)0);
    c = cbm_pp_read(fd);
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_set(fd, IEC_CLOCK);
                                                                        SETSTATEDEBUG((void)0);
#ifndef USE_CBM_IEC_WAIT
    while(!cbm_iec_get(fd, IEC_DATA));
#else
    cbm_iec_wait(fd, IEC_DATA, 1);
#endif

                                                                        SETSTATEDEBUG((void)0);
    return c;
}

static int check_error(CBM_FILE fd, int write)
{
    int error;

                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_release(fd, IEC_CLOCK);
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_wait(fd, IEC_DATA, 0);
                                                                        SETSTATEDEBUG((void)0);
    error = cbm_iec_get(fd, IEC_CLOCK) == 0;
    if(!error)
    {
                                                                        SETSTATEDEBUG((void)0);
        cbm_iec_set(fd, IEC_DATA);
                                                                        SETSTATEDEBUG((void)0);
        cbm_iec_wait(fd, IEC_CLOCK, 0); 
                                                                        SETSTATEDEBUG((void)0);
        cbm_iec_release(fd, IEC_DATA);
                                                                        SETSTATEDEBUG((void)0);
        cbm_iec_set(fd, IEC_CLOCK);
                                                                        SETSTATEDEBUG((void)0);
        cbm_iec_wait(fd, IEC_DATA, 1);
    }
    
                                                                        SETSTATEDEBUG((void)0);
    return error;
}

static int upload_turbo(CBM_FILE fd, unsigned char drive,
                        enum cbm_device_type_e drive_type, int write)
{
    const struct drive_prog *p;
    int dt;

    switch(drive_type)
    {
        case cbm_dt_cbm1541:
            dt = 0;
            break;

        case cbm_dt_cbm1570:
        case cbm_dt_cbm1571:
            dt = 1;
            break;

        default:
            return -1;
    }

    p = &drive_progs[dt * 2 + (write != 0)];
    
                                                                        SETSTATEDEBUG((void)0);
    cbm_upload(fd, drive, 0x680, p->prog, p->size);
                                                                        SETSTATEDEBUG((void)0);
    return 0;
}

static int start_turbo(CBM_FILE fd, int write)
{
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_wait(fd, IEC_DATA, 1);
                                                                        SETSTATEDEBUG((void)0);
    return 0;
}

static void exit_turbo(CBM_FILE fd, int write)
{
                                                                        SETSTATEDEBUG((void)0);
//    cbm_iec_wait(fd, IEC_DATA, 0);
                                                                        SETSTATEDEBUG((void)0);
}

DECLARE_TRANSFER_FUNCS(pp_transfer);
