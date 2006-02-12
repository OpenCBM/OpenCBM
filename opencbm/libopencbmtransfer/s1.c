/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2005 Michael Klein <michael.klein@puffin.lb.shuttle.de>
*/

#ifdef SAVE_RCSID
static char *rcsid =
    "@(#) $Id: s1.c,v 1.1.2.1 2006-02-12 16:59:20 strik Exp $";
#endif

#include "opencbm.h"
#include "libopencbmtransfer_int.h"

#include <stdlib.h>

#include "arch.h"

static const unsigned char s1_drive_prog[] = {
#include "s1.inc"
};

static CBM_FILE fd_cbm;
static int two_sided;

static int s1_write_byte_nohs(CBM_FILE fd, unsigned char c)
{
    int b, i;
    for(i=7; ; i--) {
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

        if(i<=0) return 0;

#ifndef USE_CBM_IEC_WAIT
        while(!cbm_iec_get(fd, IEC_DATA));
#else
        cbm_iec_wait(fd, IEC_DATA, 1);
#endif
    }
}

static int s1_write_byte(CBM_FILE fd, unsigned char c)
{
    s1_write_byte_nohs(fd, c);
#ifndef USE_CBM_IEC_WAIT
    while(!cbm_iec_get(fd, IEC_DATA));
#else
    cbm_iec_wait(fd, IEC_DATA, 1);
#endif
    return 0;
}

static int s1_read_byte(CBM_FILE fd, unsigned char *c)
{
    int b=0, i;
    *c = 0;
    for(i=7; i>=0; i--) {
        cbm_iec_release(fd, IEC_CLOCK);
#ifndef USE_CBM_IEC_WAIT
        while(cbm_iec_get(fd, IEC_DATA));
#else 
        cbm_iec_wait(fd, IEC_DATA, 0);
#endif
        b = cbm_iec_get(fd, IEC_CLOCK);
        *c = (*c >> 1) | (b ? 0x80 : 0);
        cbm_iec_set(fd, IEC_DATA);
#ifndef USE_CBM_IEC_WAIT
        while(b == cbm_iec_get(fd, IEC_CLOCK));
#else
        cbm_iec_wait(fd, IEC_CLOCK, !b);
#endif
        cbm_iec_set(fd, IEC_CLOCK);
        cbm_iec_release(fd, IEC_DATA);
#ifndef USE_CBM_IEC_WAIT
        while(!cbm_iec_get(fd, IEC_DATA));
#else
        cbm_iec_wait(fd, IEC_DATA, 1);
#endif
    }
    return 0;
}

static int
upload(CBM_FILE fd, unsigned char drive)
{
    unsigned int bytesWritten;

    bytesWritten = cbm_upload(fd, drive, 0x700, s1_drive_prog, sizeof(s1_drive_prog));

    if (bytesWritten != sizeof(s1_drive_prog))
    {
        DBG_ERROR((DBG_PREFIX "wanted to write %u bytes, but only %u "
            "bytes could be written", sizeof(s1_drive_prog), bytesWritten));

        return 1;
    }

    return 0;
}

static int
init(CBM_FILE fd, unsigned char drive)
{
    cbm_iec_wait(fd, IEC_DATA, 1);

    return 0;
}

static int
read1byte(CBM_FILE fd, unsigned char *c1)
{
    return s1_read_byte(fd, c1);
}

static int
read2byte(CBM_FILE fd, unsigned char *c1, unsigned char *c2)
{
    int ret = 0;

    ret = s1_read_byte(fd, c1);

    if (ret == 0)
        ret = s1_read_byte(fd, c2);

    return 1;
}

static int
readblock(CBM_FILE fd, unsigned char *p, unsigned int length)
{
    for (; length < 0x100; length++)
    {
        if (s1_read_byte(fd, p++))
            return 1;
    }

    return 0;
}

static int
write1byte(CBM_FILE fd, unsigned char c1)
{
    return s1_write_byte(fd, c1);
}

static int
write2byte(CBM_FILE fd, unsigned char c1, unsigned char c2)
{
    int ret = 0;

    ret = s1_write_byte(fd, c1);

    if (ret == 0)
        ret = s1_write_byte(fd, c2);

    return 1;
}

static int
writeblock(CBM_FILE fd, unsigned char *p, unsigned int length)
{
    for (; length < 0x100; length++)
    {
        if (s1_write_byte(fd, *p++))
            return 1;
    }

    return 0;
}

DECLARE_TRANSFER_FUNCS(s1);
