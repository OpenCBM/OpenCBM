/*
 * MNIB Read routines
 * Copyright 2001-2005 Markus Brenner <markus@brenner.de>
 * and Pete Rittwage <peter@rittwage.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef DJGPP
#include <dos.h>
#include <conio.h>
#include "cbm.h"
#include "mnib_rt.h"
#include "kernel.h"
typedef int CBM_FILE;
#else
# include <opencbm.h>
# include <arch.h>
# define delay(x) arch_usleep(x*1000)
/*
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#define delay(x)	sleep((x) / 1000)
*/
#endif // DJGPP

#include "gcr.h"
#include "mnib.h"

static BYTE diskid[3];

static BYTE paranoia_read_halftrack(CBM_FILE fd, int halftrack, BYTE * buffer);

BYTE
read_halftrack(CBM_FILE fd, int halftrack, BYTE * buffer)
{
	BYTE density;
    int defdens, i, timeout, printOutput;
	static int lasttrack = -1;

	timeout = 0;
	printOutput = (lasttrack == halftrack) ? 0 : 1;
	step_to_halftrack(fd, halftrack);

	lasttrack = halftrack;

	if (printOutput)
	{
		printf("%4.1f: (", (float) halftrack / 2);
		if(mode == MODE_READ_DISK) fprintf(fplog, "%4.1f: (", (float) halftrack / 2);
	}

	// we scan for the disk density and retry a few times if it's non-standard
	for (defdens = 3; halftrack >= bitrate_range[defdens]; defdens--);
	for (i = 0; i < 5; i++)
	{
		density = scan_track(fd, halftrack);
		if ((density & 3) == defdens ||
		  (density & BM_NO_SYNC) || (density & BM_FF_TRACK))
			break;
	}

	if (density & BM_FF_TRACK)
	{
		/* killer sync track */
		if (printOutput)
		{
			printf("KILLER:");
			if(mode == MODE_READ_DISK) fprintf(fplog, "KILLER:");

		}
	}
	else if (density & BM_NO_SYNC)
	{
		/* no sync found */
		if (printOutput)
		{
			printf("NOSYNC:");
			if(mode == MODE_READ_DISK) fprintf(fplog, "NOSYNC:");
		}
	}

	if (printOutput)
	{
		if ((density & 3) == defdens)
		{
			printf("%d) ", (density & 3));
			if(mode == MODE_READ_DISK) fprintf(fplog, "%d) ", (density & 3));
		}
		else
		{
			printf("%d!=%d) ", (density & 3), defdens);
			if(mode == MODE_READ_DISK) fprintf(fplog, "%d!=%d) ", (density & 3), defdens);
		}
	}

	// bail if we don't want to read killer tracks
	// some drives/disks timeout
	if ((density & BM_FF_TRACK) && (!read_killer))
	{
		memset(buffer, 0xff, 0x2000);
		return (density);
	}

	set_density(fd, density & 3);

	for (i = 0; i < 10; i++)
	{
		// read track

		if (density & BM_NO_SYNC)
			send_mnib_cmd(fd, FL_READWOSYNC);
		else
			send_mnib_cmd(fd, FL_READNORMAL);

		cbm_mnib_par_read(fd);
		timeout = cbm_mnib_read_track(fd, buffer, 0x2000, 0);

		// If we got a timeout, reset the port before retrying.
		if (!timeout)
		{
			putchar('?');
			fflush(stdout);
			cbm_mnib_par_read(fd);
			delay(500);
			printf("%c ", test_par_port(fd)? '+' : '-');
		}
		else
			break;
	}

	return (density);
}

