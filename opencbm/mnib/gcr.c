/* gcr.c - Group Code Recording helper functions

    (C) 2001-2005 Markus Brenner <markus@brenner.de>
    	and Pete Rittwage <peter@rittwage.com>
        based on code by Andreas Boose

    V 0.30   created file based on n2d
    V 0.31   improved error handling of convert_GCR_sector()
    V 0.32   removed some functions, added sector-2-GCR conversion
    V 0.33   improved sector extraction, added find_track_cycle() function
    V 0.34   added MAX_SYNC_OFFSET constant, for better error conversion
    V 0.35   improved find_track_cycle() function
    V 0.36   added bad GCR code detection
    V 0.36b  improved find_sync(), added find_sector_gap(), find_sector0()
    V 0.36c  convert_GCR_sector: search good header before using a damaged one
             improved find_track_cycle(), return max len if no cycle found
    V 0.36d  Untold number of additions and consequent bugfixes. (pjr)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "gcr.h"
#include "prot.h"

char sector_map_1541[43] =
{
    0,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21,     /*  1 - 10 */
    21, 21, 21, 21, 21, 21, 21, 19, 19, 19,     /* 11 - 20 */
    19, 19, 19, 19, 18, 18, 18, 18, 18, 18,     /* 21 - 30 */
    17, 17, 17, 17, 17,                         /* 31 - 35 */
    17, 17, 17, 17, 17, 17, 17                  /* 36 - 42 (non-standard) */
};


int speed_map_1541[42] =
{
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3,               /*  1 - 10 */
    3, 3, 3, 3, 3, 3, 3, 2, 2, 2,               /* 11 - 20 */
    2, 2, 2, 2, 1, 1, 1, 1, 1, 1,               /* 21 - 30 */
    0, 0, 0, 0, 0,                              /* 31 - 35 */
    0, 0, 0, 0, 0, 0, 0                         /* 36 - 42 (non-standard) */
};

/* Burst Nibbler defaults */
//unsigned int capacity[] = { 0x1857, 0x19f6, 0x1bd1, 0x1df0 };
unsigned int capacity_min[] = { 0x1827, 0x19c6, 0x1ba1, 0x1dc0 };
unsigned int capacity_max[] = { 0x18a7, 0x1a46, 0x1c21, 0x1e90 };

/* Super Card defaults */
unsigned int capacity[] = { 0x1848, 0x1a18, 0x1c08, 0x1e38 };


/* Nibble-to-GCR conversion table */
static BYTE GCR_conv_data[16] =
{
    0x0a, 0x0b, 0x12, 0x13,
    0x0e, 0x0f, 0x16, 0x17,
    0x09, 0x19, 0x1a, 0x1b,
    0x0d, 0x1d, 0x1e, 0x15
};


/* GCR-to-Nibble conversion tables */
static BYTE GCR_decode_high[32] =
{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x80, 0x00, 0x10, 0xff, 0xc0, 0x40, 0x50,
    0xff, 0xff, 0x20, 0x30, 0xff, 0xf0, 0x60, 0x70,
    0xff, 0x90, 0xa0, 0xb0, 0xff, 0xd0, 0xe0, 0xff
};

static BYTE GCR_decode_low[32] =
{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x08, 0x00, 0x01, 0xff, 0x0c, 0x04, 0x05,
    0xff, 0xff, 0x02, 0x03, 0xff, 0x0f, 0x06, 0x07,
    0xff, 0x09, 0x0a, 0x0b, 0xff, 0x0d, 0x0e, 0xff
};


int find_sync(BYTE **gcr_pptr, BYTE *gcr_end)
{
    while (1)
    {
        if ((*gcr_pptr) + 1 >= gcr_end)
        {
            *gcr_pptr = gcr_end;
            return 0; /* not found */
        }

		// sync flag goes up after 10 bits
		if ((((*gcr_pptr)[0] & 0x03) == 0x03) && ((*gcr_pptr)[1] == 0xff))
            break;

        (*gcr_pptr)++;
    }

    (*gcr_pptr)++;
    while ((*gcr_pptr < gcr_end) && (**gcr_pptr == 0xff)) (*gcr_pptr)++;
    return (*gcr_pptr < gcr_end);
}


void convert_4bytes_to_GCR(BYTE *buffer, BYTE *ptr)
{
    *ptr = GCR_conv_data[(*buffer) >> 4] << 3;
    *ptr |= GCR_conv_data[(*buffer) & 0x0f] >> 2;
    ptr++;

    *ptr = GCR_conv_data[(*buffer) & 0x0f] << 6;
    buffer++;
    *ptr |= GCR_conv_data[(*buffer) >> 4] << 1;
    *ptr |= GCR_conv_data[(*buffer) & 0x0f] >> 4;
    ptr++;

    *ptr = GCR_conv_data[(*buffer) & 0x0f] << 4;
    buffer++;
    *ptr |= GCR_conv_data[(*buffer) >> 4] >> 1;
    ptr++;

    *ptr = GCR_conv_data[(*buffer) >> 4] << 7;
    *ptr |= GCR_conv_data[(*buffer) & 0x0f] << 2;
    buffer++;
    *ptr |= GCR_conv_data[(*buffer) >> 4] >> 3;
    ptr++;

    *ptr = GCR_conv_data[(*buffer) >> 4] << 5;
    *ptr |= GCR_conv_data[(*buffer) & 0x0f];
}

