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
    "@(#) $Id: pp.c,v 1.8 2006-01-29 12:02:46 strik Exp $";
#endif

#include "opencbm.h"
#include "d64copy_int.h"

#include <stdlib.h>

#include "arch.h"

enum pp_direction_e
{
    PP_READ, PP_WRITE
};

static CBM_FILE fd_cbm;
static int two_sided;

static const unsigned char pp1541_drive_prog[] = {
#include "pp1541.inc"
};

static const unsigned char pp1571_drive_prog[] = {
#include "pp1571.inc"
};

static void pp_check_direction(enum pp_direction_e dir)
{
    static enum pp_direction_e direction = PP_READ;
    if(direction != dir)
    {
        arch_usleep(100);
        direction = dir;
    }
}

static int pp_write_nohs(CBM_FILE fd, char c1, char c2)
{
    pp_check_direction(PP_WRITE);
    cbm_pp_write(fd, c1);
    cbm_iec_release(fd, IEC_CLOCK);
#ifndef USE_CBM_IEC_WAIT
    while(cbm_iec_get(fd, IEC_DATA));
#else
    cbm_iec_wait(fd, IEC_DATA, 0);
#endif

    cbm_pp_write(fd, c2);
    cbm_iec_set(fd, IEC_CLOCK);

    return 0;
}

static int pp_write(CBM_FILE fd, char c1, char c2)
{
    pp_write_nohs(fd, c1, c2);
#ifndef USE_CBM_IEC_WAIT
    while(!cbm_iec_get(fd, IEC_DATA));
#else
    cbm_iec_wait(fd, IEC_DATA, 1);
#endif
    return 0;
}

static int pp_read(CBM_FILE fd, unsigned char *c1, unsigned char *c2)
{
    pp_check_direction(PP_READ);
    cbm_iec_release(fd, IEC_CLOCK);
#ifndef USE_CBM_IEC_WAIT
    while(cbm_iec_get(fd, IEC_DATA));
#else
    cbm_iec_wait(fd, IEC_DATA, 0);
#endif
    *c1 = cbm_pp_read(fd);

    cbm_iec_set(fd, IEC_CLOCK);
#ifndef USE_CBM_IEC_WAIT
    while(!cbm_iec_get(fd, IEC_DATA));
#else
    cbm_iec_wait(fd, IEC_DATA, 1);
#endif
    *c2 = cbm_pp_read(fd);

    return 0;
}

static int read_block(unsigned char tr, unsigned char se, char *block)
{
    int  i;
    unsigned char status;

    pp_write(fd_cbm, tr, se);
#ifndef USE_CBM_IEC_WAIT    
    arch_usleep(20000);
#endif
    pp_read(fd_cbm, &status, &status);

    for(i=0;i<BLOCKSIZE;i+=2) pp_read(fd_cbm, &block[i], &block[i+1]);

    return status;
}

static int write_block(unsigned char tr, unsigned char se, const char *blk, int size, int read_status)
{
    int i = 0;
    unsigned char status;

    pp_write(fd_cbm, tr, se);

    if(size % 2) {
        pp_write(fd_cbm, *blk, *blk);
        i = 1;
    }

    for(;i<size;i+=2) pp_write(fd_cbm, blk[i], blk[i+1]);

#ifndef USE_CBM_IEC_WAIT    
    if(size == BLOCKSIZE) {
        arch_usleep(20000);
    }
#endif

    pp_read(fd_cbm, &status, &status);

    return status;
}

static int open_disk(CBM_FILE fd, d64copy_settings *settings,
                     const void *arg, int for_writing,
                     turbo_start start, d64copy_message_cb message_cb)
{
    unsigned char d = (unsigned char)(ULONG_PTR)arg;
    unsigned const char *drive_prog;
    int prog_size;

    fd_cbm    = fd;
    two_sided = settings->two_sided;

    if(settings->drive_type != cbm_dt_cbm1541)
    {
        drive_prog = pp1571_drive_prog;
        prog_size  = sizeof(pp1571_drive_prog);
    }
    else
    {
        drive_prog = pp1541_drive_prog;
        prog_size  = sizeof(pp1541_drive_prog);
    }

    cbm_upload(fd_cbm, d, 0x700, drive_prog, prog_size);
    start(fd, d);
    pp_check_direction(PP_WRITE);
    cbm_iec_wait(fd_cbm, IEC_DATA, 1);
    return 0;
}

static void close_disk(void)
{
    pp_write_nohs(fd_cbm, 0, 0);
    arch_usleep(100);
    cbm_iec_wait(fd_cbm, IEC_DATA, 0);
}

static int send_track_map(unsigned char tr, const char *trackmap, unsigned char count)
{
    int i;
    unsigned char c;
    pp_write(fd_cbm, tr, count);
    for(i = 0; i < d64copy_sector_count(two_sided, tr); i++)
    {
        c = !NEED_SECTOR(trackmap[i]);
        pp_write(fd_cbm, c, c);
    }
    return 0;
}

static int read_gcr_block(unsigned char *se, unsigned char *gcrbuf)
{
    int i;
    unsigned char s;

    pp_read(fd_cbm, &s, &s);
    *se = s;
    pp_read(fd_cbm, &s, &s);

    if(s) {
        return s;
    }

    for(i = 0; i < GCRBUFSIZE; i += 2) {
        pp_read(fd_cbm, &gcrbuf[i], &gcrbuf[i+1]);
    }

    return 0;
}

DECLARE_TRANSFER_FUNCS_EX(pp_transfer, 1, 1);
