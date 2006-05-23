/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
*/

#ifdef SAVE_RCSID
static char *rcsid =
    "@(#) $Id: pp.c,v 1.17 2006-05-23 12:24:31 wmsr Exp $";
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

static int pp_write(CBM_FILE fd, char c1, char c2)
{
                                                                        SETSTATEDEBUG((void)0);
    pp_check_direction(PP_WRITE);
                                                                        SETSTATEDEBUG((void)0);
#ifndef USE_CBM_IEC_WAIT
    while(!cbm_iec_get(fd, IEC_DATA));
#else
    cbm_iec_wait(fd, IEC_DATA, 1);
#endif
                                                                        SETSTATEDEBUG((void)0);
    cbm_pp_write(fd, c1);
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_release(fd, IEC_CLOCK);

                                                                        SETSTATEDEBUG((void)0);
#ifndef USE_CBM_IEC_WAIT
    while(cbm_iec_get(fd, IEC_DATA));
#else
    cbm_iec_wait(fd, IEC_DATA, 0);
#endif
                                                                        SETSTATEDEBUG((void)0);
    cbm_pp_write(fd, c2);
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_set(fd, IEC_CLOCK);

                                                                        SETSTATEDEBUG((void)0);
    return 0;
}

static int pp_read(CBM_FILE fd, unsigned char *c1, unsigned char *c2)
{
                                                                        SETSTATEDEBUG((void)0);
    pp_check_direction(PP_READ);
                                                                        SETSTATEDEBUG((void)0);
#ifndef USE_CBM_IEC_WAIT
    while(!cbm_iec_get(fd, IEC_DATA));
#else
    cbm_iec_wait(fd, IEC_DATA, 1);
#endif
                                                                        SETSTATEDEBUG((void)0);
    *c1 = cbm_pp_read(fd);
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_release(fd, IEC_CLOCK);

                                                                        SETSTATEDEBUG((void)0);
#ifndef USE_CBM_IEC_WAIT
    while(cbm_iec_get(fd, IEC_DATA));
#else
    cbm_iec_wait(fd, IEC_DATA, 0);
#endif
                                                                        SETSTATEDEBUG((void)0);
    *c2 = cbm_pp_read(fd);
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_set(fd, IEC_CLOCK);

                                                                        SETSTATEDEBUG((void)0);
    return 0;
}

static int read_block(unsigned char tr, unsigned char se, unsigned char *block)
{
    int  i;
    unsigned char status;

                                                                        SETSTATEDEBUG((void)0);
    pp_write(fd_cbm, tr, se);
#ifndef USE_CBM_IEC_WAIT    
    arch_usleep(20000);
#endif
                                                                        SETSTATEDEBUG((void)0);
    pp_read(fd_cbm, &status, &status);

                                                                        SETSTATEDEBUG(debugLibD64ByteCount=0);
    for(i=0;i<BLOCKSIZE;i+=2)
    {
                                                                        SETSTATEDEBUG(debugLibD64ByteCount++);
        pp_read(fd_cbm, &block[i], &block[i+1]);
    }
                                                                        SETSTATEDEBUG(debugLibD64ByteCount=-1);

                                                                        SETSTATEDEBUG((void)0);
    return status;
}

static int write_block(unsigned char tr, unsigned char se, const unsigned char *blk, int size, int read_status)
{
    int i = 0;
    unsigned char status;

                                                                        SETSTATEDEBUG((void)0);
    pp_write(fd_cbm, tr, se);

                                                                        SETSTATEDEBUG((void)0);
    if(size % 2) {
        pp_write(fd_cbm, *blk, *blk);
        i = 1;
    }

                                                                        SETSTATEDEBUG(debugLibD64ByteCount=0);
    for(;i<size;i+=2)
    {
                                                                        SETSTATEDEBUG(debugLibD64ByteCount++);
        pp_write(fd_cbm, blk[i], blk[i+1]);
    }

                                                                        SETSTATEDEBUG(debugLibD64ByteCount=-1);
#ifndef USE_CBM_IEC_WAIT    
    if(size == BLOCKSIZE) {
        arch_usleep(20000);
    }
#endif

                                                                        SETSTATEDEBUG((void)0);
    pp_read(fd_cbm, &status, &status);

                                                                        SETSTATEDEBUG((void)0);
    return status;
}

static int open_disk(CBM_FILE fd, d64copy_settings *settings,
                     const void *arg, int for_writing,
                     turbo_start start, d64copy_message_cb message_cb)
{
    unsigned char d = (unsigned char)(ULONG_PTR)arg;
    const unsigned char *drive_prog;
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

                                                                        SETSTATEDEBUG((void)0);
    /* make sure the XP1541 portion of the cable is in input mode */
    cbm_pp_read(fd_cbm);

                                                                        SETSTATEDEBUG((void)0);
    cbm_upload(fd_cbm, d, 0x700, drive_prog, prog_size);
                                                                        SETSTATEDEBUG((void)0);
    start(fd, d);
                                                                        SETSTATEDEBUG((void)0);
    pp_check_direction(PP_READ);
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_set(fd_cbm, IEC_CLOCK);
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_wait(fd_cbm, IEC_DATA, 1);
                                                                        SETSTATEDEBUG((void)0);
    return 0;
}

static void close_disk(void)
{
                                                                        SETSTATEDEBUG((void)0);
    pp_write(fd_cbm, 0, 0);
    arch_usleep(100);
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_wait(fd_cbm, IEC_DATA, 0);

    /* make sure the XP1541 portion of the cable is in input mode */
                                                                        SETSTATEDEBUG((void)0);
    cbm_pp_read(fd_cbm);
                                                                        SETSTATEDEBUG((void)0);
}

static int send_track_map(unsigned char tr, const char *trackmap, unsigned char count)
{
    int i;
    unsigned char c;
                                                                        SETSTATEDEBUG((void)0);
    pp_write(fd_cbm, tr, count);
                                                                        SETSTATEDEBUG((void)0);
    for(i = 0; i < d64copy_sector_count(two_sided, tr); i++)
    {
        c = !NEED_SECTOR(trackmap[i]);
                                                                        SETSTATEDEBUG((void)0);
        pp_write(fd_cbm, c, c);
    }
                                                                        SETSTATEDEBUG((void)0);
    return 0;
}

static int read_gcr_block(unsigned char *se, unsigned char *gcrbuf)
{
    int i;
    unsigned char s;

                                                                        SETSTATEDEBUG((void)0);
    pp_read(fd_cbm, &s, &s);
    *se = s;
                                                                        SETSTATEDEBUG((void)0);
    pp_read(fd_cbm, &s, &s);

    if(s) {
        return s;
    }

                                                                        SETSTATEDEBUG(debugLibD64ByteCount=0);
    for(i = 0; i < GCRBUFSIZE; i += 2) {
                                                                        SETSTATEDEBUG(debugLibD64ByteCount++);
        pp_read(fd_cbm, &gcrbuf[i], &gcrbuf[i+1]);
    }
                                                                        SETSTATEDEBUG(debugLibD64ByteCount=-1);

                                                                        SETSTATEDEBUG((void)0);
    return 0;
}

DECLARE_TRANSFER_FUNCS_EX(pp_transfer, 1, 1);
