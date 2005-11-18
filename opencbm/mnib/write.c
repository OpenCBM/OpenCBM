/*
 * MNIB Write routines
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
#include "kernel.h"
#include "mnib_rt.h"
typedef int CBM_FILE;
#else
# include <opencbm.h>
# include <arch.h>
# define msleep(x) arch_usleep(x*1000)
/*
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#define msleep(x)	sleep((x) / 1000)
*/
#endif // DJGPP

#include "gcr.h"
#include "mnib.h"

static BYTE diskbuf[84 * 0x2000];
static int track_length[84], track_density[84];

static void write_halftrack(int halftrack, int density, int length,
  BYTE * gcrdata);
static void master_disk(CBM_FILE fd);
static void unformat_track(CBM_FILE fd, int track);

static void
write_halftrack(int halftrack, int density, int length, BYTE * gcrdata)
{
	int defdens, badgcr, orglen;

	badgcr = 0;

	// double-check our sync-flag assumptions
	density = check_sync_flags(gcrdata, (density & 3), length);

	// calculate standard density for comparison
	for (defdens = 3; halftrack >= bitrate_range[defdens]; defdens--);

	// user display
	printf("\n%4.1f: (density=", (float) halftrack / 2);

	if (density & BM_NO_SYNC)
		printf("NOSYNC:");

	else if (density & BM_FF_TRACK)
	{
		// reset sync killer track length to 0
		length = 0;
		printf("KILLER:");
	}

	printf("%d", density & 3);
	if (density != defdens) printf("!");
	printf(") [");

	if (length > 0)
	{
		if (fix_gcr)
		{
			badgcr = check_bad_gcr(gcrdata, length, 1);
			if (badgcr > 0)
				printf("weak:%d ", badgcr);
		}

		// If our track contains sync, we reduce to a minimum of 16
		// (only 10 are required, technically)
		orglen = length;

		if (length > capacity[density & 3] && !(density & BM_NO_SYNC) &&
		  reduce_syncs)
		{
			// then try to reduce sync within the track
			if (length > capacity[density & 3])
				length = reduce_runs(gcrdata, length,
				  capacity[density & 3], 2, 0xff);
			if (length < orglen)
				printf("rsync:%d ", orglen - length);
		}

		// We could reduce gap bytes ($55 and variants) here too,
		orglen = length;
		if (length > capacity[density & 3] && reduce_gaps)
		{
			// XXX We can reduce by 0xAA here also
			length = reduce_runs(gcrdata, length, capacity[density & 3],
			  2, 0x55);
			if (length < orglen)
				printf("rgaps:%d ", orglen - length);
		}

		// reduce weak bit runs (experimental)
		orglen = length;
		if (length > capacity[density & 3] && badgcr > 0 && reduce_weak)
		{
			length = reduce_runs(gcrdata, length, capacity[density & 3],
			  2, 0x00);
			if (length < orglen)
				printf("rweak:%d ", orglen - length);
		}

		// still not small enough, we have to truncate the end
		orglen = length;
		if (length > capacity[density & 3])
		{
			length = capacity[density & 3];
			if (length < orglen)
				printf("trunc:%d ", orglen - length);
			else
				printf("\nHad to truncate track %d by %d bytes.",
				  halftrack / 2, orglen - length);
		}
	}

	// if track is empty (unformatted) overfill with '0' bytes to simulate
	if (!length && (density & BM_NO_SYNC))
	{
		memset(gcrdata, 0x00, 0x2000);
		length = 0x2000;
	}

	// if it's a killer track, fill with sync
	if (!length && (density & BM_FF_TRACK))
	{
		memset(gcrdata, 0xff, 0x2000);
		length = 0x2000;
	}

	// replace 0x00 bytes by 0x01, as 0x00 indicates end of track
	replace_bytes(gcrdata, length, 0x00, 0x01);

	// write processed track to disk image
	track_length[halftrack] = length;
	track_density[halftrack] = density;
	memcpy(diskbuf + (halftrack * 0x2000), gcrdata, length);

	printf("] (%d) ", length);

	// print out track alignment, as determined
	if (imagetype == IMAGE_NIB)
	{
		switch (align)
		{
		case ALIGN_NONE:
			printf("(none) ");
			break;
		case ALIGN_SEC0:
			printf("(sec0) ");
			break;
		case ALIGN_GAP:
			printf("(gap) ");
			break;
		case ALIGN_LONGSYNC:
			printf("(sync) ");
			break;
		case ALIGN_WEAK:
			printf("(weak) ");
			break;
		case ALIGN_VMAX:
			printf("(v-max) ");
			break;
		case ALIGN_AUTOGAP:
			printf("(auto) ");
			break;
		}
	}
}

