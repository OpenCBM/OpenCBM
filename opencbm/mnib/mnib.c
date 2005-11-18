/*
    mnib - Markus' G64 nibbler

    (C) 2000-04 Markus Brenner and Pete Rittwage

    V 0.10   implementation of serial and parallel protocol
    V 0.11   first density scan and nibbler functionality
    V 0.12   automatic port detection and improved program flow
    V 0.13   extended data output format
    V 0.14   fixed parallel port support
    V 0.15   2nd try for parallel port fix
    V 0.16   next try with adjustments to ECP ports
    V 0.17   added automatic drive type detection
    V 0.18   added 41 track support
    V 0.19   added Density and Halftrack command switches
    V 0.20   added Bump and Reset options
    V 0.21   added timeout routine for nibble transfer
    V 0.22   added flush command during reading
    V 0.23   disable interrupts during serial protocol
    V 0.24   improved serial protocol
    V 0.25   got rid of some more msleep()s
    V 0.26   added 'S' track reading (read without waiting for Sync)
    V 0.27   added hidden 'g' switch for GEOS 1.2 disk image
    V 0.28   improved killer detection (changed retries $80 -> $c0)
    V 0.29   added direct D64 nibble functionality
    V 0.30   added D64 error correction by multiple read
    V 0.31   added 40 track support for D64 images
    V 0.32   bin-include bn_flop.prg  (bin2h bn_flop.prg floppy_code bn_flop.h)
    V 0.33   improved D64 mode
    V 0.34   improved track cycle detection
    V 0.35   added XA1541 support, paranoid mode
    V 0.36   minor improvements on scan_track()
             added NIB write support contributed by Pete Rittwage
             reduce syncs contributed by Pete
             modified convert_sector_to_GCR() to handle disk errors (Pete)
			 added write functions for D64 and G64 (Pete)
			 added $00 byte support (Pete and Markus)
			 rewrote disk read logic (Pete)
			 added new find_track_cycle() that doesn't require syncs (Pete)
			 added error scanning functions (Pete)
			 added sector compare functions (Pete)
			 added adjust_target functionality (Pete)
			 added functions to handle alternate track alignments (Pete)
			 (LONGSYNC, WEAKRUN, RAPIDLOK, VMAX, AUTOGAP/VORPAL)
			 added functions to handle tricky disk protections (Pete)
			 refined disk writing routines for reliability/compatibility (Pete)
			 changed do_reduce_syncs() to strip_runs() and reduce_runs() for reuse (Pete)
			 added unformatted track detection and handling (Pete)
			 added extended killer track handling (Pete)
			 fixed track cycle detection from getting false positives (Pete)
			 added verify functionality (Pete)
			 moved development to CVS (Pete)
			 converted codebase to compile under DOS and CBM4* (Pete and Tim)
			 code cleanup and bug fixes (Spiro and Nate) (Thanks!)
			 Makefile(s) created and later unified (Nate Lawson)
                         Made mnib compile for cbm4win, too. (Spiro)

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef DJGPP
#include <dos.h>
#include <conio.h>
#include "cbm.h"
#include "kernel.h"
typedef int CBM_FILE;
#define ARCH_MAINDECL
#else
#include <opencbm.h>
#include "arch.h"
#define delay(x) arch_usleep(x*1000)
#endif

#include "gcr.h"
#include "bn_flop.h"		/* floppy code: unsigned char floppy_code[] */
#include "version.h"
#include "mnib.h"

char bitrate_range[4] = { 43 * 2, 31 * 2, 25 * 2, 18 * 2 };
char bitrate_value[4] = { 0x00, 0x20, 0x40, 0x60 };
char density_branch[4] = { 0xb1, 0xb5, 0xb7, 0xb9 };

BYTE start_track, end_track, track_inc;
int reduce_syncs, reduce_weak, reduce_gaps;
int fix_gcr, aggressive_gcr;
int current_track;
int align, force_align, read_killer;
unsigned int error_retries;
unsigned int lpt[4];
int lpt_num;
int drivetype;
unsigned int floppybytes;
int disktype, imagetype;
int mode;
int verify;
int auto_density_adjust;

FILE *fplog;

