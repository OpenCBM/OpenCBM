/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Copyright 2009 Arnd Menge <arnd(at)jonnz(dot)de>
*/

#ifndef CBM_MODULE_H
#define CBM_MODULE_H

#if defined(__linux__)
#include <linux/ioctl.h>
#include <linux/types.h>
#else
#if defined(__FreeBSD__)
#ifndef BUILDING_FREEBSD_MODULE
#include <sys/ioctl.h>
#endif
#endif
#endif

/* all values needed by PARBURST_READ_TRACK and PARBURST_WRITE_TRACK */
typedef struct PARBURST_RW_VALUE {
    unsigned char *buffer;
    int length;
} PARBURST_RW_VALUE;

#define CBMCTRL_BASE        0xcb

#define CBMCTRL_TALK        _IOW(CBMCTRL_BASE, 0, int)
#define CBMCTRL_LISTEN      _IOW(CBMCTRL_BASE, 1, int)
#define CBMCTRL_UNTALK      _IO(CBMCTRL_BASE, 2)
#define CBMCTRL_UNLISTEN    _IO(CBMCTRL_BASE, 3)
#define CBMCTRL_OPEN        _IOW(CBMCTRL_BASE, 4, int)
#define CBMCTRL_CLOSE       _IOW(CBMCTRL_BASE, 5, int)
#define CBMCTRL_RESET       _IO(CBMCTRL_BASE, 6)
#define CBMCTRL_GET_EOI     _IOR(CBMCTRL_BASE, 7, int)
#define CBMCTRL_CLEAR_EOI   _IO(CBMCTRL_BASE, 8)

#define CBMCTRL_PP_READ     _IOR(CBMCTRL_BASE, 10, unsigned char)
#define CBMCTRL_PP_WRITE    _IOW(CBMCTRL_BASE, 11, unsigned char)
#define CBMCTRL_IEC_POLL    _IOR(CBMCTRL_BASE, 12, int)
#define CBMCTRL_IEC_SET     _IOW(CBMCTRL_BASE, 13, int)
#define CBMCTRL_IEC_RELEASE _IOW(CBMCTRL_BASE, 14, int)
#define CBMCTRL_IEC_WAIT    _IOWR(CBMCTRL_BASE, 15, int)
#define CBMCTRL_IEC_SETRELEASE _IOW(CBMCTRL_BASE, 16, int)

/* linux constants needed by parallel burst */
#define CBMCTRL_PARBURST_READ    _IOR(CBMCTRL_BASE, 17, unsigned char)
#define CBMCTRL_PARBURST_WRITE   _IOW(CBMCTRL_BASE, 18, unsigned char)
#define CBMCTRL_PARBURST_READ_TRACK        _IOW(CBMCTRL_BASE, 19, PARBURST_RW_VALUE)
#define CBMCTRL_PARBURST_WRITE_TRACK _IOW(CBMCTRL_BASE, 20, PARBURST_RW_VALUE)
#define CBMCTRL_PARBURST_READ_TRACK_VAR    _IOW(CBMCTRL_BASE, 21, PARBURST_RW_VALUE)

#endif
