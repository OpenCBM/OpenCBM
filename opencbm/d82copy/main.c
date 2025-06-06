/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2001 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Copyright 2004-2007 Spiro Trikaliotis
 *  Copyright 2011-2011 Thomas Winkler
*/

#include "opencbm.h"
#include "d82copy.h"

#include "arch.h"
#include "libmisc.h"

#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* setable via command line */
static d82copy_severity_e verbosity = sev_warning;
static int no_progress = 0;

/* other globals */
static CBM_FILE fd_cbm;


static int cbm_address_extract_drive_and_medium(char * arg, unsigned char * unit)
{
    char * endptr = NULL;
    long drive;
    int  medium;

    errno = 0;
    drive = strtol(arg, &endptr, 10);

    if (errno || drive < 0 || drive > 15) {
        // not a CBM drive address
        return 0;
    }

    // assume medium default of 0: this is the old behaviour
    medium = 0;

    if (*endptr == ':') {
        ++endptr;
        // there seems to be a medium, process that
        switch (*endptr) {
            case '0':
                medium = 0;
                ++endptr;
                break;

            case '1':
                medium = 1;
                ++endptr;
                break;

            default:
                return 0;
        }
    }

    if (*endptr) {
        // there is still something following, this is not a valid CBM address
        return 0;
    }

    *unit = (drive & 0x7F) | (medium ? 0x80 : 0);

    return 1;
}


static void help()
{
    printf(
"Usage: d82copy [OPTION]... [SOURCE] [TARGET]\n"
"Copy .d82 and .d80 disk images to a CBM-8250 or compatible drive and vice versa\n"
"\n"
"Options:\n"
"  -h, --help                display this help and exit\n"
"  -V, --version             display version information and exit\n"
"  -@, --adapter=plugin:bus  tell OpenCBM which backend plugin and bus to use\n"
"  -q, --quiet               quiet output\n"
"  -v, --verbose             control verbosity (repeatedly, up to 3 times)\n"
"  -n, --no-progress         do not display progress information\n"
"\n"
"  -s, --start-track=TRACK   set start track\n"
"  -e, --end-track=TRACK     set end track (start <= end <= 77 / 154)\n"
"\n"
"  -t, --transfer=TRANSFER   set transfermode; valid modes:\n"
"                              auto (default)\n"
"                              original       (slowest)\n"
"                            `auto' tries to determine the best option.\n"
"\n"
"  -i, --interleave=VALUE    set interleave value; ignored when reading with\n"
"                            warp mode; default values are:\n"
"\n"
"                              original     22\n"
"\n"
"                            INTERLEAVE is ignored when reading with warp mode;\n"
"                            if data transfer is very slow, increasing this\n"
"                            value may help.\n"
"\n"
"  -w, --warp                enable warp mode; this is not possible if\n"
"                            TRANSFER is set to `original'\n"
"                            This is the default if transfer is not `original'.\n"
"\n"
"      --no-warp             disable warp mode; this is the default if\n"
"                            TRANSFER is set to `original'.\n"
"\n"
"  -b, --bam-only            BAM-only copy; only allocated blocks are copied;\n"
"\n"
"  -B, --bam-save            save BAM-only copy; this is like the `-b' option\n"
"                            but copies always the entire directory track.\n"
"\n"
"  -d, --drive-type=TYPE     specify drive type:\n"
"                              8050\n"
"                              8250 or 1001\n"
"\n"
"  -r, --retry-count=COUNT   set retry count\n"
"\n"
"  -E, --error-map=WHEN      control whether the error map is appended.\n"
"                            possible values for WHEN are (abbreviations\n"
"                            available):\n"
"                              always\n"
"                              on_errors     (default)\n"
"                              never\n"
"\n"
"  -1, -one-sided            one-sided disk transfer (.d80) for CBM-8050 drive\n"
"\n"
"  -2, --two-sided           two-sided disk transfer (.d82): Requires CBM-8250\n"
"                            or SFD-1001 diskette drive.\n"
"\n"
);
}

static void hint(char *s)
{
    fprintf(stderr, "Try `%s' --help for more information.\n", s);
}

static void my_message_cb(int severity, const char *format, ...)
{
    va_list args;

    static const char *severities[4] =
    {
        "Fatal",
        "Warning",
        "Info",
        "Debug"
    };

    if(verbosity >= severity)
    {
        fprintf(stderr, "[%s] ", severities[severity]);
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        fprintf(stderr, "\n");
    }
}

