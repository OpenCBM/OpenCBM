/*
 * Protection handlers for MNIB
 * Copyright 2004-2005 Pete Rittwage <peter@rittwage.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gcr.h"
#include "prot.h"

void
shift_buffer(BYTE * buffer, int length, int n)
{
	int i;
	BYTE tempbuf[0x2000];
	BYTE carry;
	int carryshift;

	carryshift = 8 - n;
	memcpy(tempbuf, buffer, length);

	// shift buffer right by n bits
	carry = tempbuf[length - 1];
	for (i = 0; i < length; i++)
	{
		buffer[i] = (tempbuf[i] >> n) | (carry << carryshift);
		carry = tempbuf[i];
	}
}

BYTE *
align_vmax(BYTE * work_buffer, int tracklen)
{
	BYTE *pos, *buffer_end;
	int run;

	run = 0;
	pos = work_buffer;
	buffer_end = work_buffer + tracklen;

	/*
	 * Some games have all their tracks aligned to $49 bytes
	 * while others use a run of $55 $AA.
	 * need to find a way to discern/search them
	 */

	/* Try to find 0x5a track 20 marker bytes */
	while (pos < buffer_end)
	{
		// duplicator's marker?
		if (*pos == 0x5a)
		{
			if (run == 5)	// it's a marker
				return (pos - 5);

			run++;
		}
		else
			run = 0;

		pos++;
	}

	return (0);
}

// Line up the track cycle to the start of the longest gap mark
// this helps some custom protection tracks master properly
BYTE *
auto_gap(BYTE * work_buffer, int tracklen)
{
	BYTE *pos, *buffer_end, *key_temp, *key;
	int run, longest;

	run = 0;
	longest = 0;
	pos = work_buffer;
	buffer_end = work_buffer + tracklen;
	key = key_temp = NULL;

	/* try to find longest non-bad gcr run */
	while (pos < buffer_end)
	{
		if (*pos == *(pos + 1))	// && (*pos != 0x00 ))
		{
			key_temp = pos + 2;
			run++;
		}
		else
		{
			if (run > longest)
			{
				key = key_temp;
				longest = run;
				//gapbyte = *pos;
			}
			run = 0;
		}
		pos++;
	}

	/* first byte after gap */
	// printf("gapbyte: %x, len: %d\n",gapbyte,longest);
	return (key);
}

// The idea behind this is that weak bits commonly occur
// at the ends of tracks when they were mastered.
// we can line up the track cycle to this
// in lieu of no other hints
BYTE *
find_weak_gap(BYTE * work_buffer, int tracklen)
{
	BYTE *pos, *buffer_end, *key_temp, *key;
	int run, longest;

	run = 0;
	longest = 0;
	pos = work_buffer;
	buffer_end = work_buffer + tracklen;
	key = key_temp = NULL;

	/* try to find longest bad gcr run */
	while (pos < buffer_end)
	{
		if (is_bad_gcr(work_buffer, buffer_end - work_buffer,
		  pos - work_buffer))
		{
			// mark next GCR byte
			key_temp = pos + 1;
			run++;
		}
		else
		{
			if (run > longest)
			{
				key = key_temp;
				longest = run;
			}
			run = 0;
		}
		pos++;
	}

	/* first byte after bad run */
	return (key);
}

// Line up the track cycle to the start of the longest sync mark
// this helps some custom protection tracks master properly
BYTE *
find_long_sync(BYTE * work_buffer, int tracklen)
{
	BYTE *pos, *buffer_end, *key_temp, *key;
	int run, longest;

	run = 0;
	longest = 0;
	pos = work_buffer;
	buffer_end = work_buffer + tracklen;
	key = key_temp = NULL;

	/* try to find longest sync run */
	while (pos < buffer_end)
	{
		if (*pos == 0xff)
		{
			if (run == 0)
				key_temp = pos;
			run++;
		}
		else
		{
			if (run > longest)
			{
				key = key_temp;
				longest = run;
			}
			run = 0;
		}
		pos++;
	}

	/* first byte after longest sync run */
	return (key);
}
