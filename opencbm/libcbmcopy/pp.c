/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *  Copyright 2001 Michael Klein <michael.klein@puffin.lb.shuttle.de>
 */

#ifdef SAVE_RCSID
static char *rcsid =
    "@(#) $Id: pp.c,v 1.2 2004-12-07 19:44:47 strik Exp $";
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
    const char *prog;
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
    cbm_pp_write(fd, c);
    cbm_iec_release(fd, IEC_CLOCK);
#ifndef USE_CBM_IEC_WAIT
    while(cbm_iec_get(fd, IEC_DATA));
#else
    cbm_iec_wait(fd, IEC_DATA, 0);
#endif

    cbm_iec_set(fd, IEC_CLOCK);
#ifndef USE_CBM_IEC_WAIT
    while(!cbm_iec_get(fd, IEC_DATA));
#else
    cbm_iec_wait(fd, IEC_DATA, 1);
#endif

    return 0;
}

static unsigned char read_byte(CBM_FILE fd)
{
    unsigned char c;

    cbm_iec_release(fd, IEC_CLOCK);
#ifndef USE_CBM_IEC_WAIT
    while(cbm_iec_get(fd, IEC_DATA));
#else
    cbm_iec_wait(fd, IEC_DATA, 0);
#endif
    c = cbm_pp_read(fd);
    cbm_iec_set(fd, IEC_CLOCK);
#ifndef USE_CBM_IEC_WAIT
    while(!cbm_iec_get(fd, IEC_DATA));
#else
    cbm_iec_wait(fd, IEC_DATA, 1);
#endif

    return c;
}

static int check_error(CBM_FILE fd, int write)
{
    int error;

    cbm_iec_release(fd, IEC_CLOCK);
    cbm_iec_wait(fd, IEC_DATA, 0);
    error = cbm_iec_get(fd, IEC_CLOCK) == 0;
    if(!error)
    {
        cbm_iec_set(fd, IEC_DATA);
        cbm_iec_wait(fd, IEC_CLOCK, 0); 
        cbm_iec_release(fd, IEC_DATA);
        cbm_iec_set(fd, IEC_CLOCK);
        cbm_iec_wait(fd, IEC_DATA, 1);
    }
    
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
    
    cbm_upload(fd, drive, 0x680, p->prog, p->size);
    return 0;
}

static int start_turbo(CBM_FILE fd, int write)
{
    cbm_iec_wait(fd, IEC_DATA, 1);
    return 0;
}

static void exit_turbo(CBM_FILE fd, int write)
{
    cbm_iec_wait(fd, IEC_DATA, 0);
}

DECLARE_TRANSFER_FUNCS(pp_transfer);
