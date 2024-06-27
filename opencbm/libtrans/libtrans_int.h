/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2005 Michael Klein <michael.klein@puffin.lb.shuttle.de>
*/

#ifndef LIBTRANS_INT_H
#define LIBTRANS_INT_H

#include "opencbm.h"
#include "libtrans.h"

#include "arch.h"

#ifdef LIBOCT_STATE_DEBUG
# define DEBUG_STATEDEBUG
#endif
#include "statedebug.h"

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! The name of the executable */
#define DBG_PROGNAME "LIBOPENCBMTRANSFER.DLL"

#include "debug.h"


typedef struct {
    int (*set_device_type)(enum cbm_device_type_e device_type);
    int (*upload)    (CBM_FILE fd, unsigned char drive);
    int (*init)      (CBM_FILE fd, unsigned char drive);
    int (*read1byte) (CBM_FILE fd, unsigned char *c1);
    int (*read2byte) (CBM_FILE fd, unsigned char *c1, unsigned char *c2);
    int (*readblock) (CBM_FILE fd, unsigned char *p, unsigned int length);
    int (*write1byte)(CBM_FILE fd, unsigned char c1);
    int (*write2byte)(CBM_FILE fd, unsigned char c1, unsigned char c2);
    int (*writeblock)(CBM_FILE fd, unsigned char *p, unsigned int length);
} transfer_funcs;

#define DECLARE_TRANSFER_FUNCS(_name_) \
    transfer_funcs libopencbmtransfer_ ## _name_ = \
    { \
        set_device_type, \
        upload,     \
        init,       \
        read1byte,  \
        read2byte,  \
        readblock,  \
        write1byte, \
        write2byte, \
        writeblock  \
    }

extern transfer_funcs libopencbmtransfer_s1;
extern transfer_funcs libopencbmtransfer_s2;
extern transfer_funcs libopencbmtransfer_pp;

#endif /* #ifndef LIBTRANS_INT_H */
