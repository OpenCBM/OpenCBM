/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
*/

#ifdef SAVE_RCSID
static char *rcsid =
    "@(#) $Id: s1.c,v 1.3 2006-02-24 12:21:41 strik Exp $";
#endif

#include "opencbm.h"
#include "cbmcopy_int.h"

#include <stdlib.h>

static unsigned char s1r15x1[] = {
#include "s1r.inc"
};

static unsigned char s1w15x1[] = {
#include "s1w.inc"
};

static unsigned char s1r1581[] = {
#include "s1r-1581.inc"
};

static unsigned char s1w1581[] = {
#include "s1w-1581.inc"
};

static struct drive_prog
{
    const char *prog;
    size_t size;
} drive_progs[] =
{
    { s1r15x1, sizeof(s1r15x1) },
    { s1w15x1, sizeof(s1w15x1) },
    { s1r1581, sizeof(s1r1581) },
    { s1w1581, sizeof(s1w1581) }
};

static int write_byte(CBM_FILE fd, unsigned char c)
{
    int b, i;
    for(i=7; i>=0; i--) {
        b=(c >> i) & 1;
        if(b) cbm_iec_set(fd, IEC_DATA); else cbm_iec_release(fd, IEC_DATA);
        cbm_iec_release(fd, IEC_CLOCK);
#ifndef USE_CBM_IEC_WAIT
        while(!cbm_iec_get(fd, IEC_CLOCK));
#else
        cbm_iec_wait(fd, IEC_CLOCK, 1);
#endif
        if(b) cbm_iec_release(fd, IEC_DATA); else cbm_iec_set(fd, IEC_DATA);
#ifndef USE_CBM_IEC_WAIT
        while(cbm_iec_get(fd, IEC_CLOCK));
#else
        cbm_iec_wait(fd, IEC_CLOCK, 0);
#endif
        cbm_iec_release(fd, IEC_DATA);
        cbm_iec_set(fd, IEC_CLOCK);
#ifndef USE_CBM_IEC_WAIT
        while(!cbm_iec_get(fd, IEC_DATA));
#else
        cbm_iec_wait(fd, IEC_DATA, 1);
#endif
    }
    return 0;
}

static unsigned char read_byte(CBM_FILE fd)
{
    int b=0, i;
    unsigned char c;
    c = 0;
    for(i=7; i>=0; i--) {
#ifndef USE_CBM_IEC_WAIT
        while(cbm_iec_get(fd, IEC_DATA));
#else        
        cbm_iec_wait(fd, IEC_DATA, 0);
#endif
        cbm_iec_release(fd, IEC_CLOCK);
        b = cbm_iec_get(fd, IEC_CLOCK);
        c = (c >> 1) | (b ? 0x80 : 0);
        cbm_iec_set(fd, IEC_DATA);
#ifndef USE_CBM_IEC_WAIT
        while(b == cbm_iec_get(fd, IEC_CLOCK));
#else        
        cbm_iec_wait(fd, IEC_CLOCK, !b);
#endif
        cbm_iec_release(fd, IEC_DATA);
#ifndef USE_CBM_IEC_WAIT
        while(!cbm_iec_get(fd, IEC_DATA));
#else        
        cbm_iec_wait(fd, IEC_DATA, 1);
#endif
        cbm_iec_set(fd, IEC_CLOCK);
    }
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
        if(write)
        {
            cbm_iec_set(fd, IEC_CLOCK);
            cbm_iec_wait(fd, IEC_DATA, 1);
        }
        else
        {
            cbm_iec_wait(fd, IEC_DATA, 1);
            cbm_iec_set(fd, IEC_CLOCK);
        }
    }
    return error;
}

static int upload_turbo(CBM_FILE fd, unsigned char drive,
                        enum cbm_device_type_e drive_type, int write)
{
    const struct drive_prog *p;
    int dt;

    dt = (drive_type == cbm_dt_cbm1581);
    p = &drive_progs[dt * 2 + (write != 0)];

    cbm_upload(fd, drive, 0x680, p->prog, p->size);
    return 0;
}

static int start_turbo(CBM_FILE fd, int write)
{
    if(write)
    {
        cbm_iec_wait(fd, IEC_DATA, 1);
    }
    else
    {
        cbm_iec_release(fd, IEC_CLOCK);
        cbm_iec_wait(fd, IEC_DATA, 1);
        cbm_iec_set(fd, IEC_CLOCK);
    }
    return 0;
}

static void exit_turbo(CBM_FILE fd, int write)
{
//    cbm_iec_wait(fd, IEC_DATA, 0);
}

DECLARE_TRANSFER_FUNCS(s1_transfer);