static int compare_extension(char * filename, char * extension);
static void upload_code(CBM_FILE fd, BYTE drive);
static void file2disk(CBM_FILE fd, char * filename);
static void disk2file(CBM_FILE fd, char * filename);
static int verify_floppy(CBM_FILE fd);
#ifdef DJGPP
static int find_par_port(CBM_FILE fd);
#endif

static int
compare_extension(char * filename, char * extension)
{
	char *dot;

	dot = strrchr(filename, '.');
	if (dot == NULL)
		return (0);

	for (++dot; *dot != '\0'; dot++, extension++)
		if (tolower(*dot) != tolower(*extension))
			return (0);

	if (*extension == '\0')
		return (1);
	else
		return (0);
}

static void
upload_code(CBM_FILE fd, BYTE drive)
{
	unsigned int databytes, start;
	int i;

	/* patchdata if using 1571 drive */
	unsigned int patch_pos[9] =
	  { 0x72, 0x89, 0x9e, 0x1da, 0x224, 0x258, 0x262, 0x293, 0x2a6 };

	databytes = sizeof(floppy_code);

	start = floppy_code[0] + (floppy_code[1] << 8);

	/* patch code if using 1571 drive */
	if (drivetype == 1571)
	{
		for (i = 0; i < 9; i++)
		{
			if (floppy_code[patch_pos[i]] != 0x18)
				printf("Possibly bad patch at %04x!\n",
				  patch_pos[i]);
			floppy_code[patch_pos[i]] = 0x40;
		}
	}

	printf("Uploading floppy-side code...\n");
	cbm_upload(fd, drive, start, floppy_code + 2, databytes - 2);

	floppybytes = databytes;
}

int
test_par_port(CBM_FILE fd)
{
	unsigned int i, rv;

	send_mnib_cmd(fd, FL_TEST);

	for (i = 0, rv = 1; i < 0x100; i++)
		if (cbm_mnib_par_read(fd) != i)
			rv = 0;

	if (cbm_mnib_par_read(fd) != 0)
		rv = 0;
	return (rv);
}

int
verify_floppy(CBM_FILE fd)
{
	unsigned int i, rv;

	send_mnib_cmd(fd, FL_VERIFY_CODE);
	for (i = 2, rv = 1; i < floppybytes; i++)
	{
		if (cbm_mnib_par_read(fd) != floppy_code[i])
		{
			rv = 0;
			printf("diff: %d\n", i);
		}
	}
	for (; i < 0x0800 - 0x0300 + 2; i++)
		cbm_mnib_par_read(fd);

	if (cbm_mnib_par_read(fd) != 0)
		rv = 0;
	return (rv);
}

#ifdef DJGPP
int
find_par_port(CBM_FILE fd)
{
	int i;
	for (i = 0; set_par_port(i); i++)
	{
		if (test_par_port(fd))
		{
			printf(" Found!\n");
			return (1);
		}
		printf(" no\n");
	}
	return (0);		/* no parallel port found */
}
#endif // DJGPP

void
send_mnib_cmd(CBM_FILE fd, unsigned char cmd)
{
	cbm_mnib_par_write(fd, 0x00);
	cbm_mnib_par_write(fd, 0x55);
	cbm_mnib_par_write(fd, 0xaa);
	cbm_mnib_par_write(fd, 0xff);
	cbm_mnib_par_write(fd, cmd);
}

void
set_full_track(CBM_FILE fd)
{
	send_mnib_cmd(fd, FL_MOTOR);
	cbm_mnib_par_write(fd, 0xfc);	/* $1c00 CLEAR mask (clear stepper bits) */
	cbm_mnib_par_write(fd, 0x02);	/* $1c00  SET  mask (stepper bits = %10) */
	cbm_mnib_par_read(fd);
	delay(500);					/* wait for motor to step */
}

void
motor_on(CBM_FILE fd)
{
	send_mnib_cmd(fd, FL_MOTOR);
	cbm_mnib_par_write(fd, 0xf3);	/* $1c00 CLEAR mask */
	cbm_mnib_par_write(fd, 0x0c);	/* $1c00  SET  mask (LED + motor ON) */
	cbm_mnib_par_read(fd);
	delay(500);					/* wait for motor to turn on */
}