static int my_status_cb(d82copy_status status)
{
    static char trackmap[MAX_SECTORS+1];
    static int last_track;
    char *s;
    char *d;

    static const char bs2char[] =
    {
        ' ', '.', '-', '?', '*'
    };

    if(status.track == 0)
    {
        last_track = 0;
        return 0;
    }

    if(no_progress)
    {
        return 0;
    }

    if(last_track != status.track)
    {
        if(last_track)
        {
            printf("\r%2d: %-24s               \n", last_track, trackmap);
        }

        for(s = status.bam[status.track-1], d = trackmap; *s; s++, d++)
        {
            *d = bs2char[(int)*s];
        }
        *d = '\0';
        last_track = status.track;
    }

    trackmap[status.sector] =
        bs2char[(status.read_result ||
                 status.write_result) ? bs_error : bs_copied];

    printf("\r%2d: %-24s%3d%%  %4d/%d", status.track, trackmap,
           100 * status.sectors_processed / status.total_sectors,
           status.sectors_processed, status.total_sectors);

    fflush(stdout);
    return 0;
}


static void ARCH_SIGNALDECL reset(int dummy)
{
    CBM_FILE fd_cbm_local;

    /*
     * remember fd_cbm, and make the global one invalid
     * so that no routine can call a cbm_...() routine
     * once we have cancelled another one
     */
    fd_cbm_local = fd_cbm;
    fd_cbm = CBM_FILE_INVALID;

    fprintf(stderr, "\nSIGINT caught X-(  Resetting IEC bus...\n");
#ifdef LIBD82COPY_DEBUG
    printDebugLibD82Counters(my_message_cb);
#endif
    d82copy_cleanup();
    cbm_reset(fd_cbm_local);
    cbm_driver_close(fd_cbm_local);
    exit(1);
}