int convert_4bytes_from_GCR(BYTE *gcr, BYTE *plain)
{
    BYTE hnibble, lnibble;
    int badGCR;
    int nConverted;

    badGCR = 0;

    hnibble = GCR_decode_high[gcr[0] >> 3];
    lnibble = GCR_decode_low[((gcr[0] << 2) | (gcr[1] >> 6)) & 0x1f];
    if (((hnibble==0xff) || (lnibble==0xff)) && (!badGCR)) badGCR = 1;
    *plain++ = hnibble | lnibble;

    hnibble = GCR_decode_high[(gcr[1] >> 1) & 0x1f];
    lnibble = GCR_decode_low[((gcr[1] << 4) | (gcr[2] >> 4)) & 0x1f];
    if (((hnibble==0xff) || (lnibble==0xff)) && (!badGCR)) badGCR = 2;
    *plain++ = hnibble | lnibble;

    hnibble = GCR_decode_high[((gcr[2] << 1) | (gcr[3] >> 7)) & 0x1f];
    lnibble = GCR_decode_low[(gcr[3] >> 2) & 0x1f];
    if (((hnibble==0xff) || (lnibble==0xff)) && (!badGCR)) badGCR = 3;
    *plain++ = hnibble | lnibble;

    hnibble = GCR_decode_high[((gcr[3] << 3) | (gcr[4] >> 5)) & 0x1f];
    lnibble = GCR_decode_low[gcr[4] & 0x1f];
    if (((hnibble==0xff) || (lnibble==0xff)) && (!badGCR)) badGCR = 4;
    *plain++ = hnibble | lnibble;

    nConverted = (badGCR == 0) ? 4 : (badGCR - 1);

    return (nConverted);
}

int extract_id(BYTE *gcr_track, BYTE *id)
{
    BYTE header[10];
    BYTE *gcr_ptr, *gcr_end;

    int track = 18;
    int sector = 0;

    gcr_ptr = gcr_track;
    gcr_end = gcr_track+GCR_TRACK_LENGTH;

    do
    {
        if (!find_sync(&gcr_ptr, gcr_end)) return (0);

        convert_4bytes_from_GCR(gcr_ptr, header);
        convert_4bytes_from_GCR(gcr_ptr+5, header+4);
    } while ((header[0]!=0x08) || (header[2]!=sector) || (header[3]!=track));

    id[0] = header[5];
    id[1] = header[4];
    return (1);
}


