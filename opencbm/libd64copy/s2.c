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
    "@(#) $Id: s2.c,v 1.11 2006-05-23 12:24:31 wmsr Exp $";
#endif

#include "opencbm.h"
#include "d64copy_int.h"

#include <stdlib.h>

#include "arch.h"

static const unsigned char s2_drive_prog[] = {
#include "s2.inc"
};

static CBM_FILE fd_cbm;
static int two_sided;

static int s2_read_byte(CBM_FILE fd, unsigned char *c)
{
    int i;
    *c = 0;
    for(i=4; i>0; i--) {
                                                                        SETSTATEDEBUG(debugLibD64BitCount=i*2);
#ifndef USE_CBM_IEC_WAIT
        while(cbm_iec_get(fd, IEC_CLOCK));
                                                                        SETSTATEDEBUG((void)0);
        *c = (*c>>1) | (cbm_iec_get(fd, IEC_DATA) ? 0x80 : 0);
#else
        *c = (*c>>1) | ((cbm_iec_wait(fd, IEC_CLOCK, 0) & IEC_DATA) ? 0x80 : 0);
#endif
                                                                        SETSTATEDEBUG((void)0);
        cbm_iec_release(fd, IEC_ATN);
                                                                        SETSTATEDEBUG(debugLibD64BitCount--);
#ifndef USE_CBM_IEC_WAIT
        while(!cbm_iec_get(fd,IEC_CLOCK));
                                                                        SETSTATEDEBUG((void)0);
        *c = (*c>>1) | (cbm_iec_get(fd, IEC_DATA) ? 0x80 : 0);
#else
        *c = (*c>>1) | ((cbm_iec_wait(fd, IEC_CLOCK, 1) & IEC_DATA) ? 0x80 : 0);
#endif
                                                                        SETSTATEDEBUG((void)0);
        cbm_iec_set(fd, IEC_ATN);
    }
                                                                        SETSTATEDEBUG(debugLibD64BitCount=-1);
    return 0;
}

static int s2_write_byte_nohs(CBM_FILE fd, unsigned char c)
{
    int i;
    for(i=4; ; i--) {
                                                                        SETSTATEDEBUG(debugLibD64BitCount=i*2);
        c & 1 ? cbm_iec_set(fd, IEC_DATA) : cbm_iec_release(fd, IEC_DATA);
        c >>= 1;
                                                                        SETSTATEDEBUG((void)0);
        cbm_iec_release(fd, IEC_ATN);
#ifndef USE_CBM_IEC_WAIT
        while(cbm_iec_get(fd, IEC_CLOCK));
#else
        cbm_iec_wait(fd, IEC_CLOCK, 0);
#endif
                                                                        SETSTATEDEBUG(debugLibD64BitCount--);
        c & 1 ? cbm_iec_set(fd, IEC_DATA) : cbm_iec_release(fd, IEC_DATA);
        c >>= 1;
                                                                        SETSTATEDEBUG((void)0);
        cbm_iec_set(fd, IEC_ATN);

        if(i<=1) return 0;

                                                                        SETSTATEDEBUG((void)0);
#ifndef USE_CBM_IEC_WAIT
        while(!cbm_iec_get(fd, IEC_CLOCK));
#else
        cbm_iec_wait(fd, IEC_CLOCK, 1);
#endif
    }
}

static int s2_write_byte(CBM_FILE fd, unsigned char c)
{
                                                                        SETSTATEDEBUG((void)0);
    s2_write_byte_nohs(fd, c);
                                                                        SETSTATEDEBUG((void)0);
#ifndef USE_CBM_IEC_WAIT
    while(!cbm_iec_get(fd, IEC_CLOCK));
#else
    cbm_iec_wait(fd, IEC_CLOCK, 1);
#endif
                                                                        SETSTATEDEBUG(debugLibD64BitCount=-1);
    cbm_iec_release(fd, IEC_DATA);
                                                                        SETSTATEDEBUG((void)0);
    return 0;
}

static int read_block(unsigned char tr, unsigned char se, unsigned char *block)
{
    int  i;
    unsigned char status;

                                                                        SETSTATEDEBUG((void)0);
    s2_write_byte(fd_cbm, tr);
                                                                        SETSTATEDEBUG((void)0);
    s2_write_byte(fd_cbm, se);
#ifndef USE_CBM_IEC_WAIT
    arch_usleep(20000);
#endif
                                                                        SETSTATEDEBUG((void)0);
    s2_read_byte(fd_cbm, &status);
                                                                        SETSTATEDEBUG(debugLibD64ByteCount=0);
    for(i=0;i<BLOCKSIZE;i++)
    {
                                                                        SETSTATEDEBUG(debugLibD64ByteCount++);
        s2_read_byte(fd_cbm, &block[i]);
    }

                                                                        SETSTATEDEBUG(debugLibD64ByteCount=-1);
    return status;
}

