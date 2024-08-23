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

#include "opencbm-plugin.h"

#include <stdlib.h>

#include "arch.h"

static opencbm_plugin_s3_read_n_t  * opencbm_plugin_s3_read_n  = NULL;
static opencbm_plugin_s3_write_n_t * opencbm_plugin_s3_write_n = NULL;


static const unsigned char s3_1571_drive_prog[] = {
#include "s3-1571.inc"
};

static const unsigned char s3_1581_drive_prog[] = {
#include "s3-1581.inc"
};

static const unsigned char s3_fdx000_drive_prog[] = {
#include "s3-fdx000.inc"
};

static enum cbm_device_type_e device_type = cbm_dt_unknown;

static int
s3_read(CBM_FILE fd, unsigned char *buffer, size_t count)
{
    int ret = -1;

    if (opencbm_plugin_s3_read_n) {
        ret = opencbm_plugin_s3_read_n(fd, buffer, count);
    }

    return ret == count;
}

static int
s3_write(CBM_FILE fd, unsigned char *buffer, size_t count)
{
    int ret = -1;

    if (opencbm_plugin_s3_write_n) {
        ret = opencbm_plugin_s3_write_n(fd, buffer, count);
    }

    return ret == count;
}


static int
set_device_type(enum cbm_device_type_e dt)
{
    device_type = dt;

    switch (dt)
    {
    case cbm_dt_fdx000:
        /* FALL THROUGH */
    case cbm_dt_cbm1581:
        /* FALL THROUGH */
    case cbm_dt_cbm1570:
        /* FALL THROUGH */
    case cbm_dt_cbm1571:
        break;

    case cbm_dt_cbm1541:
        /* FALL THROUGH */

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
    const unsigned char *s3_drive_prog = 0;
    unsigned int s3_drive_prog_length = 0;

    unsigned int bytesWritten;

    opencbm_plugin_s3_read_n  = cbm_get_plugin_function_address("opencbm_plugin_s3_read_n");
    opencbm_plugin_s3_write_n = cbm_get_plugin_function_address("opencbm_plugin_s3_write_n");

    if (!opencbm_plugin_s3_read_n || !opencbm_plugin_s3_write_n) {
        DBG_ERROR((DBG_PREFIX "s3 is not defined in this plugin"));
        return 1;
    }

    switch (device_type)
    {
    case cbm_dt_fdx000:
        DBG_PRINT((DBG_PREFIX "recognized FDx000 (FD2000/FD4000)."));
        s3_drive_prog = s3_fdx000_drive_prog;
        s3_drive_prog_length = sizeof(s3_fdx000_drive_prog);
        break;

    case cbm_dt_cbm1581:
        DBG_PRINT((DBG_PREFIX "recognized 1581."));
        s3_drive_prog = s3_1581_drive_prog;
        s3_drive_prog_length = sizeof(s3_1581_drive_prog);
        break;

    case cbm_dt_cbm1541:
        DBG_PRINT((DBG_PREFIX "recognized 1541 - not supported!"));
        return 1;

    case cbm_dt_cbm1570:
    case cbm_dt_cbm1571:
        DBG_PRINT((DBG_PREFIX "recognized 1570/1571."));
        s3_drive_prog = s3_1571_drive_prog;
        s3_drive_prog_length = sizeof(s3_1571_drive_prog);
        break;

    case cbm_dt_unknown:
        /* FALL THROUGH */

    default:
        DBG_ERROR((DBG_PREFIX "unknown device type!"));
        return 1;
    }

    bytesWritten = cbm_upload(fd, drive, 0x700, s3_drive_prog, s3_drive_prog_length);

    if (bytesWritten != s3_drive_prog_length)
    {
        DBG_ERROR((DBG_PREFIX "wanted to write %u bytes, but only %u "
            "bytes could be written", s3_drive_prog_length, bytesWritten));

        return 1;
    }

    return 0;
}

static int
init(CBM_FILE fd, unsigned char drive)
{
    cbm_iec_release(fd, IEC_CLOCK | IEC_DATA | IEC_ATN | IEC_SRQ);

    while(cbm_iec_get(fd, IEC_CLOCK))
        ;

    cbm_iec_set(fd, IEC_DATA);

    return 0;
}

static int
read2byte(CBM_FILE fd, unsigned char *c1, unsigned char *c2)
{
    unsigned char buffer[2];

    int ret = 0;

    ret = s3_read(fd, buffer, sizeof buffer);

    if (ret) {
        *c1 = buffer[0];
        *c2 = buffer[1];
    }
    return ret;
}

static int
read1byte(CBM_FILE fd, unsigned char *c1)
{
    unsigned char dummy;

    return read2byte(fd, c1, &dummy);
}

static int
readblock(CBM_FILE fd, unsigned char *p, unsigned int length)
{
    return s3_read(fd, p, 0x100 - length);
}

static int
write2byte(CBM_FILE fd, unsigned char c1, unsigned char c2)
{
    unsigned char buffer[2] = { c1, c2 };

    return s3_write(fd, buffer, sizeof buffer);
}

static int
write1byte(CBM_FILE fd, unsigned char c1)
{
    return write2byte(fd, c1, 0);
}

static int
writeblock(CBM_FILE fd, unsigned char *p, unsigned int length)
{
    return s3_write(fd, p, 0x100 - length);
}

DECLARE_TRANSFER_FUNCS(s3);
