/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *  Copyright (C) 1999      Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Copyright (C) 2005,2006 Wolfgang Moser <cbm(a)d81(o)de>
 */

#include "opencbm.h"

#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch.h"


#define DebugFormat 1

#define CBMFORNG 1

static unsigned char dskfrmt[] = {
#include "cbmforng.inc"
};

static void help()
{
    printf(
#ifdef CBMFORNG
"Usage: cbmforng [OPTION]... DRIVE NAME,ID\n"
#else
"Usage: cbmformat [OPTION]... DRIVE NAME,ID\n"
#endif
"Fast and reliable CBM-1541 disk formatter\n"
"\n"
"  -h, --help       display this help and exit\n"
"  -V, --version    display version information and exit\n"
"\n"
"  -n, --no-bump    do not bump drive head\n"
"  -r, --retries n  number of retries on error (whole disk)\n"
"  -x, --extended   format 40 track disk\n"
"  -c, --clear      clear (demagnetize) this disk.\n"
"                   this is highly recommended if this disk\n"
"                   is used for the first time.\n"
"  -v, --verify     verify each track after it is written\n"
"  -o, --original   fill sectors with the original pattern\n"
"                   (0x4b, 0x01...) instead of zeroes\n"
"  -s, --status     display drive status after formatting\n"
"\n"
);
}

static void hint(char *s)
{
    fprintf(stderr, "Try `%s' -h for more information.\n", s);
}

static void encodeGCR_to_buffer(char **dest, char *source)
{
    unsigned char GCRdeTab[]={
      10, 11, 18, 19, 14, 15, 22, 23, 9, 25, 26, 27, 13, 29, 30, 21
      };
	int i;
	unsigned short tdest=0;		// at least 16 bits for overflow shifting

	for(i=0;i<4;++i)
	{
		tdest<<=5;
		tdest|=GCRdeTab[(source[i]>>4)&0x0f];
		tdest<<=5;
		tdest|=GCRdeTab[source[i]&0x0f];

		*((*dest)++)=(tdest>>((i+1)*2))&0xFF;
	}
	*((*dest)++)=tdest&0xFF;
}

static void prepareFmtPattern(char *GCRbuf, unsigned char pattern, unsigned char maxtrack, char HID1, char HID2)
{
	char T1_sig, Tn_sig, DBfiller, HD1_fill, HD2_fill;
	char source[4];
	int i;

	T1_sig   = 0x4b;
	Tn_sig   = 0x4b;
	DBfiller = 0x01;
	HD1_fill = 0x0f;
	HD2_fill = 0x0f;

	switch(pattern)
	{
		case 0x00:		// Zero format pattern
		default:
			T1_sig   = 0x00;
			Tn_sig   = 0x00;
			DBfiller = 0x00;
			break;
		case 0x4b:		// Commodore 1541 disk drive original pattern
		case 0x4c:		// SpeedDOS format pattern, same as original
			T1_sig   = 0x00;
			break;
		case 0x4d:		// Dolphin-DOS format pattern (40 tracks only)
			HD1_fill = maxtrack>35 ? 0x0d : 0x0f;
			break;
	}
		// prepare intermediate data block section
	for(i=3;i>=0;--i) source[i] = DBfiller;
	encodeGCR_to_buffer(&GCRbuf, source);

		// prepare start section for tracks > 1
	source[0]=0x07;			// after SYNC data block marker
	source[1]=Tn_sig;
	encodeGCR_to_buffer(&GCRbuf, source);

		// prepare start section for track 1
	source[1]=T1_sig;
	encodeGCR_to_buffer(&GCRbuf, source);

		// prepare end section for track 1
	source[0]=DBfiller;
	source[1]=T1_sig^DBfiller;	// data block checksum
#if 1					// standard CBM format filler bytes
	source[2]=0x00;
	source[3]=0x00;
#else
	source[2]=0x0f;		// for MNib analysing bash script
	source[3]=0x0f;
#endif
	encodeGCR_to_buffer(&GCRbuf, source);

		// prepare end section for tracks > 1
	source[1]=Tn_sig^DBfiller;	// data block checksum
	encodeGCR_to_buffer(&GCRbuf, source);

		// prepare second part of all header blocks
	source[0]=HID2;
	source[1]=HID1;
	source[2]=HD1_fill;
	source[3]=HD2_fill;
	encodeGCR_to_buffer(&GCRbuf, source);
}