BYTE convert_GCR_sector(BYTE *gcr_start, BYTE *gcr_cycle,
                        BYTE *d64_sector,
                        int track, int sector, BYTE *id)
{
    BYTE header[10];    /* block header */
    BYTE hdr_chksum;    /* header checksum */
    BYTE blk_chksum;    /* block  checksum */
    BYTE gcr_buffer[2 * GCR_TRACK_LENGTH];
    BYTE *gcr_ptr, *gcr_end, *gcr_last;
    BYTE *sectordata;
    BYTE error_code;
    int sync_found;
    int track_len;
    int i;
    int nConverted;

    error_code = OK;

    if (track > MAX_TRACK_D64) return (0);
    if ((gcr_cycle == NULL) || (gcr_cycle <= gcr_start)) return (0);

    /* initialize sector data with Original Format Pattern */
    memset(d64_sector, 0x01, 260);
    d64_sector[0] = 0x07; /* Block header mark */
    d64_sector[1] = 0x4b; /* Use Original Format Pattern */

    for (blk_chksum = 0, i = 1; i < 257; i++)
        blk_chksum ^= d64_sector[i + 1];
    d64_sector[257] = blk_chksum;

    /* copy to  temp. buffer with twice the track data */
    track_len = gcr_cycle - gcr_start;
    memcpy(gcr_buffer, gcr_start, track_len);
    memcpy(gcr_buffer+track_len, gcr_start, track_len);
    track_len *= 2;

    /* Check for at least one Sync */
    gcr_end = gcr_buffer+track_len;
    sync_found = 0;
    for (gcr_ptr = gcr_buffer; gcr_ptr < gcr_end; gcr_ptr++)
    {
        if (*gcr_ptr == 0xff)
        {
            if (sync_found < 2) sync_found++;
        }
        else /* (*gcr_ptr != 0xff) */
        {
            if (sync_found < 2) sync_found = 0;
            else sync_found = 3;
        }
    }
    if (sync_found != 3) return(SYNC_NOT_FOUND);

    /* Check for missing SYNCs */
    gcr_last = gcr_ptr = gcr_buffer;
    while (gcr_ptr < gcr_end)
    {
        find_sync(&gcr_ptr, gcr_end);
        if ((gcr_ptr-gcr_last) > MAX_SYNC_OFFSET)
        {
            //printf("no sync for %d\n", gcr_ptr-gcr_last);
            return (SYNC_NOT_FOUND);
        }
        else
            gcr_last = gcr_ptr;
    }

    /* Try to find a good block header for Track/Sector */
    gcr_ptr = gcr_buffer;
    gcr_end = gcr_buffer+track_len;

    do
    {
        if ((!find_sync(&gcr_ptr, gcr_end)) ||
            (gcr_ptr >= gcr_end - 10))
        {
            error_code = HEADER_NOT_FOUND;
            break;
        }
        convert_4bytes_from_GCR(gcr_ptr, header);
        convert_4bytes_from_GCR(gcr_ptr+5, header+4);
        gcr_ptr++;

		//printf("%0.2d: t%0.2d s%0.2d id1:%0.1x id2:%0.1x\n",header[0],header[3],
		//	header[2],header[5],header[4]);

    } while ( (header[0]!=0x08) || (header[2]!=sector) || (header[3]!=track)  ||
              (header[5]!=id[0]) || (header[4]!=id[1]) );

    if (error_code == HEADER_NOT_FOUND)
    {
        error_code = OK;
        /* Try to find next best match for header */
        gcr_ptr = gcr_buffer;
        gcr_end = gcr_buffer+track_len;
        do
        {
            if (!find_sync(&gcr_ptr, gcr_end))
                return (HEADER_NOT_FOUND);
            if (gcr_ptr >= gcr_end - 10)
                return (HEADER_NOT_FOUND);
            convert_4bytes_from_GCR(gcr_ptr, header);
            convert_4bytes_from_GCR(gcr_ptr+5, header+4);
            gcr_ptr++;
        } while ((header[0]==0x07) || (header[2]!=sector) || (header[3]!=track));
    }

    if (header[0] != 0x08)
        error_code = (error_code == OK) ? HEADER_NOT_FOUND : error_code;

    /* Header checksum */
    hdr_chksum = 0;
    for (i = 1; i <= 4; i++)
        hdr_chksum = hdr_chksum ^ header[i];

	if(hdr_chksum != header[5])
	{
		//printf("(S%d HEAD_CHKSUM $%0.2x != $%0,2x) ", sector, hdr_chksum, header[5]);
		error_code = (error_code == OK) ? BAD_HEADER_CHECKSUM : error_code;
	}

    if (!find_sync(&gcr_ptr, gcr_end)) return (DATA_NOT_FOUND);

    for (i = 0, sectordata = d64_sector; i < 65; i++)
    {
        if (gcr_ptr >= gcr_end-5) return (DATA_NOT_FOUND);
        if (4 != (nConverted = convert_4bytes_from_GCR(gcr_ptr, sectordata)))
        {
            if ((i < 64) || (nConverted == 0))
                /* error_code = BAD_GCR_CODE */;
        }
        gcr_ptr += 5;
        sectordata += 4;
    }

    /* check for correct disk ID */
    if ((header[5]!=id[0]) || (header[4]!=id[1]))
        error_code = (error_code == OK) ? ID_MISMATCH : error_code;

    /* check for Block header mark */
    if (d64_sector[0] != 0x07)
        error_code = (error_code == OK) ? DATA_NOT_FOUND : error_code;

    /* Block checksum */
    for (i = 1, blk_chksum = 0; i <= 256; i++)
        blk_chksum ^= d64_sector[i];

	if(blk_chksum != d64_sector[257])
	{
		//printf("(S%d DATA_CHKSUM $%0.2x != $%0.2x) ", sector, blk_chksum, d64_sector[257]);
		error_code = (error_code == OK) ? BAD_DATA_CHECKSUM : error_code;
	}
	return(error_code);
}


void convert_sector_to_GCR(BYTE *buffer, BYTE *ptr,
                           int track, int sector,
                           BYTE *diskID, int error)
{
    int i;
    BYTE buf[4];
    BYTE databuf[0x104];
    BYTE chksum;

    memset(ptr, 0x55, 361);    /* 'unformat' GCR sector */

    if (error == SYNC_NOT_FOUND)
        return;

    if (error == HEADER_NOT_FOUND)
    {
        ptr += 24;
    }
    else
    {
        memset(ptr, 0xff, 5);      /* Sync */
        ptr += 5;

        if (error == ID_MISMATCH)
        {
            diskID[0] ^= 0xff;
            diskID[1] ^= 0xff;
        }

        buf[0] = 0x08;             /* Header identifier */
        buf[1] = sector ^ track ^ diskID[1] ^ diskID[0];
        buf[2] = (BYTE) sector;
        buf[3] = (BYTE) track;

        if (error == BAD_HEADER_CHECKSUM)
            buf[1] ^= 0xff;

        convert_4bytes_to_GCR(buf, ptr);
        ptr += 5;

        buf[0] = diskID[1];
        buf[1] = diskID[0];
        buf[2] = buf[3] = 0x0f;
        convert_4bytes_to_GCR(buf, ptr);
        ptr += 5;

        memset(ptr, 0x55, 9);      /* Header Gap */
        ptr += 9;
    }

    if (error == DATA_NOT_FOUND)
        return;

    memset(ptr, 0xff, 5);      /* Sync */
    ptr += 5;

    chksum = 0;
    databuf[0] = 0x07;
    for (i = 0; i < 0x100; i++)
    {
        databuf[i+1] = buffer[i];
        chksum ^= buffer[i];
    }

    if (error == BAD_DATA_CHECKSUM)
        chksum ^= 0xff;

    databuf[0x101] = chksum;
    databuf[0x102] = 0;        /* 2 bytes filler */
    databuf[0x103] = 0;

    for (i = 0; i < 65; i++)
    {
        convert_4bytes_to_GCR(databuf+(4*i), ptr);
        ptr += 5;
    }

    /* 7 0x55 gap bytes in my reference disk */
    memset(ptr, 0x55, 7);      /* Gap before next sector */
    ptr += 7;
}