void
motor_off(CBM_FILE fd)
{
	send_mnib_cmd(fd, FL_MOTOR);
	cbm_mnib_par_write(fd, 0xf3);	/* $1c00 CLEAR mask */
	cbm_mnib_par_write(fd, 0x00);	/* $1c00  SET  mask (LED + motor OFF) */
	cbm_mnib_par_read(fd);
	delay(500);					/* wait for motor to turn on */
}

void
step_to_halftrack(CBM_FILE fd, int halftrack)
{
	send_mnib_cmd(fd, FL_STEPTO);
	cbm_mnib_par_write(fd, (halftrack != 0) ? halftrack : 1);
	cbm_mnib_par_read(fd);
}

unsigned int
track_capacity(CBM_FILE fd)
{
	unsigned int capacity;
	send_mnib_cmd(fd, FL_CAPACITY);
	capacity = (unsigned int) cbm_mnib_par_read(fd);
	capacity |= (unsigned int) cbm_mnib_par_read(fd) << 8;
	return (capacity);
}

void
reset_floppy(CBM_FILE fd, BYTE drive)
{
	char cmd[80];

	motor_on(fd);
	step_to_halftrack(fd, 36);
	send_mnib_cmd(fd, FL_RESET);
	printf("drive reset...\n");
	delay(5000);
	cbm_listen(fd, drive, 15);

	cbm_raw_write(fd, "I", 1);

	cbm_unlisten(fd);
	delay(5000);
	sprintf(cmd, "M-E%c%c", 0x00, 0x03);
	cbm_listen(fd, drive, 15);

	cbm_raw_write(fd, cmd, 5);

	cbm_unlisten(fd);
	cbm_mnib_par_read(fd);
}

int
set_density(CBM_FILE fd, int density)
{
	send_mnib_cmd(fd, FL_DENSITY);
	cbm_mnib_par_write(fd, density_branch[density]);
	cbm_mnib_par_write(fd, 0x9f);
	cbm_mnib_par_write(fd, bitrate_value[density]);
	cbm_mnib_par_read(fd);

	return (density);
}

/* $13d6 */
int
set_bitrate(CBM_FILE fd, int density)
{
	send_mnib_cmd(fd, FL_MOTOR);
	cbm_mnib_par_write(fd, 0x9f);					/* $1c00 CLEAR mask */
	cbm_mnib_par_write(fd, bitrate_value[density]);	/* $1c00  SET  mask */
	cbm_mnib_par_read(fd);
	return (density);
}

/* $13bc */
BYTE
set_default_bitrate(CBM_FILE fd, int track)
{
	BYTE density;

	for (density = 3; track >= bitrate_range[density]; density--);
	send_mnib_cmd(fd, FL_DENSITY);
	cbm_mnib_par_write(fd, density_branch[density]);
	cbm_mnib_par_write(fd, 0x9f);					/* $1c00 CLEAR mask */
	cbm_mnib_par_write(fd, bitrate_value[density]);	/* $1c00  SET  mask */
	cbm_mnib_par_read(fd);
	return (density);
}


