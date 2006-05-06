/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Copyright 2006 Wolfgang Moser, http://d81.de
 */

/* $Id: gcr.h,v 1.3 2006-05-06 21:53:58 wmsr Exp $ */

#ifndef GCR_H
#define GCR_H

#define BLOCKSIZE   256
#define GCRBUFSIZE  326

#ifdef __cplusplus
extern "C" {
#endif

extern void gcr_5_to_4_decode(const unsigned char *source, unsigned char *dest);
extern void gcr_4_to_5_encode(const unsigned char *source, unsigned char *dest);

extern int  gcr_decode(const unsigned char *gcr,   unsigned char *decoded);
extern int  gcr_encode(const unsigned char *block, unsigned char *encoded);

#ifdef __cplusplus
}
#endif

#endif
