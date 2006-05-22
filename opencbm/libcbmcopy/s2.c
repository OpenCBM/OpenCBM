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
    "@(#) $Id: s2.c,v 1.10 2006-05-22 08:34:19 wmsr Exp $";
#endif

#include "opencbm.h"
#include "cbmcopy_int.h"

#include <stdlib.h>

#include "arch.h"


static const unsigned char s2r15x1[] = {
#include "s2r.inc"
};

static const unsigned char s2w15x1[] = {
#include "s2w.inc"
};

static const unsigned char s2r1581[] = {
#include "s2r-1581.inc"
};

static const unsigned char s2w1581[] = {
#include "s2w-1581.inc"
};

static struct drive_prog
{
    const unsigned char *prog;
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
                                                                        SETSTATEDEBUG(debugCBMcopyBitCount=i*2);
        c & 1 ? cbm_iec_set(fd, IEC_DATA) : cbm_iec_release(fd, IEC_DATA);
        c >>= 1;
                                                                        SETSTATEDEBUG((void)0);
        cbm_iec_release(fd, IEC_ATN);
                                                                        SETSTATEDEBUG((void)0);
#ifndef USE_CBM_IEC_WAIT
        while(cbm_iec_get(fd, IEC_CLOCK));
#else
        cbm_iec_wait(fd, IEC_CLOCK, 0);
#endif
                                                                        SETSTATEDEBUG(debugCBMcopyBitCount--);
        c & 1 ? cbm_iec_set(fd, IEC_DATA) : cbm_iec_release(fd, IEC_DATA);
        c >>= 1;
                                                                        SETSTATEDEBUG((void)0);
        cbm_iec_set(fd, IEC_ATN);
                                                                        SETSTATEDEBUG((void)0);
#ifndef USE_CBM_IEC_WAIT
        while(!cbm_iec_get(fd, IEC_CLOCK));
#else
        cbm_iec_wait(fd, IEC_CLOCK, 1);
#endif
    }
                                                                        SETSTATEDEBUG(debugCBMcopyBitCount=-1);
    cbm_iec_release(fd, IEC_DATA);
                                                                        SETSTATEDEBUG((void)0);
    return 0;
}

static unsigned char read_byte(CBM_FILE fd)
{
    int i;
    unsigned char c;
    c = 0;
    for(i=4; i>0; i--) {
                                                                        SETSTATEDEBUG(debugCBMcopyBitCount=i*2);
#ifndef USE_CBM_IEC_WAIT
        while(cbm_iec_get(fd, IEC_CLOCK));
        c = (c>>1) | (cbm_iec_get(fd, IEC_DATA) ? 0x80 : 0);
#else
        c = (c>>1) | ((cbm_iec_wait(fd, IEC_CLOCK, 0) & IEC_DATA) ? 0x80 : 0 );
#endif
                                                                        SETSTATEDEBUG((void)0);
        cbm_iec_release(fd, IEC_ATN);
                                                                        SETSTATEDEBUG(debugCBMcopyBitCount--);
#ifndef USE_CBM_IEC_WAIT
        while(!cbm_iec_get(fd,IEC_CLOCK));
        c = (c>>1) | (cbm_iec_get(fd, IEC_DATA) ? 0x80 : 0);
#else   
        c = (c>>1) | ((cbm_iec_wait(fd, IEC_CLOCK, 1) & IEC_DATA) ? 0x80 : 0 );    
#endif  
                                                                        SETSTATEDEBUG((void)0);
        cbm_iec_set(fd, IEC_ATN);
    }   

                                                                        SETSTATEDEBUG(debugCBMcopyBitCount=-1);
    return c;
}

static int check_error(CBM_FILE fd, int write)
{
    int error;

                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_release(fd, IEC_ATN);
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_wait(fd, IEC_CLOCK, 0);
                                                                        SETSTATEDEBUG((void)0);
    error = cbm_iec_get(fd, IEC_DATA) == 0;
    if(!error)
    {
                                                                        SETSTATEDEBUG((void)0);
        cbm_iec_set(fd, IEC_DATA);
                                                                        SETSTATEDEBUG((void)0);
        cbm_iec_set(fd, IEC_ATN);
                                                                        SETSTATEDEBUG((void)0);
        cbm_iec_wait(fd, IEC_CLOCK, 1);
        if(!write)
        {
                                                                        SETSTATEDEBUG((void)0);
            cbm_iec_release(fd, IEC_DATA);
        }
    }

                                                                        SETSTATEDEBUG((void)0);
    return error;
}

static int upload_turbo(CBM_FILE fd, unsigned char drive,
                        enum cbm_device_type_e drive_type, int write)
{
    const struct drive_prog *p;
    int dt;

    dt = (drive_type == cbm_dt_cbm1581);
    p = &drive_progs[dt * 2 + (write != 0)];

                                                                        SETSTATEDEBUG((void)0);
    cbm_upload(fd, drive, 0x680, p->prog, p->size);
                                                                        SETSTATEDEBUG((void)0);
    return 0;
}

static int start_turbo(CBM_FILE fd, int write)
{
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_release(fd, IEC_CLOCK);
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_wait(fd, IEC_CLOCK, 1);
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_set(fd, IEC_ATN);
                                                                        SETSTATEDEBUG((void)0);
    arch_usleep(20000);
                                                                        SETSTATEDEBUG((void)0);
    return 0;
}

static void exit_turbo(CBM_FILE fd, int write)
{
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_release(fd, IEC_ATN);
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_release(fd, IEC_DATA);
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_set(fd, IEC_CLOCK);
                                                                        SETSTATEDEBUG((void)0);
    arch_usleep(20000);
                                                                        SETSTATEDEBUG((void)0);
//    cbm_iec_wait(fd, IEC_DATA, 0);
                                                                        SETSTATEDEBUG((void)0);
}

DECLARE_TRANSFER_FUNCS(s2_transfer);
