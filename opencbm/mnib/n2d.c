/*
    n2d.c - Converts mnib nibbler data to D64 image

    (C) 2000-05 Markus Brenner <markus@brenner.de>

    Based on code by Andreas Boose <boose@unixserv.rz.fh-hannover.de>

    V 0.19   fixed usage message, only use 35 tracks for now
    V 0.20   improved disk error detection
    V 0.21   split program in n2d.c and gcr.h/gcr.c
    V 0.22   added halftrack-image support
    V 0.23   improved/fixed conversion
    V 0.24   error conversion improved (SYNC NOT FOUND)
    V 0.25   improved track cycle detection
    V 0.35   synced versioning with other mnib tools
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include "mnibarch.h"
#include "gcr.h"
#include "version.h"

static void
usage(void)
{
	fprintf(stderr, "Usage: n2d <option> nibimage [d64image]\n");
	fprintf(stderr, "-p: Patch cosmetic disk id to real disk id\n");
	exit(-1);
}

int ARCH_MAINDECL
main(int argc, char **argv)
{
	FILE *fp_nib, *fp_d64;
	char inname[1024], outname[1024], *dotpos;;
	BYTE id[3], cosmetic_id[3];;
	int track, sector;
	BYTE gcr_track[GCR_TRACK_LENGTH], rawdata[260];
	BYTE errorinfo[MAXBLOCKSONDISK], errorcode;
	int save_errorinfo, blockindex;
	BYTE nib_header[0x100];
	int header_offset;
	BYTE *cycle_start;	/* start position of cycle    */
	BYTE *cycle_stop;	/* stop  position of cycle +1 */
	int patch_diskid = 0;

	fprintf(stdout,
	  "\nn2d - converts a NIB type disk dump into a standard D64 disk image.\n"
	  "(C) 2000-05 Markus Brenner and Pete Rittwage.\n" "Version " VERSION "\n\n");

	while (--argc && (*(++argv)[0] == '-'))
	{
		switch (tolower((*argv)[1]))
		{
		case 'p':
			printf("ARG: Patch cosmetic disk id to real disk ID\n");
			patch_diskid = 1;
			break;

		default:
			usage();
			break;
		}
	}

	if (argc == 1)
	{
		strcpy(inname, argv[0]);
		strcpy(outname, inname);
	}
	else if (argc == 2)
	{
		strcpy(inname, argv[0]);
		strcpy(outname, argv[1]);
	}
	else
		usage();

	dotpos = strrchr(outname, '.');
	if (dotpos != NULL)
		*dotpos = '\0';
	strcat(outname, ".d64");

	fp_nib = fp_d64 = NULL;

	fp_nib = fopen(inname, "rb");
	if (fp_nib == NULL)
	{
		fprintf(stderr, "Cannot open NIB image %s.\n", inname);
		goto fail;
	}

	if (fread(nib_header, sizeof(BYTE), 0x0100, fp_nib) < 0x0100)
	{
		fprintf(stderr, "Cannot read header from mnib image.\n");
		goto fail;
	}

	if (memcmp(nib_header, "MNIB-1541-RAW", 13) != 0)
	{
		fprintf(stderr, "input file %s isn't an mnib data file!\n", inname);
		goto fail;
	}

	fp_d64 = fopen(outname, "wb");
	if (fp_d64 == NULL)
	{
		fprintf(stderr, "Cannot open D64 image %s.\n", outname);
		goto fail;
	}

	/* figure out the disk ID from Track 18, Sector 0 */
	id[0] = id[1] = id[2] = '\0';
	if (nib_header[0x10 + 17 * 2] == 18 * 2)
	{
		/* normal nibble file */
		fseek(fp_nib, 17 * GCR_TRACK_LENGTH + 0x100, SEEK_SET);
	}
	else
	{
		/* halftrack nibble file */
		fseek(fp_nib, 2 * 17 * GCR_TRACK_LENGTH + 0x100, SEEK_SET);
	}

	if (fread(gcr_track, sizeof(BYTE), GCR_TRACK_LENGTH, fp_nib) <
	  GCR_TRACK_LENGTH)
	{
		fprintf(stderr, "Cannot read track from image.\n");
		goto fail;
	}
	if (!extract_id(gcr_track, id))
	{
		fprintf(stderr, "Cannot find directory sector.\n");
		goto fail;
	}
	//printf("ID: %2x %2x\n", id[0], id[1]);
	printf("\ndisk id: %s\n", id);

	// collect and print "cosmetic" disk id for comparison
	memset(cosmetic_id, 0, 3);
	extract_cosmetic_id(gcr_track, cosmetic_id);
	printf("cosmetic disk id: %s\n", cosmetic_id);

	/* reset file pointer to first track */
	fseek(fp_nib, 0x100, SEEK_SET);

	blockindex = 0;
	save_errorinfo = 0;
	header_offset = 0x10;	/* number of first nibble-track in nib image */
	for (track = 1; track <= 35; track++)
	{
		/* Skip halftracks if present in image */
		if (nib_header[header_offset] < (track * 2))
		{
			fseek(fp_nib, GCR_TRACK_LENGTH, SEEK_CUR);
			header_offset += 2;
		}
		header_offset += 2;

		/* read in one track */
		if (fread(gcr_track, sizeof(BYTE), GCR_TRACK_LENGTH, fp_nib) <
		  GCR_TRACK_LENGTH)
		{
			fprintf(stderr, "Cannot read track from G64 image.\n");
			goto fail;
		}

		printf("\nTrack: %2d - Sector: ", track);

		cycle_start = gcr_track;
		find_track_cycle(&cycle_start, &cycle_stop,
		  capacity_min[speed_map_1541[track]],
		  capacity_max[speed_map_1541[track]]);

		/* FIXME: maybe improve for cycle == NULL */

		for (sector = 0; sector < sector_map_1541[track]; sector++)
		{
			printf("%d", sector);

			errorcode = convert_GCR_sector(cycle_start, cycle_stop, rawdata,
			  track, sector, id);

			errorinfo[blockindex] = errorcode;	/* OK by default */
			if (errorcode != SECTOR_OK)
				save_errorinfo = 1;

			if (errorcode == SECTOR_OK)
				printf(" ");
			else
				printf("%d", errorcode);



			if (fwrite((char *) rawdata + 1, 256, 1, fp_d64) != 1)
			{
				fprintf(stderr,
				  "Cannot write sector data.\n");
				goto fail;
			}

			blockindex++;
		}
	}

	/* Missing: Track 36-40 detection */

	if (save_errorinfo)
	{
		if (fwrite((char *) errorinfo, BLOCKSONDISK, 1, fp_d64) != 1)
		{
			fprintf(stderr, "Cannot write error information.\n");
			goto fail;
		}
	}

	// fix cosmetic id to real disk id
	if((strcmp((char *)id, (char *)cosmetic_id)) && (patch_diskid))
	{
		printf("\n\nPatched cosmetic disk id to real disk id.\n");
		fseek(fp_d64, 0x165a2, SEEK_SET);
		fwrite(id, sizeof(BYTE), 2, fp_d64);
		// put original cosmetic id in unused part of sector
		fseek(fp_d64, 0x165fe, SEEK_SET);
		fwrite(cosmetic_id, sizeof(BYTE), 2, fp_d64);
	}

fail:
	if (fp_nib != NULL)
		fclose(fp_nib);
	if (fp_d64 != NULL)
		fclose(fp_d64);
	return (-1);
}
