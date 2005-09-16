#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gcr.h"


void shift_buffer_left(BYTE *buffer, int length, int n)
{
    int i;
    BYTE tempbuf[0x2001];
    BYTE carry;
    int carryshift = 8 - n;

    memcpy(tempbuf, buffer, length);
    tempbuf[length] = 0x00;

    // shift buffer left by n bits
    for (i = 0; i < length; i++)
    {
        carry = tempbuf[i+1];
        buffer[i] = (tempbuf[i] << n) | (carry >> carryshift);
    }
}


void shift_buffer_right(BYTE *buffer, int length, int n)
{
    int i;
    BYTE tempbuf[0x2000];
    BYTE carry;
    int carryshift = 8 - n;

    memcpy(tempbuf, buffer, length);

    // shift buffer right by n bits
    carry = 0;
    for (i = 0; i < length; i++)
    {
        buffer[i] = (tempbuf[i] >> n) | (carry << carryshift);
        carry = tempbuf[i];
    }
}



BYTE* align_vmax(BYTE *work_buffer, int track_len)
{
	BYTE *pos;
	BYTE *buffer_end;
	BYTE *key, *tempkey;
    int run = 0;

	key = tempkey = NULL;
	pos = work_buffer;
	buffer_end = work_buffer + (2 * track_len);

	// shift up to 7 times to search
	//for(i = 0; i < 7; i ++)
	{
	    /* try to find duplicator's marker bytes */
	    while (pos < buffer_end)
	    {
	        if(*pos == 0x5a) // duplicator's marker?
	        {
				if(run == 5)	// it's a marker
				{
					//printf("V-MAX! ");
					key = pos - 5;
					goto done;
				}
				run ++;
			}
			else
				run = 0;

			pos ++;
	    }
	}

done:
    /* return first marker byte or NULL */
    return (key);
}



BYTE* align_vorpal(BYTE *work_buffer, int track_len)
{
	/* this is a work in progress */

	int i, j;
	int found = 0;

	// check manually for NOSYNC
	for(i = 0; i < track_len - 1; i ++)
	{
		if( ((work_buffer[i] & 0x03) == 0x03) && (work_buffer[i+1] == 0xff) )
			return NULL;
	}

	// shift up to 7 times to search
	for(j = 0; j < 7; j ++)
	{
		found = 0;
		for(i = 0; i < track_len - 1; i ++)
		{
			// look for a header signature
			if( ((work_buffer[i] == 0x55) && (work_buffer[i+1] == 0xaa)) )
			{
				// found it, align and break
				found = 1;
				//printf(" VORPAL! ");
				return (work_buffer + i);
			}
		}
		shift_buffer_right(work_buffer, track_len, 1);
	}
	return NULL;
}