int find_track_cycle(BYTE **cycle_start, BYTE **cycle_stop, int cap_min, int cap_max)
{
    BYTE *nib_track;	/* start of nibbled track data */
    BYTE *start_pos;    /* start of periodic area */
    BYTE *cycle_pos;    /* start of cycle repetition */
    BYTE *stop_pos;     /* maximum position allowed for cycle */

    BYTE *data_pos;     /* cycle search variable */
    BYTE *p1, *p2; 		/* local pointers for comparisons */

	nib_track = *cycle_start;
    stop_pos = nib_track + GCR_TRACK_LENGTH - MATCH_LENGTH;
    cycle_pos = NULL;

    /* try to find a normal track cycle  */
    for (start_pos = nib_track;;find_sync(&start_pos, stop_pos))
    {
		if ((data_pos = start_pos + (cap_min-150)) >= stop_pos)
           		break; /* no cycle found */

		while (find_sync(&data_pos, stop_pos))
		{
			p1 = start_pos;
			cycle_pos = data_pos;
            
			for (p2 = cycle_pos; p2 < stop_pos;)
			{
				/* try to match all remaining syncs, too */
				if (memcmp(p1, p2, MATCH_LENGTH) != 0)
				{
					cycle_pos = NULL;
					break;
				}
				if (!find_sync(&p1, stop_pos)) break;
				if (!find_sync(&p2, stop_pos)) break;
			}

			if ( (cycle_pos != NULL) && (check_valid_data(data_pos, MATCH_LENGTH)) )
			{
				*cycle_start = start_pos;
				*cycle_stop = cycle_pos;
               	return (cycle_pos - start_pos);
            }
        }
    }

	/* we got nothing useful, return it all */
	*cycle_start = nib_track;
	*cycle_stop = nib_track + 0x2000;
	return (0x2000);
}

int find_nondos_track_cycle(BYTE **cycle_start, BYTE **cycle_stop, int cap_min, int cap_max)
{
    BYTE *nib_track;	/* start of nibbled track data */
    BYTE *start_pos;    /* start of periodic area */
    BYTE *cycle_pos;    /* start of cycle repetition */
    BYTE *stop_pos;     /* maximum position allowed for cycle */
    BYTE *p1, *p2;	/* local pointers for comparisons */

	nib_track = *cycle_start;
	start_pos = nib_track;
	stop_pos = nib_track + GCR_TRACK_LENGTH - MATCH_LENGTH;
	cycle_pos = NULL;

   	for(p1 = start_pos; p1 < stop_pos; p1 ++)
   	{
		/* now try to match it */
		for (p2 = p1 + (cap_min - 150); p2 < stop_pos; p2 ++)
		{
			/* try to match data */
			if (memcmp(p1, p2, MATCH_LENGTH) != 0)
				cycle_pos = NULL;
			else
				cycle_pos = p2;

			/* we found one! */
   		    if ( (cycle_pos != NULL) && (check_valid_data(cycle_pos, MATCH_LENGTH)) )
			{
				*cycle_start = p1;
				*cycle_stop = cycle_pos;
				return (cycle_pos - p1);
			}
   		}
	}

	/* we got nothing useful */
	*cycle_start = nib_track;
	*cycle_stop = nib_track + 0x2000;
	return (0x2000);
}

int check_valid_data(BYTE *data, int matchlen)
{
	/* makes assumptions on whether this is good data to match cycles */

	int i, redundbytes = 0;

	for(i = 0; i < matchlen; i++)
	{
		//printf("%.2x ",data[i]);
		if(data[i] == data[i+1]) redundbytes++;
	}

	//printf("%d\n",redundbytes);

	if(redundbytes < matchlen - 2) return 1;
	else return 0;
}



BYTE* find_sector0(BYTE *work_buffer, int tracklen, int *p_sectorlen)
{
    BYTE *pos;
    BYTE *buffer_end;
    BYTE *sync_last;

    pos = work_buffer;
    buffer_end = work_buffer + 2 * tracklen - 10;
    *p_sectorlen = 0;

    if (!find_sync(&pos, buffer_end)) return NULL;
    sync_last = pos;
    /* try to find sector 0 */
    while (pos < buffer_end)
    {
        if (!find_sync(&pos, buffer_end)) return NULL;
        if ((pos[0] == 0x52)
            && ((pos[1] & 0xc0) == 0x40)
            && ((pos[2] & 0x0f) == 0x05)
            && ((pos[3] & 0xfc) == 0x28))
        {
            *p_sectorlen = pos - sync_last;
            break;
        }
        sync_last = pos;
    }

    /* find last GCR byte before sync */
    do
    {
        pos -= 1;
        if (pos == work_buffer)
            pos += tracklen;
    } while (*pos == 0xff);

    /* move to first sync GCR byte */
    pos += 1;
    while (pos >= work_buffer+tracklen)
        pos -= tracklen;

    return pos;
}


