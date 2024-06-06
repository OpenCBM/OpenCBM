/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2024 Spiro Trikaliotis
*/

#include "opencbm.h"

#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch.h"
#include "libmisc.h"

#undef DUMMY_DRIVER_ACCESS
// #define DUMMY_DRIVER_ACCESS 1

#if DUMMY_DRIVER_ACCESS
#else
static unsigned char fdx000_turboread[] = {
#include "turboreadfdx000.inc"
};
#endif

enum imagetype_e {
    IMAGETYPE_D1M,
    IMAGETYPE_D2M,
    IMAGETYPE_D4M,
    IMAGETYPE_UNKNOWN
};

enum operationtype_e {
    OPERATIONTYPE_READ,
    OPERATIONTYPE_WRITE,
    OPERATIONTYPE_UNKNOWN
};

struct cmdline_parameter_s {
    char                 *adapter;
    const char           *image_filename;
    enum imagetype_e      imagetype;
    const char           *imagetype_text;
    enum operationtype_e  operationtype;
    const char           *operationtype_text;
    unsigned char         drive;
    enum cbm_device_type_e drive_type;
};

struct imageparameter_s {
    int max_track;
    int max_sector;
    int extra_blocks_at_end;
};

static struct imageparameter_s imageparameter[IMAGETYPE_UNKNOWN] =
{
    // D1M
    {
        80,    /* maximum track */
        4*5,   /* maximum sector */
        0      /* extra blocks at end */
    },

    // D2M
    {
        80,    /* maximum track */
        4*10,  /* maximum sector */
        0      /* extra blocks at end */
    },

    // D4M
    {
        80,    /* maximum track */
        4*20,  /* maximum sector */
        0      /* extra blocks at end */
    }
};

#define INIT_CMDLINE_PARAMETER { \
    NULL, \
    NULL, \
    IMAGETYPE_UNKNOWN, \
    "unknown CMD", \
    OPERATIONTYPE_UNKNOWN, \
    "unknown READ or WRITE", \
    -1 \
}

static void help()
{
    printf(
"Usage: fdx000copy [OPTION]... <DRIVE#> <FILENAME>\n"
"Poor man's CMD FDx000 image copier\n"
"\n"
"Currently, only READING of images is possible.\n"
"\n"
"  -h, --help                 display this help and exit\n"
"  -V, --version              display version information and exit\n"
"  -@, --adapter=plugin:bus   tell OpenCBM which backend plugin and bus to use\n"
"\n"
"  -1, --d1m                  use an 1 MB disk image (FD2000, FD4000)\n"
"  -2, --d2m                  use a 2 MB disk image (FD2000, FD4000) (default)\n"
"  -4, --d4m                  use a 4 MB disk image (FD4000 only)\n"
"  -r, --read                 read the image from device to PC (default)\n"
/* "  -w, --write                write the image from PC to device (NOT SUPPORTED)\n" */
"\n"
);
}

static void hint(char *s)
{
    fprintf(stderr, "Try `%s' -h for more information.\n", s);
}