static BYTE
paranoia_read_halftrack(CBM_FILE fd, int halftrack, BYTE * buffer)
{
	BYTE buffer1[0x2000];
	BYTE buffer2[0x2000];
	BYTE cbuffer1[0x2000];
	BYTE cbuffer2[0x2000];
	BYTE *cbufn, *cbufo, *bufn, *bufo;
	unsigned int lenn, densn, i, l, badgcr;
    BYTE denso;
	unsigned int errors, retries, short_read, long_read;
	int leno;
	char errorstring[0x1000], diffstr[80];

	badgcr = 0;
	errors = 0;
	retries = 1;
	short_read = 0;
	long_read = 0;
	bufn = buffer1;
	bufo = buffer2;
	cbufn = cbuffer1;
	cbufo = cbuffer2;

	diffstr[0] = '\0';
	errorstring[0] = '\0';

	if (!error_retries)
		error_retries = 1;

	// First pass at normal track read
	for (l = 0; l < error_retries; l++)
	{
		memset(bufo, 0, 0x2000);
		denso = read_halftrack(fd, halftrack, bufo);

		// Find track cycle and length
		memset(cbufo, 0, 0x2000);
		leno = extract_GCR_track(cbufo, bufo, &align, force_align,
		  capacity_min[denso & 3], capacity_max[denso & 3]);

		// if we have a killer track and are ignoring them, exit processing
		if((denso & BM_FF_TRACK) && (!read_killer))
			break;

		// If we get nothing (except t18), we are on an empty
		// track (unformatted)
		if (leno == 0 && halftrack != 36)
		{
			printf("- no data\n");
			fprintf(fplog, "%s (%d)\n", errorstring, leno);
			memcpy(buffer, bufo, 0x2000);
			return (denso);
		}

		// if we get less than what a track holds,
		// try again, probably bad read or a weak match
		if (leno < capacity_min[denso & 3] - 155)
		{
			printf("<! ");
			fprintf(fplog, "[%d<%d!] ", leno,
			  capacity_min[denso & 3] - 155);
			l--;
			if (short_read++ > error_retries)
				break;
			continue;
		}

		// if we get more than capacity
		// try again to make sure it's intentional
		if (leno > capacity_max[denso & 3] + 255)
		{
			printf("!> ");
			fprintf(fplog, "[%d>%d!] ", leno,
			  capacity_max[denso & 3] + 255);
			l--;
			if (long_read++ > error_retries)
				break;
			continue;
		}

		printf("%d ", leno);
		fprintf(fplog, "%d ", leno);

		// check for CBM DOS errors
		errors = check_errors(cbufo, leno, halftrack, diskid, errorstring);
		fprintf(fplog, "%s", errorstring);

		// special case for directory track
		if (halftrack == 36 && errors > 1)
			continue;

		// if we got all good sectors,
		// or all bad sectors (protection)
		// we don't need to retry any more
		if (errors == 0 || errors > 15)
			break;
	}

	// Give some indication of disk errors, unless it's all errors
	errors = check_errors(cbufo, leno, halftrack, diskid, errorstring);

	// If there are a lot of errors, the track probably doesn't contain
	// any DOS sectors (protection)
	if (errors >= 15)
	{
		printf("[PROT] ");
		fprintf(fplog, "%s ", errorstring);
	}
	else if (errors > 0)
	{
		// probably old-style intentional error(s)
		printf("%s ", errorstring);
		fprintf(fplog, "%s ", errorstring);
	}
	else
	{
		// this is all good CBM DOS-style sectors
		printf("[DOS] ");
		fprintf(fplog, "[DOS] ");
	}

	// Try to verify our read
	printf("- ");

	// Fix bad GCR in track for compare
	badgcr = check_bad_gcr(cbufo, leno, 1);

	// Don't bother to compare unformatted or bad data
	if (leno == 0x2000)
		retries = 0;

	// normal data, verify
	for (i = 0; i < retries; i++)
	{
		memset(bufn, 0, 0x2000);
		densn = read_halftrack(fd, halftrack, bufn);

		memset(cbufn, 0, 0x2000);
		lenn = extract_GCR_track(cbufn, bufn, &align, force_align,
		  capacity_min[densn & 3], capacity_max[densn & 3]);

		printf("%d ", lenn);
		fprintf(fplog, "%d ", lenn);

		// fix bad GCR in track for compare
		badgcr = check_bad_gcr(cbufn, lenn, 1);

		// compare raw gcr data, unreliable
		if (compare_tracks(cbufo, cbufn, leno, lenn, 1, errorstring))
		{
			printf("[RAW MATCH] ");
			fprintf(fplog, "[RAW MATCH] ");
			break;
		}
		else
			fprintf(fplog, "%s", errorstring);

		// compare sector data
		if (compare_sectors(cbufo, cbufn, leno, lenn, diskid, diskid,
		  halftrack, errorstring))
		{
			printf("[SEC MATCH] ");
			fprintf(fplog, "[SEC MATCH] ");
			break;
		}
		else
			fprintf(fplog, "%s", errorstring);
	}

	if (badgcr)
	{
		printf("(weak:%d)", badgcr);
		fprintf(fplog, "(weak:%d) ", badgcr);
	}
	printf("\n");
	fprintf(fplog, "%s (%d)\n", errorstring, leno);
	memcpy(buffer, bufo, 0x2000);
	return denso;
}