BYTE* find_sector_gap(BYTE *work_buffer, int tracklen, int *p_sectorlen)
{
    int  gap, maxgap;
    BYTE *pos;
    BYTE *buffer_end;
    BYTE *sync_last;
    BYTE *sync_max;

    pos = work_buffer;
    buffer_end = work_buffer + 2 * tracklen - 10;
    *p_sectorlen = 0;

    if (!find_sync(&pos, buffer_end)) return NULL;
    sync_last = pos;
    maxgap = 0;

    /* try to find biggest (sector) gap */
    while (pos < buffer_end)
    {
        if (!find_sync(&pos, buffer_end)) break;
        gap = pos - sync_last;
        if (gap > maxgap)
        {
            maxgap = gap;
            sync_max = pos;
        }
        sync_last = pos;
    }
    *p_sectorlen = maxgap;

    if (maxgap == 0) return NULL; /* no gap found */

    /* find last GCR byte before sync */
    pos = sync_max;
    do
    {
        pos -= 1;
        if (pos == work_buffer)
            pos += tracklen;
    } while (*pos == 0xff);

    /* move to first sync GCR byte */
    pos += 1;
    while (pos >= work_buffer+tracklen)
        pos -= tracklen;

    return pos;
}


int check_formatted(BYTE *gcrdata)
{
	// checks if there is any reasonable section of formatted (GCR) data
    int i, run = 0;

    /* try to find longest good gcr run */
    for(i = 0; i < 0x2000; i++)
    {
    	if(is_bad_gcr(gcrdata, 0x2000, i))
	 		run = 0;
     	else
        	run++;

		if(run >= GCR_MIN_FORMATTED)
			return 1;

    }
	return 0;
}

int extract_GCR_track(BYTE *destination, BYTE *source, int *align, int force_align, int cap_min, int cap_max)
{
    /* try to extract one complete cycle of GCR data from an 8kB buffer.
       Align track to sector gap if possible, else align to sector 0,
       else copy cyclic loop from begin of source.
       If buffer has no good GCR, return tracklen = 0;
       [Input]  destination buffer, source buffer
       [Return] length of copied track fragment
    */

    BYTE work_buffer[0x4000]; /* working buffer */
    BYTE *cycle_start;  /* start position of cycle */
    BYTE *cycle_stop;   /* stop position of cycle  */
    DWORD track_len;

    BYTE *sector0_pos = NULL;	/* position of sector 0 */
    BYTE *sectorgap_pos = NULL;	/* position of sector gap */
    BYTE *longsync_pos = NULL;	/* position of longest sync run */
    BYTE *weakgap_pos = NULL;	/* position of weak bit run */
	BYTE *marker_pos = NULL;	/* generic marker used by protection handlers */

    int sector0_len;    /* length of gap before sector 0 */
    int sectorgap_len;  /* length of longest gap */

	/* if this track doesn't have enough formatted data, return blank */
	if(!check_formatted(source)) return 0;

    cycle_start = source;
	memset(work_buffer, 0, sizeof(work_buffer));
	memcpy(work_buffer, cycle_start, 0x2000);

	/* find cycle */
    find_track_cycle(&cycle_start, &cycle_stop, cap_min, cap_max);
    track_len = cycle_stop-cycle_start;

	/* second pass to find a cycle in custom track */
	if(track_len == 0x2000)
	{
		find_nondos_track_cycle(&cycle_start, &cycle_stop, cap_min, cap_max);
		track_len = cycle_stop-cycle_start;
	}

    /* copy twice the data to work buffer */
    memcpy(work_buffer, cycle_start, track_len);
    memcpy(work_buffer+track_len, cycle_start, track_len);

	// forced track alignments
	if(force_align != ALIGN_NONE)
	{
		if (force_align == ALIGN_VMAX)
		{
			*align = ALIGN_VMAX;
			marker_pos = align_vmax(work_buffer, track_len);
			memcpy(destination, marker_pos, track_len);
        	return(track_len);
		}

		if (force_align == ALIGN_VORPAL)
		{
			*align = ALIGN_VORPAL;
			marker_pos = align_vorpal(work_buffer, track_len);
			memcpy(destination, marker_pos, track_len);
        	return(track_len);
		}

		if (force_align == ALIGN_SEC0)
		{
			// for the occasional stickler
			*align = ALIGN_SEC0;
			marker_pos = find_sector0(work_buffer, track_len, &sector0_len);
			memcpy(destination, marker_pos, track_len);
        	return(track_len);
		}

		if (force_align == ALIGN_GAP)
		{
			// for the occasional stickler
			*align = ALIGN_GAP;
			marker_pos = find_sector_gap(work_buffer, track_len, &sectorgap_len);
			memcpy(destination, marker_pos, track_len);
        	return(track_len);
		}
	}

    /* try to guess original alignment */
    sector0_pos   = find_sector0(work_buffer, track_len, &sector0_len);
    sectorgap_pos = find_sector_gap(work_buffer, track_len, &sectorgap_len);

    if (sectorgap_len > (GCR_BLOCK_DATA_LEN + SIGNIFICANT_GAPLEN_DIFF))
    //if (sectorgap_len >= sector0_len + 0x40) /* Burstnibbler's calc */
    {
		*align = ALIGN_GAP;
        memcpy(destination, sectorgap_pos, track_len);
        return(track_len);
    }
    else if (sector0_len)
    {
		*align = ALIGN_SEC0;
        memcpy(destination, sector0_pos, track_len);
    	return(track_len);
    }
    else if (sectorgap_len)
    {
        memcpy(destination, sectorgap_pos, track_len);
        *align = ALIGN_GAP;
    	return(track_len);
    }

    /* try some custom last-ditch efforts, if activated */
	if(force_align == ALIGN_LONGSYNC)
	{
    	longsync_pos = find_long_sync(work_buffer, track_len);
 		if (longsync_pos)
		{
			memcpy(destination, longsync_pos, track_len);
			*align = ALIGN_LONGSYNC;
			return(track_len);
		}
	}

	if(force_align == ALIGN_WEAK)
	{
		weakgap_pos  = find_weak_gap(work_buffer, track_len);
    	if (weakgap_pos)
		{
			memcpy(destination, weakgap_pos, track_len);
			*align = ALIGN_WEAK;
			return(track_len);
		}
	}

	if(force_align == ALIGN_AUTOGAP)
	{
		marker_pos = auto_gap(work_buffer, track_len);
		memcpy(destination, marker_pos, track_len);
		return(track_len);
	}

	// we give up, just return everything
	memcpy(destination, work_buffer, track_len);
    *align = ALIGN_NONE;
    return(track_len);
}