static void
master_disk(CBM_FILE fd)
{
	int track, i, length, density;
	int align_offset;	// how many bytes we are "late"
	BYTE rawtrack[0x2400];
	BYTE cmptrack[0x2000], cmpraw[0x2000];
	BYTE gapbyte = 0x55;
	char errorstring[0x1000];
	BYTE diskid[3];

	memset(diskid, 0, sizeof(diskid));
	extract_id(diskbuf + ((18 * 2) * 0x2000), diskid);

	printf("\n-\nBurst Writing...\n");

	if (!verify)
		printf("[");

	for (track = start_track; track <= end_track; track += track_inc)
	{
		// skip empty tracks (raw mode tests)
		if (track_length[track] == 0 && mode == MODE_WRITE_RAW)
		{
			printf(".");
			continue;
		}

		align_offset = 0x100 + (0x100 * (track_density[track] & 3));

		// add filler so track is completely erased
		// also used for timing from track to track
		memset(rawtrack, gapbyte, sizeof(rawtrack));

		// append track data after alignment filler
		memcpy(rawtrack + align_offset, diskbuf + (track * 0x2000),
		  track_length[track]);

		// step to destination track and set density
		step_to_halftrack(fd, track);
		set_density(fd, track_density[track] & 3);

		// send track
		for (i = 0; i < 10; i++)
		{
			send_mnib_cmd(fd, FL_WRITENOSYNC);
			cbm_mnib_par_write(fd, 0);

			if (!cbm_mnib_write_track(fd, rawtrack, align_offset +
			  track_length[track], 0))
			{
				putchar('?');
				fflush(stdin);
				cbm_mnib_par_read(fd);
				msleep(500);
				printf("%c ", test_par_port(fd)? '+' : '-');
			}
			else
				break;
		}

		if (!verify)
			printf("%d", track_density[track] & 3);
		else
		{
			density = read_halftrack(fd, track, cmptrack);

			length = extract_GCR_track(cmpraw, cmptrack, &align, force_align,
			  capacity_min[density & 3], capacity_max[density & 3]);

			printf("(%d) ", length);

			// compare raw gcr data, unreliable
			if (compare_tracks(diskbuf + (track * 0x2000), cmpraw,
			  track_length[track], length, 1, errorstring))
				printf("[RAW VERIFY] ");

			// compare sector data
			else if (compare_sectors(diskbuf + (track * 0x2000), cmpraw,
			  track_length[track], length, diskid, diskid, track,
			  errorstring))
				printf("[SEC VERIFY] ");

			else
				printf("[NO VERIFY]");

			printf("\n");
		}
	}
	if (!verify)
		printf("]\n");
}

void
write_raw(CBM_FILE fd)
{
	int track, density;
	BYTE trackbuf[0x2000];
	char testfilename[16];
	FILE *trkin;
	int length;

	motor_on(fd);
	if (auto_density_adjust)
		adjust_target(fd);

	for (track = start_track; track <= end_track; track += track_inc)
	{
		// read in raw track at density (in filename)
		for (density = 0; density <= 3; density++)
		{
			sprintf(testfilename, "raw/tr%dd%d", track / 2,
			  density);
			if ((trkin = fopen(testfilename, "rb")))
				break;
		}

		if (trkin)
		{
			memset(trackbuf, 0x55, 0x2000);
			fseek(trkin, 0, SEEK_END);
			length = ftell(trkin);
			rewind(trkin);
			fread(trackbuf, length, 1, trkin);
			fclose(trkin);

			write_halftrack(track, density, length, trackbuf);
		}
	}
	master_disk(fd);
	step_to_halftrack(fd, 18 * 2);
}

void
unformat_disk(CBM_FILE fd)
{
	int track;

	motor_on(fd);
	set_density(fd, 2);

	printf("\nUnformatting [");

	for (track = start_track; track <= end_track; track += track_inc)
	{
		printf("X");
		unformat_track(fd, track);
	}
	printf("]\n");
}

void
unformat_track(CBM_FILE fd, int track)
{
	int i;
	BYTE buffer[0x2000];

	// step head
	step_to_halftrack(fd, track);

	// write 0x01 $2000 times
	memset(buffer, 0x01, 0x2000);

	for (i = 0; i < 10; i++)
	{
		send_mnib_cmd(fd, FL_WRITENOSYNC);
		cbm_mnib_par_write(fd, 0);

		if (!cbm_mnib_write_track(fd, buffer, 0x2000, 0))
		{
			putchar('?');
			fflush(stdout);
			cbm_mnib_par_read(fd);
			msleep(500);
			printf("%c ", test_par_port(fd)? '+' : '-');
		}
		else
			break;
	}
}

