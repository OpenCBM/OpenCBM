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
    "@(#) $Id: gcr.c,v 1.1 2004-11-07 11:05:12 strik Exp $";
#endif

#include "gcr.h"

#include <stdlib.h>

int gcr_decode(unsigned const char *gcr, unsigned char *decoded)
{
    static const unsigned char lo[] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0x08, 0x00, 0x01, 0xff, 0x0c, 0x04, 0x05,
        0xff, 0xff, 0x02, 0x03, 0xff, 0x0f, 0x06, 0x07,
        0xff, 0x09, 0x0a, 0x0b, 0xff, 0x0d, 0x0e, 0xff
    };

    static const unsigned char hi[] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0x80, 0x00, 0x10, 0xff, 0xc0, 0x40, 0x50,
        0xff, 0xff, 0x20, 0x30, 0xff, 0xf0, 0x60, 0x70,
        0xff, 0x90, 0xa0, 0xb0, 0xff, 0xd0, 0xe0, 0xff
    };

    unsigned char ref, chk = 0, hdr, *c = NULL;
    register unsigned char g0, g1, g2, g3, g4;
    int i;

    g0 = *(gcr++);
    g1 = *(gcr++);

    hdr = hi[(g0 >> 3) & 0x1f] |
          lo[((g0 & 0x07) << 2) | ((g1 >> 6) & 0x03)];

    if(hdr != 0x07) {
        return 4;
    }

    for(i = 0; i < BLOCKSIZE; i+=4) {
        g2 = *(gcr++);
        g3 = *(gcr++);
        g4 = *(gcr++);
        if(c) {
            chk ^= *c++ = hi[(g0 >> 3) & 0x1f] |
                          lo[((g0 & 0x07) << 2) | ((g1 >> 6) & 0x03)];
        } else {
            c = decoded;
        }
        chk ^= *c++ = hi[(g1 >> 1) & 0x1f] |
                      lo[((g1 & 0x01) << 4) | ((g2 >> 4) & 0x0f)];
        chk ^= *c++ = hi[((g2 & 0x0f) << 1) | ((g3 >> 7) & 0x01)] |
                      lo[(g3 >> 2) & 0x1f];
        chk ^= *c++ = hi[((g3 & 0x03) << 3) | ((g4 >> 5) & 0x07)] |
                      lo[g4 & 0x1f];
        g0 = *(gcr++);
        g1 = *(gcr++);
    }
    chk ^= *c = hi[(g0 >> 3) & 0x1f] |
                lo[((g0 & 0x07) << 2) | ((g1 >> 6) & 0x03)];
    g2 = *gcr;
    ref = hi[(g1 >> 1) & 0x1f] |
          lo[((g1 & 0x01) << 4) | ((g2 >> 4) & 0x0f)];

    return (chk != ref) ? 5 : 0;
}

int gcr_encode(unsigned const char *block, unsigned char *encoded)
{
    register unsigned char c = 0x07, h, l, g0, g1, g2, g3, g4;
    unsigned char chk = 0;
    int i;

    static const unsigned char gcr[] = {
        0x0a, 0x0b, 0x12, 0x13, 0x0e, 0x0f, 0x16, 0x17,
        0x09, 0x19, 0x1a, 0x1b, 0x0d, 0x1d, 0x1e, 0x15
    };

    static unsigned char empty[] = {
        0x00, 0x00, 0x00, 0x00
    };

    for(i = 0; i <= BLOCKSIZE; i+=4) {
        if(i == BLOCKSIZE) {
            block    = empty;
            empty[0] = chk;
        }
        h    = gcr[c >> 4]; l = gcr[c & 0x0f];
        g0   = (h << 3) | (l >> 2);
        g1   = (l & 0x03) << 6;
        chk ^= c = *(block++);
        h    = gcr[c >> 4]; l = gcr[c & 0x0f];
        g1  |= (h << 1) | (l >> 4);
        g2   = (l & 0x0f) << 4;
        chk ^= c = *(block++);
        h    = gcr[c >> 4]; l = gcr[c & 0x0f];
        g2  |= (h >> 1);
        g3   = ((h & 0x01) << 7) | (l << 2);
        chk ^= c = *(block++);
        h    = gcr[c >> 4]; l = gcr[c & 0x0f];
        g3  |= (h >> 3);
        g4   = ((h & 0x07) << 5) | (l);

        *(encoded++) = g0;
        *(encoded++) = g1;
        *(encoded++) = g2;
        *(encoded++) = g3;
        *(encoded++) = g4;

        chk ^= c = *(block++);
    }
    return 0;
}
