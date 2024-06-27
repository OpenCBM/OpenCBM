/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2005 Michael Klein <michael.klein@puffin.lb.shuttle.de>
*/

#include "opencbm.h"
#include "libtrans_int.h"

#include <stdlib.h>

#include "arch.h"

static const unsigned char s2_1541_1571_drive_prog[] = {
#include "s2-1541-1571.inc"
};

static const unsigned char s2_1581_drive_prog[] = {
#include "s2-1581.inc"
};

static enum cbm_device_type_e device_type = cbm_dt_unknown;

static int s2_read_byte(CBM_FILE fd, unsigned char *c)
{
    int i;
    *c = 0;
    for(i=4; i>0; i--) {
                                                                        SETSTATEDEBUG(DebugBitCount = i*2);
        cbm_iec_release(fd, IEC_ATN);
                                                                        SETSTATEDEBUG((void)0);
#ifndef USE_CBM_IEC_WAIT
        while(cbm_iec_get(fd, IEC_CLOCK)) {
        }
                                                                        SETSTATEDEBUG((void)0);
        *c = (*c>>1) | (cbm_iec_get(fd, IEC_DATA) ? 0x80 : 0);
#else
        *c = (*c>>1) | ((cbm_iec_wait(fd, IEC_CLOCK, 0) & IEC_DATA) ? 0x80 : 0);
#endif
                                                                        SETSTATEDEBUG(DebugBitCount--);
        cbm_iec_set(fd, IEC_ATN);
                                                                        SETSTATEDEBUG((void)0);
#ifndef USE_CBM_IEC_WAIT
        while(!cbm_iec_get(fd,IEC_CLOCK)) {
        }
                                                                        SETSTATEDEBUG((void)0);
        *c = (*c>>1) | (cbm_iec_get(fd, IEC_DATA) ? 0x80 : 0);
#else
        *c = (*c>>1) | ((cbm_iec_wait(fd, IEC_CLOCK, 1) & IEC_DATA) ? 0x80 : 0);
#endif
    }
                                                                        SETSTATEDEBUG(DebugBitCount = -1);
    cbm_iec_release(fd, IEC_ATN);
                                                                        SETSTATEDEBUG((void)0);
#ifndef USE_CBM_IEC_WAIT
    while(cbm_iec_get(fd, IEC_CLOCK)) {
    }
#else
    cbm_iec_wait(fd, IEC_CLOCK, 0);
#endif
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_set(fd, IEC_ATN);
                                                                        SETSTATEDEBUG((void)0);
#ifndef USE_CBM_IEC_WAIT
    while(!cbm_iec_get(fd, IEC_CLOCK)) {
    }
#else
    cbm_iec_wait(fd, IEC_CLOCK, 1);
#endif
                                                                        SETSTATEDEBUG((void)0);
    return 0;
}

static int s2_write_byte(CBM_FILE fd, unsigned char c)
{
    int i;
    for(i=4; i>0; i--) {
                                                                        SETSTATEDEBUG(DebugBitCount = i*2);
        c & 1 ? cbm_iec_set(fd, IEC_DATA) : cbm_iec_release(fd, IEC_DATA);
        c >>= 1;
                                                                        SETSTATEDEBUG((void)0);
        cbm_iec_release(fd, IEC_ATN);
                                                                        SETSTATEDEBUG((void)0);
#ifndef USE_CBM_IEC_WAIT
        while(cbm_iec_get(fd, IEC_CLOCK)) {
        }
#else
        cbm_iec_wait(fd, IEC_CLOCK, 0);
#endif
                                                                        SETSTATEDEBUG(DebugBitCount--);
        c & 1 ? cbm_iec_set(fd, IEC_DATA) : cbm_iec_release(fd, IEC_DATA);
        c >>= 1;
                                                                        SETSTATEDEBUG((void)0);
        cbm_iec_set(fd, IEC_ATN);

                                                                        SETSTATEDEBUG((void)0);
#ifndef USE_CBM_IEC_WAIT
        while(!cbm_iec_get(fd, IEC_CLOCK)) {
        }
#else
        cbm_iec_wait(fd, IEC_CLOCK, 1);
#endif
    }
                                                                        SETSTATEDEBUG(DebugBitCount = -1);
    cbm_iec_release(fd, IEC_DATA);
                                                                        SETSTATEDEBUG((void)0);
    return 0;
}


static int
set_device_type(enum cbm_device_type_e dt)
{
    device_type = dt;

    switch (dt)
    {
    case cbm_dt_cbm1541:
    case cbm_dt_cbm1570:
    case cbm_dt_cbm1571:
    case cbm_dt_cbm1581:
        break;

    case cbm_dt_unknown:
        /* FALL THROUGH */

    default:
        DBG_ERROR((DBG_PREFIX "unknown device type!"));
        device_type = cbm_dt_unknown;
        return 1;
    }

    return 0;
}

