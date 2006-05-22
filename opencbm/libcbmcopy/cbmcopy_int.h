/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *  Copyright 2001 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
*/

/* $Id: cbmcopy_int.h,v 1.7 2006-05-22 08:34:19 wmsr Exp $ */

#ifndef CBMCOPY_INT_H
#define CBMCOPY_INT_H

#include "opencbm.h"
#include "cbmcopy.h"

typedef struct {
    int  (*write_byte)(CBM_FILE, unsigned char); 
    unsigned char (*read_byte)(CBM_FILE); 
    int  (*check_error)(CBM_FILE,int); 
    int  (*upload_turbo)(CBM_FILE, unsigned char, enum cbm_device_type_e,int);
    int  (*start_turbo)(CBM_FILE,int);
    void (*exit_turbo)(CBM_FILE,int);
} transfer_funcs;

#ifdef LIBCBMCOPY_DEBUG
    extern volatile signed int debugCBMcopyLineNumber, debugCBMcopyBitCount;
    extern volatile char *     debugCBMcopyFileName;
#   define SETSTATEDEBUG(_x)  \
        debugCBMcopyLineNumber=__LINE__; \
        debugCBMcopyFileName  =__FILE__; \
        (_x)
#else
#   define SETSTATEDEBUG(_x) (void)0
#endif

#define DECLARE_TRANSFER_FUNCS(x) \
    transfer_funcs cbmcopy_ ## x = {write_byte, read_byte, check_error, \
                        upload_turbo, start_turbo, exit_turbo}

#endif