static int process_cmdline(int argc, char *argv[], struct cmdline_parameter_s *cmdline)
{
    static const struct option longopts[] =
    {
        { "help"       , no_argument      , NULL, 'h' },
        { "version"    , no_argument      , NULL, 'V' },
        { "adapter"    , required_argument, NULL, '@' },
        { "d1m"        , no_argument      , NULL, '1' },
        { "d2m"        , no_argument      , NULL, '2' },
        { "d4m"        , no_argument      , NULL, '4' },
        { "read"       , no_argument      , NULL, 'r' },

        /* undocumented */
        { "write"      , no_argument      , NULL, 'w' },
        { NULL         , 0                , NULL, 0   }
    };

    static const char shortopts[] ="hV@:124rw";

    int option;

    const char *arg;
    unsigned char drive;

    while((option = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1)
    {
        switch(option)
        {
            case 'h': help();
                      return 0;
            case 'V': printf("cbmformat %s\n", OPENCBM_VERSION);
                      return 0;
            case '@': if (cmdline->adapter == NULL)
                          cmdline->adapter = cbmlibmisc_strdup(optarg);
                      else
                      {
                          fprintf(stderr, "--adapter/-@ given more than once.\n");
                          hint(argv[0]);
                          return 1;
                      }
                      break;
            case '1': if ((cmdline->imagetype != IMAGETYPE_UNKNOWN) && (cmdline->imagetype != IMAGETYPE_D1M))
                      {
                          fprintf(stderr, "multiple drivetype specified, don't know what to do\n");
                          hint(argv[0]);
                          return 1;
                      }
                      cmdline->imagetype = IMAGETYPE_D1M;
                      cmdline->imagetype_text = "D1M";
                      break;
            case '2': if ((cmdline->imagetype != IMAGETYPE_UNKNOWN) && (cmdline->imagetype != IMAGETYPE_D2M))
                      {
                          fprintf(stderr, "multiple drivetype specified, don't know what to do\n");
                          hint(argv[0]);
                          return 1;
                      }
                      cmdline->imagetype = IMAGETYPE_D2M;
                      cmdline->imagetype_text = "D2M";
                      break;
            case '4': if ((cmdline->imagetype != IMAGETYPE_UNKNOWN) && (cmdline->imagetype != IMAGETYPE_D4M))
                      {
                          fprintf(stderr, "multiple drivetype specified, don't know what to do\n");
                          hint(argv[0]);
                          return 1;
                      }
                      cmdline->imagetype = IMAGETYPE_D4M;
                      cmdline->imagetype_text = "D4M";
                      break;

            case 'r': if ((cmdline->operationtype != OPERATIONTYPE_UNKNOWN) &&  (cmdline->operationtype != OPERATIONTYPE_READ))
                      {
                          fprintf(stderr, "operationtype (-r, -w) both specified, don't know what to do\n");
                          hint(argv[0]);
                          return 1;
                      }
                      cmdline->operationtype = OPERATIONTYPE_READ;
                      cmdline->operationtype_text = "READ";
                      break;
            case 'w': if ((cmdline->operationtype != OPERATIONTYPE_UNKNOWN) &&  (cmdline->operationtype != OPERATIONTYPE_WRITE))
                      {
                          fprintf(stderr, "operationtype (-r, -w) both specified, don't know what to do\n");
                          hint(argv[0]);
                          return 1;
                      }
                      cmdline->operationtype = OPERATIONTYPE_WRITE;
                      cmdline->operationtype_text = "WRITE";
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
    cmdline->drive = drive;

    cmdline->image_filename = cbmlibmisc_strdup(argv[optind++]);

#if 0
    fprintf(stderr, "Options recognized:\n");
    if (cmdline->adapter)        fprintf(stderr, "- adapter = '%s'\n", cmdline->adapter);
    if (cmdline->image_filename) fprintf(stderr, "- image   = '%s'\n", cmdline->image_filename);
    if (cmdline->drive >= 0)     fprintf(stderr, "- drive   = %d\n",   cmdline->drive);

    switch (cmdline->imagetype) {
        case IMAGETYPE_D1M:
            fprintf(stderr, "- D1M (CMD FD2000, FD4000)");
            break;
        case IMAGETYPE_D2M:
            fprintf(stderr, "- D2M (CMD FD2000, FD4000)");
            break;
        case IMAGETYPE_D4M:
            fprintf(stderr, "- D4M (CMD FD4000)");
            break;
        case IMAGETYPE_UNKNOWN:
            fprintf(stderr, "- unknown DxM!");
            break;
    }
    fprintf(stderr, " (%s)\n", cmdline->imagetype_text);

    switch (cmdline->operationtype) {
        case OPERATIONTYPE_READ:
            fprintf(stderr, "- READ");
            break;
        case OPERATIONTYPE_WRITE:
            fprintf(stderr, "- WRITE (not supported)");
            return 1;
            break;
        case OPERATIONTYPE_UNKNOWN:
            fprintf(stderr, "- unknown operation");
            break;
    }
    fprintf(stderr, " (%s)\n", cmdline->operationtype_text);
#endif

    if (cmdline->operationtype == OPERATIONTYPE_UNKNOWN) {
        /* no operation was provided, assume 'read' */
        cmdline->operationtype = OPERATIONTYPE_READ;
        cmdline->operationtype_text = "READ";
    }

    if (cmdline->imagetype == IMAGETYPE_UNKNOWN) {
        /* no drive type was provided, assume 'D2M' */
        cmdline->imagetype = IMAGETYPE_D2M;
        cmdline->imagetype_text = "D2M";
    }

    return 0;
}

static int perform_action_read_data(struct cmdline_parameter_s *cmdline, CBM_FILE fd, const struct imageparameter_s * const imageparameter, FILE *image)
{
    int rv = 1;

    do {
        int track;
        int side;
        int sector;

        unsigned char jobresult[1];

        for (track = 0; track <= imageparameter->max_track; ++track) {
            for (side = 0; side < 2; ++side) {

                fprintf(stderr, "\rReading Track %d / Side %d ...                 ", track, side);
                fflush(stderr);

#if DUMMY_DRIVER_ACCESS
                rv = 2;
#else
                /* read the next track into the cache of the FDx000 */
                rv = cbm_exec_command(fd, cmdline->drive, "U4", 2);
#endif

                if (rv < 0) {
                    fprintf(stderr, "\n\n Error executing U4, rv = %d!\n",rv);
                    goto error;
                }

#if DUMMY_DRIVER_ACCESS
                jobresult[0] = 0;
                rv = 0;
#else
                /* read the return value from the drive code */
                rv = cbm_download(fd, cmdline->drive, 0x0509, jobresult, sizeof jobresult);
#endif
                if (rv < 0) {
                    fprintf(stderr, "\n\nError getting JOBRESULT, rv = %d!\n",rv);
                    goto error;
                }

                if (jobresult[0]) {
                    fprintf(stderr, "\n\nJOBRESULT indicated an error: 0x%02x!\n", (unsigned int) jobresult[0]);
                    goto error;
                }

                /* read the sectors from the track cache */
                for (sector = 1; sector <= imageparameter->max_sector; ++sector) {
                    char buffer[256];

                    fprintf(stderr, "\rReading Track %d / Side %d / Sector %d ...     ", track, side, sector);
                    fflush(stderr);

#if DUMMY_DRIVER_ACCESS
                    memset(buffer, 0xAA, sizeof buffer);
                    buffer[0] = track;
                    buffer[1] = side;
                    buffer[2] = sector;
                    buffer[3] = 0;
                    buffer[0xff] = 0xff;

                    rv = 0x100;
#else
                    rv = cbm_download(fd, cmdline->drive, 0x5000 + (sector - 1) * 0x100, buffer, sizeof buffer);
#endif

                    if (rv < 0) {
                        fprintf(stderr, "\n\nError reading data, rv = %d!\n", rv);
                        goto error;
                    }

                    if (rv != sizeof buffer) {
                        fprintf(stderr, "\n\nShort read getting data, rv = %d!\n", rv);
                        goto error;
                    }

                    fwrite(buffer, sizeof buffer, 1, image);
                }
            }
        }

        for (sector = 0; sector < imageparameter->extra_blocks_at_end; ++sector) {
            static unsigned char buffer[256] = { 0 };
            fwrite(buffer, sizeof buffer, 1, image);
        }

        rv = 0;
    } while (0);

error:
    return rv;
}

static int perform_action_read(struct cmdline_parameter_s *cmdline, CBM_FILE fd, const struct imageparameter_s * const imageparameter)
{
    int rv = 1;

    FILE * image = NULL;

    do {
        image = fopen(cmdline->image_filename, "w");
        if (!image) {
            fprintf(stderr, "Could not open file '%s' for writing, aborting...\n", cmdline->image_filename);
            break;
        }

#if DUMMY_DRIVER_ACCESS
#else
        rv = cbm_upload(fd, cmdline->drive, 0x500, fdx000_turboread, sizeof fdx000_turboread);
#endif
        if (rv < 0) {
            fprintf(stderr, "\n\n Error uploading code, rv = %d!\n",rv);
            break;
        }

#if DUMMY_DRIVER_ACCESS
        rv = 2;
#else
        /* init the routine for reading in the FDx000 */
        rv = cbm_exec_command(fd, cmdline->drive, "U3", 2);
#endif
        if (rv < 0) {
            fprintf(stderr, "\n\n Error with U3, rv = %d!\n",rv);
            break;
        }

        rv = perform_action_read_data(cmdline, fd, imageparameter, image);
        if (rv) break;

        rv = 0;

    } while (0);

    if (image) {
        fclose(image);
    }

    return rv;
}

static int perform_action(struct cmdline_parameter_s *cmdline) {
    int rv;
    CBM_FILE fd = CBM_FILE_INVALID;

    do {
#if DUMMY_DRIVER_ACCESS
        rv = 0;
#else
        const char *type_str;

        rv = cbm_driver_open_ex(&fd, cmdline->adapter);
#endif
        if (rv) break;

#if DUMMY_DRIVER_ACCESS
#else
        if (cbm_identify(fd, cmdline->drive, &cmdline->drive_type, &type_str)) {
            fprintf(stderr, "Could not identify drive!\n");
            rv = 1;
            break;
        }

        if (cmdline->drive_type != cbm_dt_fdx000) {
            fprintf(stderr, "Detected a %s, not an FD2000 nor FD4000, aborting\n", type_str);
            rv = 1;
            break;
        }
#endif

        switch (cmdline->operationtype) {
            case OPERATIONTYPE_READ:
                rv = perform_action_read(cmdline, fd, &imageparameter[cmdline->imagetype]);
                break;
            case OPERATIONTYPE_WRITE:
                // @@@TODO not implemented yet
                break;
            case OPERATIONTYPE_UNKNOWN:
                assert("Operationtype unknown should not happen here!");
        }
        if (rv) break;

        rv = 0;
    } while (0);

    if (fd != CBM_FILE_INVALID) {
#if DUMMY_DRIVER_ACCESS
#else
        cbm_driver_close(fd);
#endif
    }
    return rv;
}

int ARCH_MAINDECL main(int argc, char *argv[])
{
    static struct cmdline_parameter_s cmdline = INIT_CMDLINE_PARAMETER;

    int rv;

    do {
        rv = process_cmdline(argc, argv, &cmdline);
        if (rv) break;

        fprintf(stderr, "Performing %s on %s drive #%d with image file '%s'\n",
                cmdline.operationtype_text, cmdline.imagetype_text, cmdline.drive, cmdline.image_filename);

        rv = perform_action(&cmdline);
        if (rv) break;

    } while (0);

    return rv;
}