#if defined(DebugFormat) && DebugFormat!=0
static int cbm_download(CBM_FILE HandleDevice, __u_char DeviceAddress, 
                        int DriveMemAddress, void *Program, size_t Size)
{
    char *bufferToProgram = Program;

    unsigned char command[] = { 'M', '-', 'R', ' ', ' ', ' ' };
    size_t i;
    int rv = 0;
    int c;

    for(i = 0; i < Size; i += 32)
    {
        cbm_listen(HandleDevice, DeviceAddress, 15);

        // Calculate how much bytes are left

        c = Size - i;

        // Do we have more than 32? Than, restrict to 32

        if (c > 32)
        {
            c = 32;
        }

        // The command M-R consists of:
        // M-R <lowaddress> <highaddress> <count>
        // build that command:

        command[3] = (unsigned char) (DriveMemAddress % 256);
        command[4] = (unsigned char) (DriveMemAddress / 256);
        command[5] = (unsigned char) c; 

        // Write the M-W command to the drive...

        cbm_raw_write(HandleDevice, command, sizeof(command));

        // The UNLISTEN is the signal for the drive 
        // to start execution of the command

        cbm_unlisten(HandleDevice);

        cbm_talk(HandleDevice, DeviceAddress, 15);

        // ... as well as the (up to 32) data bytes

        cbm_raw_read(HandleDevice, bufferToProgram, c);

        // Now, advance the pointer into drive memory
        // as well to the program in PC's memory in case we
        // might need to use it again for another M-W command

        DriveMemAddress += c;
        bufferToProgram += c;

        // Advance the return value of send bytes, too.

        rv += c;

        // The UNLISTEN is the signal for the drive 
        // to start execution of the command

        cbm_untalk(HandleDevice);
    }

    return rv;
}
#endif