int strip_runs(BYTE *buffer, int length, int minrun, BYTE target)
{
	 /* This strips exactly one byte at minrun from each
	  * eligible run when called.  It can be called repeatedly
	  * for a proportional reduction.
	  */

    int run = 0;
    int skipped = 0;

    BYTE *source;
    BYTE *end = buffer+length;

    for (source = buffer; source < end; source++)
    {
        if (*source == target)
        {
			if (run == minrun)
				skipped++;
   			else
                *buffer++ = target;

            run++;
        }
        else
        {
            run = 0;
            *buffer++ = *source;
        }
    }
    return skipped;
}


int reduce_runs(BYTE *buffer, int length, int length_max, int minrun, BYTE target)
{
    /* try to shorten repeated data until length <= length_max */
    int skipped;

    do
    {
		if (length <= length_max)
			return (length);

		skipped = strip_runs(buffer, length, minrun, target);
		length -= skipped;
	}
	while ((skipped > 0) && (length > length_max));

    return (length);
}


int check_sync_flags(BYTE *gcrdata, int density, int length)
{
	// this routine checks the track data and makes simple decisions
	// about the special cases of being all sync or having no sync

	int i;
	int syncs = 0;

	// empty tracks have no sync
	if(!length)
		return (density |= BM_NO_SYNC);

	// check manually for SYNCKILL
	for(i = 0; i < length; i ++)
	{
		if(gcrdata[i] == 0xff)
			syncs++;
	}

	if(syncs == length)
		return(density |= BM_FF_TRACK);

	// check manually for NOSYNC
	for(i = 0; i < length - 1; i ++)
	{
		if( ((gcrdata[i] & 0x03) == 0x03) && (gcrdata[i+1] == 0xff) )
			break;
	}

	if(i == (length - 1))
		return(density |= BM_NO_SYNC);

	return(density);
}


int compare_tracks(BYTE *track1, BYTE *track2, int length1, int length2, int same_disk,
				   char *outputstring)
{
	int match = 0;
	int j = 0;
	int k = 0;
	int sync_diff = 0;
	int presync_diff = 0;
	int gap_diff = 0;
	int weak_diff = 0;
	int size_diff = 0;
	char tmpstr[256];

	outputstring[0] = '\0';

	if ((length1 == length2) &&
 		(0 == memcmp(track1, track2, length1)))
			match=1;

    else if ((length1 > 0) && (length2 > 0))
    {
	    for (j = k = 0; (j < length2) && (k < length1); j++, k++)
		{
			if (track1[j] == track2[k])
				continue;

			// we ignore sync length differences
			if (track1[j] == 0xff)
			{
				sync_diff++;
				k--;
				continue;
			}

			if (track2[k] == 0xff)
			{
				sync_diff++;
				j--;
				continue;
			}

			// we ignore start of sync differences
			if((j < length1-1) && (k < length2-1))
			{
				if( ((track1[j] & 0x03) == 0x03) && (track1[j+1] == 0xff) )
				{
					presync_diff++;
					k--;
					continue;
				}

				if( ((track2[k] & 0x03) == 0x03) && (track2[k+1] == 0xff) )
				{
					presync_diff++;
					j--;
					continue;
				}
			}

			// we can sometimes ignore 0x55 and 0xaa variations due to framing
			// errors caused by stopped writing at the end-of-sector
			//if(!same_disk)
			//{
			//	if ( ((track1[j] & 0x05) == 0x05) || ((track2[k] & 0x05) == 0x05) ||
			//	     ((track1[j] & 0x50) == 0x50) || ((track2[k] & 0x50) == 0x50) ||
			//	     ((track1[j] & 0xa0) == 0xa0) || ((track2[k] & 0xa0) == 0xa0) ||
			//	     ((track1[j] & 0x0a) == 0x0a) || ((track2[k] & 0x0a) == 0x0a) )
			//	{
			//		gap_diff ++;
			//		j++; k++;
			//		continue;
			//	}
			//}

			// we ignore bad gcr bytes
			if ( (is_bad_gcr(track1, length1, j)) || (is_bad_gcr(track2, length2, k)) )
			{
				weak_diff++;
				j++; k++;
				continue;
			}

			// it just didn't work out. :)
			break;
		}

		if (!((j >= length1-1) && (k >= length2-1)))
			size_diff++;

		// we got to the end of one of them OK and not all sync/weak
        if ( ((j >= length1-1) || (k >= length2-1)) &&
        	 ((sync_diff < 0x100) && (weak_diff < 0x100)) )
			match=1;
	}

	if(sync_diff)
	{
		sprintf(tmpstr,"(sync:%d)",sync_diff);
		strcat(outputstring,tmpstr);
	}

	if(presync_diff)
	{
		sprintf(tmpstr,"(presync:%d)",presync_diff);
		strcat(outputstring,tmpstr);
	}

	if(gap_diff)
	{
		sprintf(tmpstr,"(gap:%d)",gap_diff);
		strcat(outputstring,tmpstr);
	}

	if(weak_diff)
	{
		sprintf(tmpstr,"(weak:%d)",weak_diff);
		strcat(outputstring,tmpstr);
	}

	if(size_diff)
	{
		sprintf(tmpstr,"(size:%d)",size_diff);
		strcat(outputstring,tmpstr);
	}

	return match;
}