void
parse_disk(CBM_FILE fd, FILE * fpin, char *track_header)
{
	int track, density, dens_pointer, header_entry;
	BYTE buffer[0x2000];
	BYTE gcrdata[0x2000];
	int length, g64tracks, g64maxtrack;

	// clear our buffers
	memset(diskbuf, 0, sizeof(diskbuf));
	memset(track_length, 0, sizeof(track_length));
	memset(track_density, 0, sizeof(track_density));

	if (imagetype == IMAGE_NIB)
	{
		header_entry = 0;
		for (track = start_track; track <= end_track; track += track_inc)
		{
			// clear buffers
			memset(buffer, 0, 0x2000);
			memset(gcrdata, 0, 0x2000);

			// if this image has halftracks, skip them
			if (track_header[header_entry * 2] != track)
			{
				printf("\nskipping halftrack");
				fread(buffer, 1, 0x2000, fpin);
				memset(buffer, 0, 0x2000);
				header_entry++;
			}

			/* get density from header or use default */
			density = (track_header[header_entry * 2 + 1]);
			header_entry++;

			// get track from file
			align = ALIGN_NONE;	// reset track alignment feedback
			fread(buffer, 1, 0x2000, fpin);
			length = extract_GCR_track(gcrdata, buffer, &align,
			  force_align, capacity_min[density & 3],
			  capacity_max[density & 3]);

			// write track
			write_halftrack(track, density, length, gcrdata);
		}
	}
	else if (imagetype == IMAGE_G64)
	{
		g64tracks = track_header[0];
		g64maxtrack = (BYTE) track_header[2] << 8 | (BYTE) track_header[1];
		printf("G64: %d tracks, %d bytes each", g64tracks,
		  g64maxtrack);

		// reduce tracks if > 40, or else causes alignment problems (PJR)
		if (g64tracks > 80)
			g64tracks = 80;

		dens_pointer = 0;
		for (track = start_track; track <= g64tracks; track += track_inc)
		{
			// clear buffers
			memset(buffer, 0, 0x2000);
			memset(gcrdata, 0, 0x2000);

			/*
			 * There shouldn't be any G64's with halftracks out there yet? :)
			 * we couldn't write them anyway
			 */

			/* get density from header or use default */
			density = track_header[0x153 + dens_pointer];
			dens_pointer += 8;

			/* get length */
			buffer[0] = char_fgetc(fpin);
			buffer[1] = char_fgetc(fpin);
			length = buffer[1] << 8 | buffer[0];

			/* get track from file */
			fread(gcrdata, 1, g64maxtrack, fpin);

			// write track
			write_halftrack(track, density, length, gcrdata);
		}
	}
	master_disk(fd);
	step_to_halftrack(fd, 18 * 2);
}

int
write_d64(CBM_FILE fd, FILE * fpin)
{
	int track, sector, sector_ref, density;
	BYTE buffer[256], gcrdata[0x2000];
	BYTE errorinfo[MAXBLOCKSONDISK];
	BYTE id[3] = { 0, 0, 0 };
	int length, error, d64size, last_track;
	char errorstring[0x1000], tmpstr[8];

	/* here we get to rebuild tracks from scratch */
	memset(errorinfo, SECTOR_OK, MAXBLOCKSONDISK);

	/* determine d64 image size */
	fseek(fpin, 0, SEEK_END);
	d64size = ftell(fpin);
	switch (d64size)
	{
	case (BLOCKSONDISK * 257):		/* 35 track image with errorinfo */
		fseek(fpin, BLOCKSONDISK * 256, SEEK_SET);
		fread(errorinfo, sizeof(BYTE), BLOCKSONDISK, fpin);
		/* FALLTHROUGH */
	case (BLOCKSONDISK * 256):		/* 35 track image w/o errorinfo */
		last_track = 35;
		break;

	case (MAXBLOCKSONDISK * 257):	/* 40 track image with errorinfo */
		fseek(fpin, MAXBLOCKSONDISK * 256, SEEK_SET);
		fread(errorinfo, sizeof(BYTE), MAXBLOCKSONDISK, fpin);
		/* FALLTHROUGH */
	case (MAXBLOCKSONDISK * 256):	/* 40 track image w/o errorinfo */
		last_track = 40;
		break;

	default:
		rewind(fpin);
		fprintf(stderr, "Bad d64 image size.\n");
		return 0;
	}

	// determine disk id from track 18 (offsets $165A2, $165A3)
	fseek(fpin, 0x165a2, SEEK_SET);
	fread(id, sizeof(BYTE), 2, fpin);
	printf("\ndisk id: %s", id);

	rewind(fpin);

	sector_ref = 0;
	for (track = 1; track <= last_track; track++)
	{
		// clear buffers
		memset(gcrdata, 0x55, 0x2000);
		errorstring[0] = '\0';

		for (sector = 0; sector < sector_map_1541[track]; sector++)
		{
			// get error and increase reference pointer in errorinfo
			error = errorinfo[sector_ref++];
			if (error != SECTOR_OK)
			{
				sprintf(tmpstr, " E%dS%d", error, sector);
				strcat(errorstring, tmpstr);
			}

			// read sector from file
			fread(buffer, sizeof(BYTE), 256, fpin);

			// convert to gcr
			convert_sector_to_GCR(buffer,
			  gcrdata + (sector * 361), track, sector, id, error);
		}

		// calculate track length
		length = sector_map_1541[track] * 361;

		// use default densities for D64
		for (density = 3; track * 2 >= bitrate_range[density]; density--);

		// write track
		write_halftrack(track * 2, density, length, gcrdata);
		printf("%s", errorstring);
	}

	// "unformat" last 5 tracks on 35 track disk
	if (last_track == 35)
	{
		for (track = 36 * 2; track <= end_track; track += 2)
		{
			track_length[track] = 0x2000;
			track_density[track] = 2;
			memset(diskbuf + (track * 0x2000), 0x01, 0x2000);
		}
	}

	master_disk(fd);
	step_to_halftrack(fd, 18 * 2);
	return (1);
}