int ARCH_MAINDECL main(int argc, char *argv[])
{
    int status = 0, id_ofs = 0, name_len;
    CBM_FILE fd;
    unsigned char drive, starttrack = 1, endtrack = 35, bump = 1, orig = 0;
    unsigned char verify = 0, demagnetize = 0, retries = 7;
    char cmd[40], c, name[20], *arg;
    int berror = 0;

    struct option longopts[] =
    {
        { "help"       , no_argument      , NULL, 'h' },
        { "version"    , no_argument      , NULL, 'V' },
        { "no-bump"    , no_argument      , NULL, 'n' },
        { "extended"   , no_argument      , NULL, 'x' },
        { "original"   , no_argument      , NULL, 'o' },
        { "status"     , no_argument      , NULL, 's' },
        { "verify"     , no_argument      , NULL, 'v' },
        { "clear"      , no_argument      , NULL, 'c' },
        { "retries"    , required_argument, NULL, 'r' },

        /* undocumented */
        { "fillpattern", required_argument, NULL, 'f' },
        { "begin-track", required_argument, NULL, 'b' },
        { "end-track"  , required_argument, NULL, 'e' },
        { NULL         , 0                , NULL, 0   }
    };

    const char shortopts[] ="hVnxosvcr:f:b:e:";

    while((c=(unsigned char)getopt_long(argc, argv, shortopts, longopts, NULL)) != -1)
    {
        switch(c)
        {
            case 'n': bump = 0;
                      break;
            case 'o': orig = 0x4b;
                      break;
            case 's': status = 1;
                      break;
            case 'x': starttrack =  1;
                      endtrack   = 40;
                      break;
            case 'h': help();
                      return 0;
#ifdef CBMFORNG
            case 'V': printf("cbmforng %s\n", OPENCBM_VERSION);
#else
            case 'V': printf("cbmformat %s\n", OPENCBM_VERSION);
#endif
                      return 0;
            case 'v': verify = 1;
                      break;
            case 'c': demagnetize = 1;
                      break;
            case 'r': retries = arch_atoc(optarg);
                      if(retries<1)       retries= 1;
                      else if(retries>63) retries=63;
                      break;

            case 'f': orig = arch_atoc(optarg);
                      break;
            case 'b': starttrack = arch_atoc(optarg);
                      break;
            case 'e': endtrack = arch_atoc(optarg);
                      break;
            default : hint(argv[0]);
                      return 1;
        }
    }

    if(optind + 2 != argc)
    {
        fprintf(stderr, "Usage: %s [OPTION]... DRIVE NAME,ID\n", argv[0]);
        hint(argv[0]);
        return 1;
    }

    arg = argv[optind++];
    drive = arch_atoc(arg);
    if(drive < 8 || drive > 11)
    {
        fprintf(stderr, "Invalid drive number (%s)\n", arg);
        return 1;
    }
    
    arg      = argv[optind++];
    name_len = 0;
    while(*arg)
    {
        c = (unsigned char) toupper(*arg);
        if(c == ',')
        {
            if(id_ofs)
            {
                fprintf(stderr, "More than one `,' in disk name\n");
                return 1;
            }
            id_ofs = name_len;
        }
        name[name_len++] = c;
        if(name_len > 19)
        {
            fprintf(stderr, "Disk name too long\n");
            return 1;
        }
        arg++;
    }
    name[name_len] = 0;

	if(name_len - id_ofs != 3)
	{
		fprintf(stderr, "Missing `,' in disk name or ID field not equal to two characters\n");
		return 1;
	}

    if(cbm_driver_open(&fd, 0) == 0)
    {
        cbm_upload(fd, drive, 0x0300, dskfrmt, sizeof(dskfrmt));


		prepareFmtPattern(cmd, orig, endtrack, name[id_ofs+1], name[id_ofs+2]);
		cmd[30]=starttrack;		// start track parameter
		cmd[31]=endtrack+1;		// end track parameter
		cmd[32]=(retries & ~0xC0) | (bump?0x40:0xC0);
								// number of retries (per disk, not per track)
		cmd[33]=bump;			// flag, if an initial head bump should be done
		cmd[34]=demagnetize;	// flag, if the disk should be demagnetized
		cmd[35]=verify;			// flag, if the disk should be verified

#if 0		// for checking the generated format patterns
{
	int j,k;
	for(j=0;j<34;j+=5)
	{
		for(k=0;k<5;k++)
		{
			printf(" $%02X", cmd[j+k]&0xFF);
		}
		printf("\n");
	}
	printf(" $%02X\n", cmd[j]&0xFF);
}
#endif
		cbm_upload(fd, drive, 0x0200 - 36, cmd, 36);

        sprintf(cmd, "M-E%c%c0:%s", 3, 3, name);
        cbm_exec_command(fd, drive, cmd, 7+name_len);
        berror = cbm_device_status(fd, drive, cmd, sizeof(cmd));

        if(berror && status)
        {
            printf("%s\n", cmd);
        }

        if(!berror && (endtrack > 35))
        {
            cbm_open(fd, drive, 2, "#", 1);
            cbm_exec_command(fd, drive, "U1:2 0 18 0", 11);
            cbm_exec_command(fd, drive, "B-P2 192", 8);
            cbm_listen(fd, drive, 2);
            while(endtrack > 35)
            {
                cbm_raw_write(fd, "\021\377\377\001", 4);
                endtrack--;
            }
            cbm_unlisten(fd);
            cbm_exec_command(fd, drive, "U2:2 0 18 0", 11);
            cbm_close(fd, drive, 2);
        }
        if(!berror && status)
        {
            cbm_device_status(fd, drive, cmd, sizeof(cmd));
            printf("%s\n", cmd);
        }
#if defined(DebugFormat) && DebugFormat!=0	// verbose output
        {
        	float RPMval;
           	int sectors, virtGAPsze, remainder, trackTailGAP, flags, retry = 0, lastTr;
           	const char *vrfy;
            unsigned char data[0x100];

				// in case of an error, get the logging buffer from 0x0700 instead of 0x0500
            if (cbm_download(fd, drive, berror?0x0700:0x0500, data, sizeof(data)) == sizeof(data))
            {
                int i;
				printf("Track|Retry|sctrs|slctd|| GAP |modulo |modulo|tail| Verify  | RPM  |\n"
				       "     |     |     | GAP ||adjst|complmt| dvsr |GAP |         |      |\n"
				       "-----+-----+-----+-----++-----+-------+------+----+---------+------+\n");

				lastTr=-1;
                for (i=0; i < sizeof(data); i+=4)
                {
                	if(data[i]==0) break;	// no more data is available

					if(data[i]==lastTr) retry++;
					else				retry=0;
					lastTr=data[i];

                	if(data[i]>=25)			// preselect track dependent constants
                	{
                		if(data[i]>=31) sectors=17, RPMval=60000000.0f/16;
                		else            sectors=18, RPMval=60000000.0f/15;
                	}
                	else
                	{
                		if(data[i]>=18) sectors=19, RPMval=60000000.0f/14;
                		else            sectors=21, RPMval=60000000.0f/13;
                	}

						// separate some flags
					flags=(data[i+3]>>6)&0x03;
					data[i+3]&=0x3f;

					switch(flags)
					{
						case 0x01: vrfy="SYNC fail"; break;
						case 0x02: vrfy="   OK    "; break;
						case 0x03: vrfy="vrfy fail"; break;
						default:   vrfy="   ./.   ";
					}

						// recalculation of the track tail GAP out of the
						// choosen GAP for this track, the new GAP size
						// adjustment and the complement of the remainder
						// of the adjustment division

					virtGAPsze=data[i+1] 		-5;	// virtual GAP increase to
						// prevent reformatting, when only one byte is missing
						// and other offset compensations


					remainder=((data[i+2]==0xff) ? virtGAPsze : sectors)
					             - data[i+3];


					trackTailGAP=((data[i+2]==0xff) ? 0 : data[i+2]*sectors + virtGAPsze)
					             + remainder;

						// the following constants are nybble based (double the
						// size of the well known constants for SYNC lengths,
						// block header size, data block GAP and data block)
						//
						// (0x01&data[i+1]&sectors) is a correction term, if "half
						// GAPs" are written and the number of sectors is odd
						//
					// RPMval / (sectors * (10+20+18+10 + 650 + data[i+1]) - (0x01&data[i+1]&sectors) + trackTailGAP - data[i+1])

					RPMval = (flags != 0x01) ?
						RPMval / (sectors * (10+20+18+10 + 650 + data[i+1]) - (0x01&data[i+1]&sectors) + trackTailGAP - data[i+1])
						: 0;

					printf(" %3u | ", data[i]);
					if(retry>0) printf("%3u", retry);
					else        printf("   ");

				   /*      " |sctrs |slctd   || GAP    |modulo    |modulo   |tail | Verify  | RPM  |\n"
				    *      " |      | GAP    ||adjst   |complmt   |         | GAP |         |      |\n"
				    *      "-+----- +-----   ++-----   +-------   +------   +---- +---------+------+\n"
				    */
					printf(" |  %2u |$%02X.%d||$%02X.%d| $%02X.%d | $%02X.%d|$%03X|%9s|%6.2f|\n",
					       sectors,
					       data[i+1]>>1,                       (data[i+1]<<3)&8,			// selected GAP
					       (((signed char)data[i+2])>>1)&0xFF, (data[i+2]<<3)&8,			// GAP adjust
					       data[i+3]>>1,                       (data[i+3]<<3)&8,			// modulo complement
					       remainder>>1,                       (remainder<<3)&8,			// modulo
					       (trackTailGAP>>1) + 1,                  							// track tail GAP (with roundup)
					       vrfy, RPMval);
                }
                printf("\n  *) Note: The fractional parts of all the GAP based numbers shown here\n"
                         "           (sedecimal values) are given due to nybble based calculations.\n");
            }
            else
            {
                fprintf(stderr, "error reading debug logging data!\n");
            }
        }
#endif
        cbm_driver_close(fd);
        return 0;
    }
    else
    {
        arch_error(0, arch_get_errno(), "%s", cbm_get_driver_name(0));
        return 1;
    }
}