int compare_sectors(BYTE *track1, BYTE *track2, int length1, int length2, int track,
	 				char *outputstring)
{
	int sec_match = 0;
	int numsecs = 0;
	int sector, error, error2, i;

	BYTE checksum1 = 0;
	BYTE checksum2 = 0;
	BYTE secbuf[260];
	BYTE secbuf2[260];
	BYTE id[3];
	char tmpstr[256];

	outputstring[0] = '\0';

	// ignore dead tracks
	if(	(!length1) || (!length2) || (length1 + length2 == 0x4000))
		return 0;

	// check for sector matches
	for (sector = 0; sector < sector_map_1541[(track/2)+1]; sector++)
	{
		numsecs ++;

	 	memset(secbuf, 0, sizeof(secbuf));
	 	memset(secbuf2, 0, sizeof(secbuf));

		error = convert_GCR_sector(track1, track1 + length1,
				secbuf, track/2, sector, id);

		error2 = convert_GCR_sector(track2, track2 + length2,
				secbuf2, track/2, sector, id);

		// compare data returned
		checksum1 = checksum2 = 0;
		for(i = 1; i <= 256; i++)
		{
			checksum1 ^= secbuf[i];
			checksum2 ^= secbuf2[i];
		}

		// either sector header missing
		if(error == HEADER_NOT_FOUND)
		{
			sprintf(tmpstr,"\nS%d: header missing from first track",sector);
			strcat(outputstring, tmpstr);
			continue;
		}
		if(error2 == HEADER_NOT_FOUND)
		{
			sprintf(tmpstr,"\nS%d: header missing from second track",sector);
			strcat(outputstring, tmpstr);
			continue;
		}

		// either sector data missing
		if(error == DATA_NOT_FOUND)
		{
			sprintf(tmpstr,"\nS%d: data missing from first track",sector);
			strcat(outputstring, tmpstr);
			continue;
		}
		if(error2 == DATA_NOT_FOUND)
		{
			sprintf(tmpstr,"\nS%d: data missing from second track",sector);
			strcat(outputstring, tmpstr);
			continue;
		}

		// either sector data missing
		if(error == SYNC_NOT_FOUND)
		{
			sprintf(tmpstr,"\nS%d: sync missing from first track",sector);
			strcat(outputstring, tmpstr);
			continue;
		}
		if(error2 == SYNC_NOT_FOUND)
		{
			sprintf(tmpstr,"\nS%d: sync missing from second track",sector);
			strcat(outputstring, tmpstr);
			continue;
		}

		// continue checking
		if((checksum1 == checksum2) && (error == error2))
		{
			//printf("S%d: sector data match\n",sector);
			sec_match++;
		}
		else
		{
			sprintf(tmpstr,"\nS%d: data/error mismatch (%.2x!=%.2x)",sector,checksum1,checksum2);
			strcat(outputstring, tmpstr);
		}
	}

	if(sec_match == sector_map_1541[(track/2)+1])
		return sec_match;
	else
		return 0;

}

int check_errors(BYTE *gcrdata, int length, int track, char *id, char *errorstring)
{
	// check for CBM DOS errors
	int errors = 0;
	int sector, errorcode;
	char tmpstr[16];
	BYTE secbuf[260];

	errorstring[0] = '\0';

	for (sector = 0; sector < sector_map_1541[(track/2)+1]; sector++)
	{
		errorcode = convert_GCR_sector(gcrdata, gcrdata+length, secbuf, (track/2), sector, id);

		if(errorcode != OK)
		{
			errors++;
			sprintf(tmpstr,"[E%dS%d]",errorcode,sector);
			strcat(errorstring,tmpstr);
		}
	}
	return errors;
}