/* $152b Density Scan */
BYTE
scan_track(CBM_FILE fd, int track)
{
	BYTE density, killer_info;
	int i;
    int bin;
	BYTE count;
	BYTE density_major[4], iMajorMax; /* 50% majorities for bit rate */
	BYTE density_stats[4], iStatsMax; /* total occurrences */

	density = set_default_bitrate(fd, track);

	/* Scan for killer track */
	send_mnib_cmd(fd, FL_SCANKILLER);
	killer_info = cbm_mnib_par_read(fd);

	if (killer_info & BM_FF_TRACK)
		return (density | killer_info);

	for (bin = 0; bin < 4; bin++)
		density_major[bin] = density_stats[bin] = 0;

	/* Use medium bitrate for scan */
	set_bitrate(fd, 2);

	for (i = 0; i < 6; i++)
	{
		send_mnib_cmd(fd, FL_SCANDENSITY);

		/* Floppy sends statistic data in reverse bit-rate order */
		for (bin = 3; bin >= 0; bin--)
		{
			count = cbm_mnib_par_read(fd);
			if (count >= 0x40)
				density_major[bin]++;
			density_stats[bin] += count;
		}
		cbm_mnib_par_read(fd);
	}

	if (density_major[density] > 0)
		return (density | killer_info);

	iMajorMax = iStatsMax = 0;
	for (bin = 1; bin < 4; bin++)
	{
		if (density_major[bin] > density_major[iMajorMax])
			iMajorMax = (BYTE) bin;
		if (density_stats[bin] > density_stats[iStatsMax])
			iStatsMax = (BYTE) bin;
	}

	if (density_major[iMajorMax] > 0)
		density = iMajorMax;
	else if (density_stats[iStatsMax] > density_stats[density])
		density = iStatsMax;

	/* Set bitrate to the discovered density and scan for killer track */
	set_bitrate(fd, density);
	send_mnib_cmd(fd, FL_SCANKILLER);
	killer_info = cbm_mnib_par_read(fd);

	return (density | killer_info);
}

BYTE
scan_density(CBM_FILE fd)
{
	BYTE track;
	BYTE density;

	if (!test_par_port(fd))
		return 0;

	printf("SCAN\n");
	motor_on(fd);
	for (track = start_track; track <= end_track; track += track_inc)
	{
		step_to_halftrack(fd, track);
		printf("\n%02d: ", track);
		density = scan_track(fd, track);
		if (density & BM_FF_TRACK)
			printf("F");
		else if (density & BM_NO_SYNC)
			printf("S");
		else
			printf("%d", (density & 3));
	}
	return (density);
}

/* This routine measures track capacity at all densities */
void
adjust_target(CBM_FILE fd)
{
	int i;
	unsigned int cap1, cap2;

	printf("\nTesting track capacity for each density:\n");

	step_to_halftrack(fd, 2);

	for (i = 0; i < 4; i++)
	{
		set_bitrate(fd, i);
		cap1 = track_capacity(fd);
		cap2 = track_capacity(fd);

		capacity[i] = ((cap1 + cap2) / 2) - CAPACITY_MARGIN;
		printf("(%d) %d, %d: %d - (%d < %d < %d) ", i, cap1, cap2,
		  capacity[i] + CAPACITY_MARGIN, capacity_min[i], capacity[i],
		  capacity_max[i]);

		if (capacity[i] < capacity_min[i] || capacity[i] > capacity_max[i])
		{
			printf("- OUT OF RANGE\n");
		}
		else if (capacity[i] < capacity_min[i] - 100 ||
		  capacity[i] > capacity_max[i] + 100)
		{
			printf("\nCapacity is too far out of range.\n\n");
			printf("Possible problems:\n");
			printf("1) No disk in drive.\n");
			printf("2) Write protect is on.\n");
			printf("3) Disk is damaged.\n");
			printf("4) Disk drive needs adjusted or repaired.\n");
			exit(0);
		}
		else
			printf("- OK\n");
	}

	printf("\nDrive speed averaged %.1f RPM.\n",
	  (float) 2309195 / (capacity[3] + CAPACITY_MARGIN));
}

char
char_fgetc(FILE *stream)
{
    return (char) fgetc(stream);
}