static int
upload(CBM_FILE fd, unsigned char drive)
{
    const unsigned char *s2_drive_prog = 0;
    unsigned int s2_drive_prog_length = 0;

    unsigned int bytesWritten;

    switch (device_type)
    {
    case cbm_dt_cbm1581:
        DBG_PRINT((DBG_PREFIX "recognized 1581."));
        s2_drive_prog = s2_1581_drive_prog;
        s2_drive_prog_length = sizeof(s2_1581_drive_prog);
        break;

    case cbm_dt_cbm1541:
        DBG_PRINT((DBG_PREFIX "recognized 1541."));
        s2_drive_prog = s2_1541_1571_drive_prog;
        s2_drive_prog_length = sizeof(s2_1541_1571_drive_prog);
        break;

    case cbm_dt_cbm1570:
    case cbm_dt_cbm1571:
        DBG_PRINT((DBG_PREFIX "recognized 1571."));
        s2_drive_prog = s2_1541_1571_drive_prog;
        s2_drive_prog_length = sizeof(s2_1541_1571_drive_prog);
        break;

    case cbm_dt_unknown:
        /* FALL THROUGH */

    default:
        DBG_ERROR((DBG_PREFIX "unknown device type!"));
        return 1;
    }

    bytesWritten = cbm_upload(fd, drive, 0x700, s2_drive_prog, s2_drive_prog_length);

    if (bytesWritten != s2_drive_prog_length)
    {
        DBG_ERROR((DBG_PREFIX "wanted to write %u bytes, but only %u "
            "bytes could be written", s2_drive_prog_length, bytesWritten));

        return 1;
    }

    return 0;
}

static int
init(CBM_FILE fd, unsigned char drive)
{
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_release(fd, IEC_CLOCK);
                                                                        SETSTATEDEBUG((void)0);
    while(!cbm_iec_get(fd, IEC_CLOCK)) {
    }
                                                                        SETSTATEDEBUG((void)0);
    cbm_iec_set(fd, IEC_ATN);
                                                                        SETSTATEDEBUG((void)0);
    arch_sleep_ms(20);

    return 0;
}

static int
read1byte(CBM_FILE fd, unsigned char *c1)
{
    int ret;
                                                                        SETSTATEDEBUG(DebugByteCount = -6401);
    ret = s2_read_byte(fd, c1);
                                                                        SETSTATEDEBUG(DebugByteCount = -1);
    return ret;
}

static int
read2byte(CBM_FILE fd, unsigned char *c1, unsigned char *c2)
{
    int ret = 0;
                                                                        SETSTATEDEBUG(DebugByteCount = -25601);
    ret = s2_read_byte(fd, c1);
                                                                        SETSTATEDEBUG(DebugByteCount = -12802);
    if (ret == 0)
        ret = s2_read_byte(fd, c2);
                                                                        SETSTATEDEBUG(DebugByteCount = -1);
    return 1;
}

static int
readblock(CBM_FILE fd, unsigned char *p, unsigned int length)
{
                                                                        SETSTATEDEBUG(DebugByteCount = 0);
    for (; length < 0x100; length++)
    {
                                                                        SETSTATEDEBUG(DebugByteCount++);
        if (s2_read_byte(fd, p++))
        {
                                                                        SETSTATEDEBUG(DebugByteCount = -1);
            return 1;
        }
    }
                                                                        SETSTATEDEBUG(DebugByteCount = -1);
    return 0;
}

static int
write1byte(CBM_FILE fd, unsigned char c1)
{
    int ret;
                                                                        SETSTATEDEBUG(DebugByteCount = -6401);
    ret = s2_write_byte(fd, c1);
                                                                        SETSTATEDEBUG(DebugByteCount = -1);
    return ret;
}

static int
write2byte(CBM_FILE fd, unsigned char c1, unsigned char c2)
{
    int ret = 0;
                                                                        SETSTATEDEBUG(DebugByteCount = -12801);
    ret = s2_write_byte(fd, c1);
                                                                        SETSTATEDEBUG(DebugByteCount = -12802);
    if (ret == 0)
        ret = s2_write_byte(fd, c2);
                                                                        SETSTATEDEBUG(DebugByteCount = -1);
    return 1;
}

static int
writeblock(CBM_FILE fd, unsigned char *p, unsigned int length)
{
                                                                        SETSTATEDEBUG(DebugByteCount = 0);
    for (; length < 0x100; length++)
    {
                                                                        SETSTATEDEBUG(DebugByteCount++);
        if (s2_write_byte(fd, *p++))
        {
                                                                        SETSTATEDEBUG(DebugByteCount = -1);
            return 1;
        }
    }
                                                                        SETSTATEDEBUG(DebugByteCount = -1);
    return 0;
}

DECLARE_TRANSFER_FUNCS(s2);