int ARCH_MAINDECL main(int argc, char *argv[])
{
    d82copy_settings *settings = d82copy_get_default_settings();

    char *tm = NULL;
    char *src_arg;
    char *dst_arg;

    char *adapter = NULL;

    int  option;
    int  rv = 1;
    int  l;

    unsigned char src_unit;
    unsigned char dst_unit;
    int src_is_cbm;
    int dst_is_cbm;

    struct option longopts[] =
    {
        { "help"       , no_argument      , NULL, 'h' },
        { "version"    , no_argument      , NULL, 'V' },
        { "adapter"    , required_argument, NULL, '@' },
        { "warp"       , no_argument      , NULL, 'w' },
        { "no-warp"    , no_argument      , &settings->warp, 0 },
        { "quiet"      , no_argument      , NULL, 'q' },
        { "verbose"    , no_argument      , NULL, 'v' },
        { "no-progress", no_argument      , NULL, 'n' },
        { "interleave" , required_argument, NULL, 'i' },
        { "start-track", required_argument, NULL, 's' },
        { "end-track"  , required_argument, NULL, 'e' },
        { "transfer"   , required_argument, NULL, 't' },
        { "bam-only"   , no_argument      , NULL, 'b' },
        { "bam-save"   , no_argument      , NULL, 'B' },
        { "drive-type" , required_argument, NULL, 'd' },
        { "retry-count", required_argument, NULL, 'r' },
        { "one-sided"  , no_argument      , NULL, '1' },
        { "two-sided"  , no_argument      , NULL, '2' },
        { "error-map"  , required_argument, NULL, 'E' },
        { NULL         , 0                , NULL, 0   }
    };

    const char shortopts[] ="hVwqbBt:i:s:e:d:r:12vnE:@:";

    while((option = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1)
    {
        switch(option)
        {
            case 'h': help();
                      return 0;
            case 'V': printf("d82copy %s\n", OPENCBM_VERSION);
                      return 0;
            case 'w': settings->warp = 1;
                      break;
            case 'q': if(verbosity > 0) verbosity--;
                      break;
            case 'v': verbosity++;
                      break;
            case 'n': no_progress = 1;
                      break;
            case 'i': settings->interleave = arch_atoc(optarg);
                      break;
            case 's': settings->start_track = atoi(optarg);
                      break;
            case 'e': settings->end_track = atoi(optarg);
                      break;
            case 't': tm = optarg;
                      break;
            case 'b': settings->bam_mode = bm_allocated;
                      break;
            case 'B': settings->bam_mode = bm_save;
                      break;
            case 'd': if(strcmp(optarg, "8050") == 0)
                      {
                          settings->drive_type = cbm_dt_cbm8050;
                      }
                      else if(strcmp(optarg, "8250") == 0)
                      {
                          settings->drive_type = cbm_dt_cbm8250;
                      }
                      else if(strcmp(optarg, "1001") == 0)
                      {
                          settings->drive_type = cbm_dt_sfd1001;
                      }
                      else
                      {
                          settings->drive_type = atoi(optarg) != 0 ?
                              cbm_dt_cbm8250 : cbm_dt_cbm8050;
                      }
                      break;
            case 'r': settings->retries = atoi(optarg);
                      break;
            case '1': settings->two_sided = 0;
                      break;
            case '2': settings->two_sided = 1;
                      break;
            case 'E': l = strlen(optarg);
                      if(strncmp(optarg, "always", l) == 0)
                      {
                          settings->error_mode = em_always;
                      }
                      else if(strncmp(optarg, "on_errors", l) == 0)
                      {
                          settings->error_mode = em_on_error;
                      }
                      else if(strncmp(optarg, "never", l) == 0)
                      {
                          settings->error_mode = em_never;
                      }
                      else
                      {
                          hint(argv[0]);
                          return 1;
                      }
                      break;
            case '@': if (adapter == NULL)
                          adapter = cbmlibmisc_strdup(optarg);
                      else
                      {
                          my_message_cb(sev_fatal, "--adapter/-@ given more than once.");
                          hint(argv[0]);
                          exit(1);
                      }
                      break;
            case 0:   break; // needed for --no-warp
            default : hint(argv[0]);
                      return 1;
        }
    }

    settings->transfer_mode = d82copy_get_transfer_mode_index(tm);
    if(settings->transfer_mode < 0)
    {
        char *modes = d82copy_get_transfer_modes();
        char *m;

        fprintf(stderr, "Unknown transfer mode: %s\nAvailable modes:\n", tm);

        for(m = modes; *m; m+=(strlen(m)+1))
        {
            fprintf(stderr, "  %s\n", m);
        }

        free(modes);
        return 1;
    }

    my_message_cb(3, "transfer mode is %d", settings->transfer_mode );

    if(optind + 2 != argc)
    {
        fprintf(stderr, "Usage: %s [OPTION]... [SOURCE] [TARGET]\n", argv[0]);
        hint(argv[0]);
        return 1;
    }

    src_arg = argv[optind];
    dst_arg = argv[optind+1];

    src_is_cbm = cbm_address_extract_drive_and_medium(src_arg, &src_unit);
    dst_is_cbm = cbm_address_extract_drive_and_medium(dst_arg, &dst_unit);

    if(src_is_cbm == dst_is_cbm)
    {
        my_message_cb(0, "either source or target must be a CBM drive");
        return 1;
    }

    if(cbm_driver_open_ex(&fd_cbm, adapter) == 0)
    {
        /*
         * If the user specified auto transfer mode, find out
         * which transfer mode to use.
         */
        settings->transfer_mode =
            d82copy_check_auto_transfer_mode(fd_cbm,
                settings->transfer_mode,
                src_is_cbm ? src_unit : dst_unit);

        my_message_cb(3, "decided to use transfer mode %d", settings->transfer_mode );

        arch_set_ctrlbreak_handler(reset);

        if(src_is_cbm)
        {
            rv = d82copy_read_image(fd_cbm, settings, src_unit, dst_arg,
                    my_message_cb, my_status_cb);
        }
        else
        {
            rv = d82copy_write_image(fd_cbm, settings, src_arg, dst_unit,
                    my_message_cb, my_status_cb);
        }

        if(!no_progress && rv >= 0)
        {
            printf("\n%d blocks copied.\n", rv);
        }

        cbm_driver_close(fd_cbm);
        rv = 0;
    }
    else
    {
        arch_error(0, arch_get_errno(), "%s", cbm_get_driver_name_ex(adapter));
    }

    cbmlibmisc_strfree(adapter);
    free(settings);

    return rv;
}
