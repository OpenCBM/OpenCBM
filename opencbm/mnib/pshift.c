/*
 * Pete's CBM NIB Image Track Shifter
 * Copyright (c) 2005 Pete Rittwage <peter@rittwage.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef unsigned char BYTE;
#include "prot.h"

static void usage(void);

static void
usage()
{
	fprintf(stderr, "usage: pshift [-t] [-b] infile\n"
	  "-t track: track to shift\n"
	  "-b bits: number of bits to shift right\n");
	exit(1);
}

int
main(int argc, char *argv[])
{
	char infile[80], outfile[80];
	char *dotpos;
	char mnibheader[0x100];
	int i, nibsize;
	int track, strack = 0, bits = 1;
	int header_entry = 0;
	int start_track;
	int end_track;
	int track_inc;
	FILE *fpin, *fpout;
	BYTE buffer[0x2000];

	printf("\npshift - Commodore NIB image track shifter.\n"
	  "(C) 2005 Pete Rittwage.\n"
	  "Version 0.1\n\n");

	start_track = 1 * 2;
	end_track = 41 * 2;
	track_inc = 2;

	while (--argc && (*(++argv)[0] == '-'))
	{
		switch (tolower((*argv)[1]))
		{
		case 't':	// track to shift
			strack = atoi(*argv + 2);
			if (strack == 0 || strack > 41) {
				fprintf(stderr,
				  "error: track must be from 1 to 41\n");
				exit(1);
			}
			printf("ARG: shifting track %d\n", strack);
			strack *= 2;
			break;
		case 'b':	// bits to shift
			bits = atoi(*argv + 2);
			if (bits < 1 || bits > 7) {
				fprintf(stderr,
				  "error: bits must be from 1 to 7\n");
				exit(1);
			}
			printf("ARG: shifting by %d bits\n", bits);
			break;
		default:
			usage();
			break;
		}
	}

	if (argc < 1)
		usage();

	strcpy(infile, argv[0]);
	strcpy(outfile, infile);
	dotpos = strrchr(outfile, '.');
	if (dotpos != NULL)
		*dotpos = '\0';
	strcat(outfile, "2.nib");

	if ((fpin = fopen(infile, "rb")) == NULL)
	{
		fprintf(stderr, "Couldn't open input file %s!\n", infile);
		exit(2);
	}

	if ((fpout = fopen(outfile, "wb")) == NULL)
	{
		fprintf(stderr, "Couldn't open output file %s!\n", outfile);
		exit(2);
	}

	/* determine number of tracks (40 or 41) */
	fseek(fpin, 0, SEEK_END);
	nibsize = ftell(fpin);
	//printf("image size:%d\n",nibsize);

	if (nibsize == 327936)	// 40 tracks
	{
		end_track = 40 * 2;
		printf("40 track image\n");
	}
	else
	{
		end_track = 41 * 2;	// 41 tracks
		printf("41 track image\n");
	}
	rewind(fpin);

	memset(mnibheader, 0, 0x100);
	fread(mnibheader, 0x100, 1, fpin);
	fwrite(mnibheader, 0x100, 1, fpout);

	printf("Processing GCR image [");

	for (track = start_track; track <= end_track; track += track_inc)
	{
		// clear buffers
		memset(buffer, 0, 0x2000);

		// skip halftracks or nonexistant tracks
		if ((mnibheader[0x10 + (header_entry * 2)] != track) ||
		  (mnibheader[0x10 + (header_entry * 2)] == 0))
		{
			fread(buffer, 0x2000, 1, fpin);
			memset(buffer, 0, 0x2000);
			fwrite(buffer, 0x2000, 1, fpout);
			printf("-");
			header_entry++;
			continue;
		}

		// get track from file
		fread(buffer, 1, 0x2000, fpin);
		if (track >= strack)
		{
			if (track == strack)
			{
				for (i = 0; i < bits; i++)
					shift_buffer(buffer, 0x2000, 1);

				printf("S");
			}
			else
				printf("o");
		}
		else
			printf("o");

		fwrite(buffer, 0x2000, 1, fpout);
		header_entry++;

	}
	printf("] done\n");

	fclose(fpin);
	fclose(fpout);
	exit(0);
}