void
read_nib(CBM_FILE fd, FILE * fpout, char *track_header)
{
    BYTE track, density;
	int header_entry, i;
	BYTE buffer[0x2000];
	//unsigned int track_len;
	//BYTE *cycle_start;  /* start position of cycle */
	//BYTE *cycle_stop;   /* stop position of cycle  */

	memset(diskid, 0, sizeof(diskid));

	/* read track 18 */
	density = read_halftrack(fd, 18 * 2, buffer);

	/* determine disk id for e11 checks */
	if (!extract_id(buffer, diskid))
		fprintf(stderr, "[Cannot find directory sector!]\n");
	else
	{
		printf("ID: %s\n", diskid);
		fprintf(fplog, "ID: %s\n", diskid);
	}

#if 0
	/* determine speed of drive that wrote this disk */
	cycle_start = buffer;
	find_track_cycle(&cycle_start, &cycle_stop, capacity_min[density&3],
	  capacity_max[density&3]);
	track_len = cycle_stop-cycle_start;
	printf(" (mastering drive averaged %.1f RPM)\n", (float)2143190/track_len);
	fprintf(fplog," (mastering drive averaged %.1f RPM)\n",
	  (float)2143190/track_len);
#endif // 0

	header_entry = 0;
	for (track = start_track; track <= end_track; track += track_inc)
	{
		memset(buffer, 0, sizeof(buffer));

		// density = read_halftrack(fd, track, buffer);
		density = paranoia_read_halftrack(fd, track, buffer);
		track_header[header_entry * 2] = track;
		track_header[header_entry * 2 + 1] = density;
		header_entry++;

		/* process and save track to disk */
		for (i = 0; i < 0x2000; i++)
			fputc(buffer[i], fpout);
	}
	step_to_halftrack(fd, 18 * 2);
}