int check_empty(BYTE *gcrdata, int length, int track, char *id, char *errorstring)
{
	// check for CBM DOS errors
	int i, empty = 0;
	int sector, errorcode;
	char tmpstr[16];
	char temp_errorstring[256];
	BYTE secbuf[260];

	errorstring[0] = '\0';
	temp_errorstring[0] = '\0';


	for(sector = 0; sector < sector_map_1541[(track/2)+1]; sector++)
	{
		errorcode = convert_GCR_sector(gcrdata, gcrdata+length, secbuf, (track/2), sector, id);

		if(errorcode == OK)
		{
			/* checks for empty (unused) sector */
			for (i = 2; i <= 256; i++)
			{
				if(secbuf[i] != 0x01)
				{
					//printf("%d:%0.2x ",i,secbuf[i]);
					break;
				}
			}

			if(i == 257)
			{
				sprintf(tmpstr,"%d,",sector);
		    	strcat(temp_errorstring,tmpstr);
				empty++;
			}
		}
	}

	if(empty)
		sprintf(errorstring,"[empty: %s]",temp_errorstring);

	return(empty);
}

int replace_bytes(BYTE *buffer, int length, BYTE srcbyte, BYTE dstbyte)
{
    /* replace 'srcbyte' by 'dstbyte'
     * returns total number of bytes replaced */

    int i;
    int replaced = 0;

    for (i = 0; i < length; i++)
    {
        if (buffer[i] == srcbyte)
        {
            buffer[i] = dstbyte;
            replaced++;
        }
    }
    return replaced;
}

int is_bad_gcr(BYTE *gcrdata, int length, int pos)
{
    // check if byte at pos contains a 000 bit combination
    unsigned int lastbyte, mask, data;

    lastbyte = (pos == 0) ? gcrdata[length-1] : gcrdata[pos-1];
    data = ((lastbyte & 0x03) << 8) | gcrdata[pos];

    for (mask = (7 << 7); mask >= 7; mask >>= 1)
    {
        if ((data & mask) == 0)
            break;
    }
    return (mask >= 7);
}

int check_bad_gcr(BYTE *gcrdata, int length, int fix)
{
    /* check and "correct" bad GCR bits:
     * substitute bad GCR bytes by 0x00 until next good GCR byte
     * when two in a row or row+1 occur (bad bad -or- bad good bad)
     *
	 * all known disks that use this for protection break out
	 * of it with $55, $AA, or $FF byte.
	 *
	 * fix_first, fix_last not used because while "correct", the real hardware
	 * is not this precise and it fails the protection checks sometimes.
	 */

    /* state machine definitions */
    enum ebadgcr { S_BADGCR_OK, S_BADGCR_ONCE_BAD, S_BADGCR_LOST };

    int i = 0;
    int lastpos;
    enum ebadgcr sbadgcr;
    int total = 0;
    int b_badgcr, n_badgcr;

    lastpos = length-1;

    if (is_bad_gcr(gcrdata, length, length-1))
        sbadgcr = S_BADGCR_ONCE_BAD;
    else
        sbadgcr = S_BADGCR_OK;

    for (i = 0; i < length - 1; i++)
    {
        b_badgcr = is_bad_gcr(gcrdata, length, i);
        n_badgcr = is_bad_gcr(gcrdata, length, i+1);

        switch (sbadgcr)
        {
            case S_BADGCR_OK:
                if (b_badgcr)
            	{
					total++;
            		//sbadgcr = S_BADGCR_ONCE_BAD;
					sbadgcr = S_BADGCR_LOST;
				}
                break;

            case S_BADGCR_ONCE_BAD:
                if ((b_badgcr) || (n_badgcr))
                {
                    sbadgcr = S_BADGCR_LOST;
                    //if(fix) fix_first_gcr(gcrdata, length, lastpos);
                	if(fix) gcrdata[lastpos] = 0x00;
                	total++;
                }
                else
                    sbadgcr = S_BADGCR_OK;
                break;

            case S_BADGCR_LOST:
				if ((b_badgcr) || (n_badgcr))
				{
					if(fix) gcrdata[lastpos] = 0x00;
				}
				else
				{
					sbadgcr = S_BADGCR_OK;
  					//if(fix) fix_last_gcr(gcrdata, length, lastpos);
					if(fix) gcrdata[lastpos] = 0x00;
				}
				total++;
				break;
        }
        lastpos = i;
    } /* end for */

    // clean up after last byte; lastpos = length - 1
    b_badgcr = is_bad_gcr(gcrdata, length, 0);
    n_badgcr = is_bad_gcr(gcrdata, length, 1);
    switch (sbadgcr)
    {
        case S_BADGCR_OK:
            break;

        case S_BADGCR_ONCE_BAD:
            if ((b_badgcr) || (n_badgcr))
            {
				//if(fix) fix_first_gcr(gcrdata, length, lastpos);
                if(fix) gcrdata[lastpos] = 0x00;
                total++;
            }
            break;

        case S_BADGCR_LOST:
            if ((b_badgcr) || (n_badgcr))
                gcrdata[lastpos] = 0x00;
			else
			{
				//if(fix) fix_last_gcr(gcrdata, length, lastpos);
				if(fix) gcrdata[lastpos] = 0x00;
			}
            total++;
            break;
    }
    return total;
}






