/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2006 Wolfgang Moser, http://d81.de
 */

#ifdef SAVE_RCSID
static char *rcsid =
    "@(#) $Id: gcr.c,v 1.3 2006-05-06 21:53:58 wmsr Exp $";
#endif

#include "gcr.h"

#include <stdlib.h>

#include <stdio.h>

void gcr_5_to_4_decode(const unsigned char *source, unsigned char *dest)
{
		/* -1 denote illegar GCR bytes, for error checking extensions */
	static const unsigned char decodeGCR[32] =
	    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  8,  0,  1, -1, 12,  4,  5,
	      -1, -1,  2,  3, -1, 15,  6,  7, -1,  9, 10, 11, -1, 13, 14, -1 };
    int i;
        /* at least 24 bits for shifting into bits 16...20 */
    register unsigned int tdest = *source;

    tdest <<= 13;

    for(i = 5; i < 13; i += 2, dest++)
    {
        source++;
        tdest  |= ((unsigned int)(*source)) << i;

            // "tdest >> 16" could be optimized to a word
            // aligned access, hopefully the compiler does
            // this for us (in a portable way)
        *dest   = decodeGCR[ (tdest >> 16) & 0x1f ] << 4;
        tdest <<= 5;

        *dest  |= decodeGCR[ (tdest >> 16) & 0x1f ];
        tdest <<= 5;
    }
}

void gcr_4_to_5_encode(const unsigned char *source, unsigned char *dest)
{
	static const unsigned char encodeGCR[16] =
	    { 10, 11, 18, 19, 14, 15, 22, 23, 9, 25, 26, 27, 13, 29, 30, 21 };
    int i;
    	/* at least 16 bits for overflow shifting */
    register unsigned int tdest = 0;

    for(i = 2; i < 10; i += 2, source++, dest++)
    {
        tdest <<= 5;  /* make room for the upper nybble */
        tdest  |= encodeGCR[ (*source) >>   4 ];

        tdest <<= 5;  /* make room for the lower nybble */
        tdest  |= encodeGCR[ (*source) & 0x0f ];

        *dest   = (unsigned char)(tdest >> i);
    }

    *dest = (unsigned char)tdest;
}

int gcr_decode(unsigned const char *gcr, unsigned char *decoded)
{
	unsigned char chkref[4], chksum = 0;
	int i,j;

   	gcr_5_to_4_decode(gcr, chkref);
   	gcr += 5;

   	if(chkref[0] != 0x07)
   	{
		return 4;
   	}

		/* move over the remaining three bytes */
	for(j = 1; j < 4; j++, decoded++)
	{
		*decoded = chkref[j];
		chksum  ^= chkref[j];
	}

		/* main block processing loop */
    for(i = 1; i < BLOCKSIZE/4; i++)
    {
    	gcr_5_to_4_decode(gcr, decoded);
    	gcr += 5;

		for(j = 0; j < 4; j++, decoded++)
		{
			chksum ^= *decoded;
		}
	}

	gcr_5_to_4_decode(gcr, chkref);
		/* move over the remaining last byte */
	*decoded = chkref[0];
	chksum  ^= chkref[0];

	return (chkref[1] != chksum) ? 5 : 0;
}

int gcr_encode(unsigned const char *block, unsigned char *encoded)
{

	unsigned char chkref[4] = { 0x07, 0, 0, 0};
	int i,j;

		/* start with encoding the data block
		 * identifier and the first three bytes
		 */
	for(j = 1; j < 4; j++, block++)
	{
		chkref[j] = *block;
	}
   	gcr_4_to_5_encode(chkref, encoded);
   	encoded += 5;

		/* add the three bytes into one checksum */
	chkref[1] ^= (chkref[2] ^ chkref[3]);

		/* main block processing loop */
    for(i = 1; i < BLOCKSIZE/4; i++)
    {
    	gcr_4_to_5_encode(block, encoded);
    	encoded += 5;

		for(j = 0; j < 4; j++, block++)
		{
				/* directly encode the checksum */
			chkref[1] ^= *block;
		}
	}

		/* move over the remaining last byte */
	chkref[0]  = *block;
		/* add the last byte to the checksum */
	chkref[1] ^= *block;
   	gcr_4_to_5_encode(chkref, encoded);

    return 0;
}
