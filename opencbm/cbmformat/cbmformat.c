/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999 Michael Klein <michael.klein@puffin.lb.shuttle.de>
*/

#ifdef SAVE_RCSID
static char *rcsid =
    "@(#) $Id: cbmformat.c,v 1.4 2005-04-17 15:32:17 strik Exp $";
#endif

#include "opencbm.h"

#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch.h"

static unsigned char dskfrmt[] = {
#include "cbmformat.inc"
};

static void help()
{
    printf(
"Usage: cbmformat [OPTION]... DRIVE NAME,ID\n"
"Fast CBM-1541 disk formatter\n"
"\n"
"  -h, --help       display this help and exit\n"
"  -V, --version    display version information and exit\n"
"\n"
"  -n, --no-bump    do not bump drive head\n"
"  -x, --extended   format 40 track disk\n"
"  -o, --original   fill sectors with the original pattern (0x4b, 0x01...)\n"
"                   instead of zeroes\n"
"  -s, --status     display drive status after formatting\n"
"  -p, --progress   display progress indicator\n"
"\n"
);
}

static void hint(char *s)
{
    fprintf(stderr, "Try `%s' -h for more information.\n", s);
}

int
cbm_download(CBM_FILE HandleDevice, __u_char DeviceAddress, 
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

int ARCH_MAINDECL main(int argc, char *argv[])
{
    int status = 0, id_ofs = 0, name_len, i;
    CBM_FILE fd;
    unsigned char drive, tracks = 35, bump = 1, orig = 0, show_progress = 0;
    char cmd[40], c, name[20], *arg;

    struct option longopts[] =
    {
        { "help"       , no_argument      , NULL, 'h' },
        { "version"    , no_argument      , NULL, 'V' },
        { "no-bump"    , no_argument      , NULL, 'n' },
        { "extended"   , no_argument      , NULL, 'x' },
        { "original"   , no_argument      , NULL, 'o' },
        { "status"     , no_argument      , NULL, 's' },
        { "progress"   , no_argument      , NULL, 'p' },

        /* undocumented */
        { "end-track"  , required_argument, NULL, 't' },
        { NULL         , 0                , NULL, 0   }
    };

    const char shortopts[] ="hVnxospt:";

    while((c=(unsigned char)getopt_long(argc, argv, shortopts, longopts, NULL)) != -1)
    {
        switch(c)
        {
            case 'n': bump = 0;
                      break;
            case 'o': orig = 1;
                      break;
            case 's': status = 1;
                      break;
            case 'x': tracks = 40;
                      break;
            case 'h': help();
                      return 0;
            case 'V': printf("cbmformat %s\n", VERSION);
                      return 0;
            case 'p': show_progress = 1;
                      break;
            case 't': tracks = arch_atoc(optarg);
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
    
    if(cbm_driver_open(&fd, 0) == 0)
    {
        cbm_upload(fd, drive, 0x0500, dskfrmt, sizeof(dskfrmt));
        sprintf(cmd, "M-E%c%c%c%c%c%c0:%s", 3, 5, tracks + 1, 
                orig, bump, show_progress, name);
        cbm_exec_command(fd, drive, cmd, 11+strlen(name));

        if(show_progress)
        {
            /* do some handshake */
            cbm_iec_release(fd, IEC_CLOCK);
            for(i = 1; i <= tracks; i++)
            {
                cbm_iec_wait(fd, IEC_DATA, 1);
                cbm_iec_set(fd, IEC_CLOCK);
                cbm_iec_wait(fd, IEC_DATA, 0);
                cbm_iec_release(fd, IEC_CLOCK);

                printf("#");
                fflush(stdout);
            }
            printf("\n");
        }

        if(tracks > 35)
        {
            cbm_open(fd, drive, 2, "#", 1);
            cbm_exec_command(fd, drive, "U1:2 0 18 0", 11);
            cbm_exec_command(fd, drive, "B-P2 192", 8);
            cbm_listen(fd, drive, 2);
            while(tracks > 35)
            {
                cbm_raw_write(fd, "\021\377\377\001", 4);
                tracks--;
            }
            cbm_unlisten(fd);
            cbm_exec_command(fd, drive, "U2:2 0 18 0", 11);
            cbm_close(fd, drive, 2);
        }
        if(status)
        {
            cbm_device_status(fd, drive, cmd, sizeof(cmd));
            printf("%s\n", cmd);
        }
/**/
        {
            unsigned char data[80];
            if (cbm_download(fd, drive, 0x506, data, sizeof(data)) == sizeof(data))
            {
                int i;
                for (i=0; i < 40; i++)
                {
                    printf("Track %2u: Tries = %02X, GAP = %02X\n", i+1, data[i], data[i+40]);
                }
            }
            else
            {
                fprintf(stderr, "error reading data!\n");
            }
        }
/**/
        cbm_driver_close(fd);
        return 0;
    }
    else
    {
        arch_error(0, arch_get_errno(), "%s", cbm_get_driver_name(0));
        return 1;
    }
}