int
read_d64(CBM_FILE fd, FILE * fpout)
{
	int density, track, sector;
	int csec;		/* compare sector variable */
	int blockindex, save_errorinfo, save_40_errors, save_40_tracks;
	int retry;
	BYTE buffer[0x2100];
	BYTE *cycle_start;	/* start position of cycle     */
	BYTE *cycle_stop;	/* stop  position of cycle + 1 */
	BYTE id[3];
	BYTE rawdata[260];
	BYTE sectordata[16 * 21 * 260];
	BYTE d64data[MAXBLOCKSONDISK * 256];
	BYTE *d64ptr;
	BYTE errorinfo[MAXBLOCKSONDISK], errorcode;
	int sector_count[21];	/* number of different sector data read */
	int sector_occur[21][16];	/* how many times was this sector data read? */
	BYTE sector_error[21][16];	/* type of error on this sector data */
	int sector_use[21];	/* best data for this sector so far */
	int sector_max[21];	/* # of times the best sector data has occured */
	int goodtrack, goodsector;
	int any_sectors;	/* any valid sectors on track at all? */
	int blocks_to_save;

	blockindex = 0;
	save_errorinfo = 0;
	save_40_errors = 0;
	save_40_tracks = 0;

	density = read_halftrack(fd, 18 * 2, buffer);
	printf("\n");
	if (!extract_id(buffer, id))
	{
		fprintf(stderr, "Cannot find directory sector.\n");
		return (0);
	}

	d64ptr = d64data;
	for (track = 1; track <= 40; track += 1)
	{
		/* no sector data read in yet */
		for (sector = 0; sector < 21; sector++)
			sector_count[sector] = 0;

		any_sectors = 0;
		for (retry = 0; retry < 16; retry++)
		{
			goodtrack = 1;
			density = read_halftrack(fd, 2 * track, buffer);

			cycle_start = buffer;
			find_track_cycle(&cycle_start, &cycle_stop,
			  capacity_min[density & 3],
			  capacity_max[density & 3]);

			for (sector = 0; sector < sector_map_1541[track]; sector++)
			{
				sector_max[sector] = 0;
				/* convert sector to free sector buffer */
				errorcode =
				  convert_GCR_sector(cycle_start, cycle_stop,
				  rawdata, track, sector, id);

				if (errorcode == SECTOR_OK)
					any_sectors = 1;

				/* check, if identical sector has been read before */
				for (csec = 0; csec < sector_count[sector]; csec++)
				{
					if (memcmp(sectordata + (21 * csec + sector) * 260,
					  rawdata, 260) == 0 &&
					  sector_error[sector][csec] == errorcode)
					{
						sector_occur[sector][csec] += 1;
						break;
					}
				}
				if (csec == sector_count[sector])
				{
					/* sectordaten sind neu, kopieren, zaehler erhoehen */
					memcpy(sectordata + (21 * csec + sector) * 260, rawdata,
					  260);
					sector_occur[sector][csec] = 1;
					sector_error[sector][csec] = errorcode;
					sector_count[sector] += 1;
				}

				goodsector = 0;
				for (csec = 0; csec < sector_count[sector]; csec++)
				{
					if (sector_occur[sector][csec] -
					  ((sector_error[sector][csec] == SECTOR_OK) ? 0 : 8) >
					  sector_max[sector])
					{
						sector_use[sector] = csec;
						sector_max[sector] = csec;
					}

					if (sector_occur[sector][csec] -
					  ((sector_error[sector][csec] == SECTOR_OK) ? 0 : 8) >
					  (retry / 2) + 1)
					{
						goodsector = 1;
					}
				}
				if (goodsector == 0)
					goodtrack = 0;
			}

			if (goodtrack == 1)
				break;
			if (retry == 1 && any_sectors == 0)
				break;
		}

		/* keep the best data for each sector */
		for (sector = 0; sector < sector_map_1541[track]; sector++)
		{
			printf("%d", sector);

			memcpy(d64ptr,
			  sectordata + 1 + (21 * sector_use[sector] + sector) * 260, 256);
			d64ptr += 256;

			errorcode = sector_error[sector][sector_use[sector]];
			errorinfo[blockindex] = errorcode;

			if (errorcode != SECTOR_OK)
			{
				if (track <= 35)
					save_errorinfo = 1;
				else
					save_40_errors = 1;
			}
			else if (track > 35)
			{
				save_40_tracks = 1;
			}

			/* screen information */
			if (errorcode == SECTOR_OK)
				printf(" ");
			else
				printf("%d", errorcode);
			blockindex++;
		}
		printf("\n");

	}

	blocks_to_save = (save_40_tracks) ? MAXBLOCKSONDISK : BLOCKSONDISK;

	if (fwrite((char *) d64data, blocks_to_save * 256, 1, fpout) != 1)
	{
		fprintf(stderr, "Cannot write d64 data.\n");
		return (0);
	}

	if (save_errorinfo == 1)
	{
		if (fwrite((char *) errorinfo, blocks_to_save, 1, fpout) != 1)
		{
			fprintf(stderr, "Cannot write sector data.\n");
			return (0);
		}
	}
	return (1);
}