static void
file2disk(CBM_FILE fd, char * filename)
{
	int i;
	FILE *fpin;
	char mnibheader[0x100], g64header[0x2a0];
	int nibsize;

	motor_on(fd);
	if (auto_density_adjust)
		adjust_target(fd);

	if ((fpin = fopen(filename, "rb")) == NULL)
	{
		fprintf(stderr, "Couldn't open input file %s!\n", filename);
		exit(2);
	}

	if (compare_extension(filename, "D64"))
	{
		imagetype = IMAGE_D64;
		write_d64(fd, fpin);
	}
	else if (compare_extension(filename, "G64"))
	{
		imagetype = IMAGE_G64;
		memset(g64header, 0x00, 0x2ac);
		for (i = 0; i < 0x2ac; i++)
			g64header[i] = char_fgetc(fpin);
		parse_disk(fd, fpin, g64header + 0x9);
	}
	else if (compare_extension(filename, "NIB"))
	{
		imagetype = IMAGE_NIB;

		/* Determine number of tracks (40 or 41) */
		fseek(fpin, 0, SEEK_END);
		nibsize = ftell(fpin);
		// printf("image size:%d\n",nibsize);

		if (nibsize == 327936)	// 40 tracks
		{
			end_track = 40 * 2;
			printf("\n40 track image");
		}
		else if (nibsize == 338128)
		{
			end_track = 41 * 2;	// 41 tracks
			printf("\n41 track image");
		}
		else
		{
			printf("unsupported file format");
			exit(0);
		}
		rewind(fpin);

		memset(mnibheader, 0x00, 0x100);
		for (i = 0; i < 0x100; i++)
			mnibheader[i] = char_fgetc(fpin);
		parse_disk(fd, fpin, mnibheader + 0x10);
	}
	else
		printf("\nUnknown image type");

	printf("\n");
	cbm_mnib_par_read(fd);

	fclose(fpin);
}

static void
disk2file(CBM_FILE fd, char *filename)
{
	int i;
	FILE *fpout;
	char header[0x100], logfilename[128], *dotpos;

	if (compare_extension(filename, "D64"))
		imagetype = IMAGE_D64;
	else if (compare_extension(filename, "G64"))
		imagetype = IMAGE_G64;
	else if (compare_extension(filename, "NIB"))
		imagetype = IMAGE_NIB;
	else
	{
		strcat(filename, ".nib");
		imagetype = IMAGE_NIB;
	}

	if (imagetype != IMAGE_NIB && imagetype != IMAGE_D64)
	{
		printf("Only the NIB and D64 formats are supported for reading.");
		exit(2);
	}

	/* create log file */
	strcpy(logfilename, filename);
	dotpos = strrchr(logfilename, '.');
	if (dotpos != NULL)
		*dotpos = '\0';
	strcat(logfilename, ".log");

	if ((fplog = fopen(logfilename, "wb")) == NULL)
	{
		fprintf(stderr, "Couldn't create log file %s!\n",
		  logfilename);
		exit(2);
	}
	fprintf(fplog, "%s\n", VERSION);

	/* create output file */
	if ((fpout = fopen(filename, "wb")) == NULL)
	{
		fprintf(stderr, "Couldn't create output file %s!\n",
		  filename);
		exit(2);
	}

	/* read data from drive to file */
	motor_on(fd);

	if (imagetype == IMAGE_NIB)
	{
		/* write NIB-header */
		memset(header, 0x00, 0x100);
		sprintf(header, "MNIB-1541-RAW%c%c%c", 1, 0, 0);

		for (i = 0; i < 0x100; i++)
			fputc(header[i], fpout);

		/* read out disk into file */
		read_nib(fd, fpout, header + 0x10);

		/* fill NIB-header */
		rewind(fpout);
		for (i = 0; i < 0x100; i++)
		{
			fputc(header[i], fpout);
		}
		fseek(fpout, 0, SEEK_END);

	}
	else
	{
		/* D64 */
		read_d64(fd, fpout);
	}

	cbm_mnib_par_read(fd);
	printf("done reading.\n");

	fclose(fpout);
	fclose(fplog);
}

static void
usage(void)
{
	fprintf(stderr, "usage: mnib [options] <filename>\n");
	fprintf(stderr, " -w : Write disk image\n");
	fprintf(stderr, " -v : Verify written data (W)\n");
	fprintf(stderr, " -u : Unformat disk (removes *ALL* data)\n");
	fprintf(stderr, " -l : Limit functions to 40 tracks (R/W)\n");
	fprintf(stderr, " -h : Use halftracks (R/W)\n");
	fprintf(stderr, " -k : Disable reading 'killer' tracks (R)\n");
	fprintf(stderr, " -r : Disable 'reduce syncs' option (R)\n");
	fprintf(stderr, " -g : Enable 'reduce gaps' option (R)\n");
	fprintf(stderr, " -0 : Enable 'reduce weak' option (R)\n");
	fprintf(stderr, " -f : Disable weak GCR bit simulation (W)\n");
	fprintf(stderr, " -aX: Alternative track alignments (W)\n");
	fprintf(stderr, " -eX: Extended read retries (R)\n");
	fprintf(stderr, " -pX: Custom protection handlers (W)\n");
	exit(1);
}

