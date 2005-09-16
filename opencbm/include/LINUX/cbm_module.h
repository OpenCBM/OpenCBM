/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999 Michael Klein <michael.klein@puffin.lb.shuttle.de>
*/

/* $Id: cbm_module.h,v 1.1.4.1 2005-09-16 12:39:54 strik Exp $ */

#ifndef CBM_MODULE_H
#define CBM_MODULE_H

#include <linux/ioctl.h>
#include <linux/types.h>

#define CBMCTRL_BASE	0xcb

#define CBMCTRL_TALK	    _IO(CBMCTRL_BASE, 0)
#define CBMCTRL_LISTEN	    _IO(CBMCTRL_BASE, 1)
#define CBMCTRL_UNTALK      _IO(CBMCTRL_BASE, 2)
#define CBMCTRL_UNLISTEN    _IO(CBMCTRL_BASE, 3)
#define CBMCTRL_OPEN        _IO(CBMCTRL_BASE, 4)
#define CBMCTRL_CLOSE       _IO(CBMCTRL_BASE, 5)
#define CBMCTRL_RESET       _IO(CBMCTRL_BASE, 6)
#define CBMCTRL_GET_EOI     _IO(CBMCTRL_BASE, 7)
#define CBMCTRL_CLEAR_EOI   _IO(CBMCTRL_BASE, 8)

#define CBMCTRL_PP_READ     _IO(CBMCTRL_BASE, 10)
#define CBMCTRL_PP_WRITE    _IO(CBMCTRL_BASE, 11)
#define CBMCTRL_IEC_POLL    _IO(CBMCTRL_BASE, 12)
#define CBMCTRL_IEC_SET     _IO(CBMCTRL_BASE, 13)
#define CBMCTRL_IEC_RELEASE _IO(CBMCTRL_BASE, 14)
#define CBMCTRL_IEC_WAIT    _IO(CBMCTRL_BASE, 15)
#define CBMCTRL_IEC_SETRELEASE _IO(CBMCTRL_BASE, 16)

/*linux constants needed by mnib */
#define CBMCTRL_MNIB_PAR_READ    _IO(CBMCTRL_BASE, 17)
#define CBMCTRL_MNIB_PAR_WRITE   _IO(CBMCTRL_BASE, 18)
#define CBMCTRL_MNIB_READ_TRACK        _IO(CBMCTRL_BASE, 19)
#define CBMCTRL_MNIB_WRITE_TRACK _IO(CBMCTRL_BASE, 20)

typedef struct MNIB_RW_VALUE {     // all values needed by MNIB_READ_TRACK and MNIB_WRITE_TRACK
       unsigned char *buffer;
       int length;
       int mode;
} MNIB_RW_VALUE;

#endif
