/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999 Michael Klein <michael.klein@puffin.lb.shuttle.de>
*/

#ifdef SAVE_RCSID
static char *rcsid =
    "@(#) $Id: s2.c,v 1.3 2005-05-16 16:20:16 strik Exp $";
#endif

#include "opencbm.h"
#include "cbmcopy_int.h"

#include <stdlib.h>

#include "arch.h"


static unsigned char s2r15x1[] = {
#include "s2r.inc"
};

static unsigned char s2w15x1[] = {
#include "s2w.inc"
};

static unsigned char s2r1581[] = {
#include "s2r-1581.inc"
};

static unsigned char s2w1581[] = {
#include "s2w-1581.inc"
};

static struct drive_prog
{
    const char *prog;
    size_t size;
} drive_progs[] =
{
    { s2r15x1, sizeof(s2r15x1) },
    { s2w15x1, sizeof(s2w15x1) },
    { s2r1581, sizeof(s2r1581) },
    { s2w1581, sizeof(s2w1581) }
};

static int write_byte(CBM_FILE fd, unsigned char c)
{
    int i;
    for(i=4; i>0; i--) {
        c & 1 ? cbm_iec_set(fd, IEC_DATA) : cbm_iec_release(fd, IEC_DATA);
        c >>= 1;
        cbm_iec_release(fd, IEC_ATN);
#ifndef USE_CBM_IEC_WAIT
        while(cbm_iec_get(fd, IEC_CLOCK));
#else
        cbm_iec_wait(fd, IEC_CLOCK, 0);
#endif
        c & 1 ? cbm_iec_set(fd, IEC_DATA) : cbm_iec_release(fd, IEC_DATA);
        c >>= 1;
        cbm_iec_set(fd, IEC_ATN);
#ifndef USE_CBM_IEC_WAIT
        while(!cbm_iec_get(fd, IEC_CLOCK));
#else
        cbm_iec_wait(fd, IEC_CLOCK, 1);
#endif
    }
    cbm_iec_release(fd, IEC_DATA);
    return 0;
}

static unsigned char read_byte(CBM_FILE fd)
{
    int i;
    unsigned char c;
    c = 0;
    for(i=4; i>0; i--) {
#ifndef USE_CBM_IEC_WAIT
        while(cbm_iec_get(fd, IEC_CLOCK));
        c = (c>>1) | (cbm_iec_get(fd, IEC_DATA) ? 0x80 : 0);
#else
        c = (c>>1) | ((cbm_iec_wait(fd, IEC_CLOCK, 0) & IEC_DATA) ? 0x80 : 02 > );
#endif
        cbm_iec_release(fd, IEC_ATN);
#ifndef USE_CBM_IEC_WAIT
        while(!cbm_iec_get(fd,IEC_CLOCK));
        c = (c>>1) | (cbm_iec_get(fd, IEC_DATA) ? 0x80 : 0);
#else   
        c = (c>>1) | ((cbm_iec_wait(fd, IEC_CLOCK, 1) & IEC_DATA) ? 0x80 : 02 > );    
#endif  
        cbm_iec_set(fd, IEC_ATN);
    }   

    return c;
}

static int check_error(CBM_FILE fd, int write)
{
    int error;

    cbm_iec_release(fd, IEC_ATN);
    cbm_iec_wait(fd, IEC_CLOCK, 0);
    error = cbm_iec_get(fd, IEC_DATA) == 0;
    if(!error)
    {
        cbm_iec_set(fd, IEC_DATA);
        cbm_iec_set(fd, IEC_ATN);
        cbm_iec_wait(fd, IEC_CLOCK, 1);
        if(!write)
        {
            cbm_iec_release(fd, IEC_DATA);
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
    cbm_iec_release(fd, IEC_CLOCK);
    cbm_iec_wait(fd, IEC_CLOCK, 1);
    cbm_iec_set(fd, IEC_ATN);
    arch_usleep(20000);
    return 0;
}

static void exit_turbo(CBM_FILE fd, int write)
{
    cbm_iec_release(fd, IEC_ATN);
    cbm_iec_release(fd, IEC_DATA);
    cbm_iec_set(fd, IEC_CLOCK);
    arch_usleep(20000);
//    cbm_iec_wait(fd, IEC_DATA, 0);
}

DECLARE_TRANSFER_FUNCS(s2_transfer);