int ARCH_MAINDECL
main(int argc, char *argv[])
{
    CBM_FILE fd;
    BYTE drive = 8;
	int bump, reset, ok;
	char cmd[80], error[500];
	char filename[80];

	fprintf(stdout,
	  "\nmnib - Commodore 1541/1571 disk image nibbler.\n"
	  "(C) 2000-05 Markus Brenner and Pete Rittwage.\n"
	  "Version " VERSION "\n\n");

#ifdef DJGPP
	fd = 1;
#endif
	bump = reset = 1;	// by default, use reset, bump

	start_track = 1 * 2;
	end_track = 41 * 2;
	track_inc = 2;

	reduce_syncs = 1;
	reduce_weak = 0;
	reduce_gaps = 0;
	read_killer = 1;
	fix_gcr = 1;
	error_retries = 10;
	verify = 0;
	auto_density_adjust = 1;

	mode = MODE_READ_DISK;	// default to read a disk
	disktype = DISK_NORMAL;
	align = ALIGN_NONE;
	force_align = ALIGN_NONE;

	while (--argc && (*(++argv)[0] == '-'))
	{
		switch (tolower((*argv)[1]))
		{
		case 'h':
			track_inc = 1;
			printf("* Reading halftracks\n");
			break;

		case 'v':
			verify = 1;
			printf("* Verify written data\n");
			break;

		case 'k':
			read_killer = 0;
			printf("* Ignoring 'killer' tracks\n");
			break;

		case 'l':
			end_track = 40 * 2;
			printf("* Limiting functions to 40 tracks\n");
			break;

		case 'w':
			mode = MODE_WRITE_DISK;
			break;

		case 'u':
			mode = MODE_UNFORMAT_DISK;
			break;

		case 't':
			// hidden secret raw track file writing mode
			printf("* Raw MTOOL track writing mode\n");
			mode = MODE_WRITE_RAW;
			break;

		case 'p':
			// custom protection handling
			printf("* Custom copy protection handler: ");
			if ((*argv)[2] == 'x')
			{
				printf("V-MAX!\n");
				force_align = ALIGN_VMAX;
				fix_gcr = 0;
			}
			else if ((*argv)[2] == 'g')
			{
				printf("GMA/SecuriSpeed\n");
				reduce_syncs = 0;
				reduce_weak = 1;
			}
			else if ((*argv)[2] == 'v')
			{
				printf("VORPAL (NEWER)\n");
				force_align = ALIGN_AUTOGAP;
			}
			else if ((*argv)[2] == 'r')
			{
				printf("RAPIDLOK\n");
				reduce_syncs = 1;
				reduce_weak = 1;
			}
			else
				printf("Unknown protection handler\n");
			break;

		case 'a':
			// custom alignment handling
			printf("* Custom alignment: ");
			if ((*argv)[2] == '0')
			{
				printf("sector 0\n");
				force_align = ALIGN_SEC0;
			}
			if ((*argv)[2] == 'g')
			{
				printf("gap\n");
				force_align = ALIGN_GAP;
			}
			else if ((*argv)[2] == 'w')
			{
				printf("longest weak run\n");
				force_align = ALIGN_WEAK;
			}
			else if ((*argv)[2] == 's')
			{
				printf("longest sync\n");
				force_align = ALIGN_LONGSYNC;
			}
			else if ((*argv)[2] == 'a')
			{
				printf("autogap\n");
				force_align = ALIGN_AUTOGAP;
			}
			else
				printf("Unknown alignment parameter\n");
			break;

		case 'r':
			reduce_syncs = 0;
			printf("* Disabled 'reduce syncs' option\n");
			break;

		case '0':
			reduce_weak = 1;
			printf("* Enabled 'reduce weak' option\n");
			break;

		case 'g':
			reduce_gaps = 1;
			printf("* Enabled 'reduce gaps' option\n");
			break;

		case 'f':
			fix_gcr = 0;
			printf("* Disabled weak GCR bit simulation\n");
			break;

		case 'd':
			auto_density_adjust = 0;
			printf("* Auto density adjust off\n");
			break;

		case 'e':	// change read retries
			if (!(*argv)[2])
				usage();
			error_retries = atoi((char *) (&(*argv)[2]));
			printf("* Read retries set to %d\n", error_retries);
			break;

		default:
			usage();
			break;
		}
	}
	printf("-\n");

	if (argc < 1 && !mode)
		usage();
	strcpy(filename, argv[0]);


#ifdef DJGPP
	calibrate();

	if (!detect_ports(reset))
		exit(3);
#else
	/* under Linux we have to open the device via cbm4linux */
	cbm_driver_open(&fd, 0);
#endif

	/* prepare error string $73: CBM DOS V2.6 1541 */
	sprintf(cmd, "M-W%c%c%c%c%c%c%c%c", 0, 3, 5, 0xa9, 0x73, 0x4c, 0xc1, 0xe6);
	cbm_exec_command(fd, drive, cmd, 11);
	sprintf(cmd, "M-E%c%c", 0x00, 0x03);
	cbm_exec_command(fd, drive, cmd, 5);
	cbm_device_status(fd, drive, error, 500);
	printf("Drive Version: %s\n", error);

	if (error[18] == '4')
		drivetype = 1541;
	else if (error[18] == '7')
		drivetype = 1571;
	else
		drivetype = 0;	/* unknown drive, use 1541 code */

	printf("Drive type: %d\n", drivetype);

	if (bump)
	{
		/* perform a bump */
		delay(1000);
		printf("Bumping...\n");
		sprintf(cmd, "M-W%c%c%c%c%c", 6, 0, 2, 1, 0);
		cbm_exec_command(fd, drive, cmd, 8);
		sprintf(cmd, "M-W%c%c%c%c", 0, 0, 1, 0xc0);
		cbm_exec_command(fd, drive, cmd, 7);
		delay(2500);
	}

	cbm_exec_command(fd, drive, "U0>M0", 0);
	cbm_exec_command(fd, drive, "I0:", 0);
	printf("Initializing\n");

	upload_code(fd, drive);
	sprintf(cmd, "M-E%c%c", 0x00, 0x03);
	cbm_exec_command(fd, drive, cmd, 5);

	cbm_mnib_par_read(fd);

#ifdef DJGPP
	if (!find_par_port(fd))
		exit(4);
#endif

	fprintf(stderr, "Port Test: %s, ", test_par_port(fd)? "OK" : "FAILED");
	fprintf(stderr, "Code Verify: %s\n", (ok =
	    verify_floppy(fd))? "OK" : "FAILED");
	if (!ok)
	{
		printf("Failed parallel port transfer test. Check cabling.\n");

#ifdef linux
		cbm_reset(fd);
		cbm_driver_close(fd);
#endif
		exit(5);
	}
	printf("\n");

	switch (mode)
	{
	case MODE_READ_DISK:
		disk2file(fd, filename);
		break;

	case MODE_WRITE_DISK:
		printf("Ready to write '%s'.\n", filename);
		printf("Current disk WILL be OVERWRITTEN!\n"
		  "Press ENTER to continue or CTRL-C to quit.\n");
		getchar();
		file2disk(fd, filename);
		break;

	case MODE_UNFORMAT_DISK:
		printf("Ready to unformat disk.\n");
		printf("Current disk WILL be DESTROYED!\n"
		  "Press ENTER to continue or CTRL-C to quit.\n");
		getchar();
		unformat_disk(fd);
		break;

	case MODE_WRITE_RAW:
		printf("Ready to write raw tracks to disk.\n");
		printf("Current disk WILL be OVERWRITTEN!\n"
		  "Press ENTER to continue or CTRL-C to quit.\n");
		getchar();
		write_raw(fd);
		break;
	}

	motor_on(fd);
	step_to_halftrack(fd, 18 * 2);

#ifdef DJGPP
	send_mnib_cmd(fd, FL_RESET);
	printf("drive reset...\n");
	delay(2000);
#else
	cbm_reset(fd);
	cbm_driver_close(fd);
#endif

	exit(1);
}
