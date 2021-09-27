/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2021 Spiro Trikaliotis
*/

#include "opencbm.h"

#include "arch.h"
#include "libmisc.h"


#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char uint8_t;

static unsigned char downloadrom[] = {
# include "downloadrom.inc"
};

static void help()
{
    printf(
"Usage: demodownloadrom [OPTION]... [SOURCE]\n"
"Download floppy rom of a CBM-1541 or compatible drive\n"
"\n"
"Options:\n"
"  -h, --help                display this help and exit\n"
"  -V, --version             display version information and exit\n"
"  -@, --adapter=plugin:bus  tell OpenCBM which backend plugin and bus to use\n"
"\n"
"  -s, --startblock=BLOCK    set start block (high byte of address)\n"
"  -c, --count=COUNT         number of pages to read\n"
"  -d, --drive=DRIVE         drive number (default: 8)\n"
"\n"
);
}

static void hint(char *s)
{
    fprintf(stderr, "Try `%s' --help for more information.\n", s);
}

static void show_monkey(unsigned int c)
{
    // const static char monkey[]={"¸,ø¤*º°´`°º*¤ø,¸"};     // for fast moves
    // const static char monkey[]={"\\|/-"};    // from cbmcopy
    // const static char monkey[]={"-\\|/"};    // from libtrans (reversed)
    // const static char monkey[]={"\\-/|"};    // from cbmcopy  (reversed)
    // const static char monkey[]={"-/|\\"};       // from libtrans
    // const static char monkey[]={",;:!^*Oo"};// for fast moves
    const static char monkey[]={",oO*^!:;"};// for fast moves

    c %= sizeof(monkey) - 1;
    fprintf(stderr, (c != 0) ? "\b%c" : "\b.%c" , monkey[c]);
    fflush(stderr);
}

int ARCH_MAINDECL main(int argc, char *argv[])
{
    int       option;
    char     *adapter    = NULL;
    uint8_t   startblock = 0xff;
    uint8_t   count      = 1;
    uint8_t   drv        = 8;
    CBM_FILE  fd;
    int       rv;
    char     *outputfilename = NULL;
    FILE     *fileout        = NULL;
	int       currentblock;

    struct option longopts[] =
    {
        { "help"       , no_argument      , NULL, 'h' },
        { "version"    , no_argument      , NULL, 'V' },
        { "adapter"    , required_argument, NULL, '@' },
        { "startblock" , required_argument, NULL, 's' },
        { "count"      , required_argument, NULL, 'c' },
        { "drive"      , required_argument, NULL, 'd' },
        { NULL         , 0                , NULL, 0   }
    };

    const char shortopts[] ="hV@:s:c:d:";

    optind = 0;

    while((option = getopt_long(argc, argv, shortopts, longopts, NULL)) != EOF)
    {
        char * endptr = NULL;

        switch(option)
        {
            case 'h': help();
                      return 0;
            case 'V': printf("d64copy %s\n", OPENCBM_VERSION);
                      return 0;
            case 's': startblock = (uint8_t) strtol(optarg, &endptr, 0);
                      if (endptr != NULL && endptr[0] != 0) {
                          fprintf(stderr, "startblock has extra data '%s'\n", endptr);
                          exit(1);
                      }
                      break;
            case 'c': count = (uint8_t) strtol(optarg, &endptr, 0);
                      if (endptr != NULL && endptr[0] != 0) {
                          fprintf(stderr, "count has extra data '%s'\n", endptr);
                          exit(1);
                      }
                      break;
            case 'd': drv = arch_atoc(optarg);
                      break;
            case '@': if (adapter == NULL)
                          adapter = cbmlibmisc_strdup(optarg);
                      else
                      {
                          fprintf(stderr, "--adapter/-@ given more than once.");
                          hint(argv[0]);
                          exit(1);
                      }
                      break;
            case 0:   break; // needed for --no-warp
            default : hint(argv[0]);
                      return 1;
        }
    }

    // skip the options that are already processed
    argc -= optind;
    argv += optind;

    outputfilename = argc > 0 ? argv[0] : 0;

    printf("Start at %04X, length %04X on drive #%u.\n", startblock << 8, count << 8, (unsigned int) drv);

    if (outputfilename == NULL) {
        fprintf(stderr, "no output file given, aborting...\n");
        return 1;
    }
    printf("Output file is: %s.\n", outputfilename);

    do {
        downloadrom[3] = startblock;
        downloadrom[4] = count;

        rv = cbm_driver_open_ex(&fd, adapter);
        if (rv != 0) {
            if (arch_get_errno()) {
                arch_error(0, arch_get_errno(), "%s", cbm_get_driver_name_ex(adapter));
            }
            else {
                fprintf(stderr, "An error occurred opening OpenCBM, aborting...\n");
            }
            break;
        }

        arch_set_errno(0);

        cbm_upload(fd, drv, 0x500, downloadrom, sizeof(downloadrom));

        fileout = fopen(outputfilename, "wb");
        if (fileout == 0) {
            fprintf(stderr, "Could not open file '%s' for writing.\n", outputfilename);
        }

        for (currentblock = 0; currentblock < count; currentblock++) {
            static char buffer[256];

            show_monkey(currentblock);

            cbm_exec_command(fd, drv, "U3", 0);
            cbm_download(fd, drv, 0x0300, buffer, sizeof(buffer));

            fwrite(buffer, sizeof(buffer), 1, fileout);
        }
    } while (0);

    if (fileout) {
        fclose(fileout);
    }

    if (fd) {
        cbm_driver_close(fd);
    }

    return 0;
}