static int write_block(unsigned char tr, unsigned char se, const unsigned char *blk, int size, int read_status)
{
    int  i;
    unsigned char status;

                                                                        SETSTATEDEBUG((void)0);
    s2_write_byte(fd_cbm, tr);
                                                                        SETSTATEDEBUG((void)0);
    s2_write_byte(fd_cbm, se);
                                                                        SETSTATEDEBUG(debugLibD64ByteCount=0);
    for(i=0;i<size;i++)
    {
                                                                        SETSTATEDEBUG(debugLibD64ByteCount++);
        s2_write_byte(fd_cbm, blk[i]);
    }

                                                                        SETSTATEDEBUG(debugLibD64ByteCount=-1);
#ifndef USE_CBM_IEC_WAIT
    if(size == BLOCKSIZE) {
        arch_usleep(20000);
    }
#endif
                                                                        SETSTATEDEBUG((void)0);
    s2_read_byte(fd_cbm, &status);

                                                                        SETSTATEDEBUG((void)0);
    return status;
}

static int open_disk(CBM_FILE fd, d64copy_settings *settings,
                     const void *arg, int for_writing,
                     turbo_start start, d64copy_message_cb message_cb)
{
    unsigned char d = (unsigned char)(ULONG_PTR)arg;

    fd_cbm = fd;
    two_sided = settings->two_sided;

                                                                        SETSTATEDEBUG((void)0);
    cbm_upload(fd_cbm, d, 0x700, s2_drive_prog, sizeof(s2_drive_prog));
                                                                        SETSTATEDEBUG((void)0);
    start(fd, d);
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_release(fd_cbm, IEC_CLOCK);
                                                                        SETSTATEDEBUG((void)0);
    while(!cbm_iec_get(fd_cbm, IEC_CLOCK));
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_set(fd_cbm, IEC_ATN);
    arch_usleep(20000);
    
                                                                        SETSTATEDEBUG((void)0);
    return 0;
}

static void close_disk(void)
{
                                                                        SETSTATEDEBUG((void)0);
    s2_write_byte(fd_cbm, 0);
                                                                        SETSTATEDEBUG((void)0);
    s2_write_byte_nohs(fd_cbm, 0);
    arch_usleep(100);
                                                                        SETSTATEDEBUG(debugLibD64BitCount=-1);
    cbm_iec_release(fd_cbm, IEC_DATA);
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_release(fd_cbm, IEC_ATN);
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_set(fd_cbm, IEC_CLOCK);
                                                                        SETSTATEDEBUG((void)0);
}

static int send_track_map(unsigned char tr, const char *trackmap, unsigned char count)
{
    int i;
                                                                        SETSTATEDEBUG((void)0);
    s2_write_byte(fd_cbm, tr);
                                                                        SETSTATEDEBUG((void)0);
    s2_write_byte(fd_cbm, count);
    for(i = 0; i < d64copy_sector_count(two_sided, tr); i++)
    {
                                                                        SETSTATEDEBUG((void)0);
        s2_write_byte(fd_cbm, (unsigned char) !NEED_SECTOR(trackmap[i]));
    }
                                                                        SETSTATEDEBUG((void)0);
    return 0;
}

static int read_gcr_block(unsigned char *se, unsigned char *gcrbuf)
{
    int i;
    unsigned char s;

                                                                        SETSTATEDEBUG((void)0);
    s2_read_byte(fd_cbm, &s);
    *se = s;
                                                                        SETSTATEDEBUG((void)0);
    s2_read_byte(fd_cbm,  &s);

    if(s) {
        return s;
    }
                                                                        SETSTATEDEBUG(debugLibD64ByteCount=0);
    for(i = 0; i < GCRBUFSIZE; i++) {
                                                                        SETSTATEDEBUG(debugLibD64ByteCount++);
        s2_read_byte(fd_cbm, &gcrbuf[i]);
    }

                                                                        SETSTATEDEBUG(debugLibD64ByteCount=-1);
    return 0;
}

DECLARE_TRANSFER_FUNCS_EX(s2_transfer, 1, 1);
