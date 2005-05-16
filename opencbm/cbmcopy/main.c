/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2001-2004 Michael Klein <michael.klein@puffin.lb.shuttle.de>
 */

#ifdef SAVE_RCSID
static char *rcsid =
    "@(#) $Id: main.c,v 1.5 2005-05-16 16:20:16 strik Exp $";
#endif

#include <ctype.h>
#include <getopt.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch.h"

#include "opencbm.h"
#include "cbmcopy.h"
#include "inputfiles.h"

/* global, because of signal handler */
static CBM_FILE fd_cbm;

static cbmcopy_severity_e verbosity = sev_info;
static int no_progress = 0;

static void my_message_cb(cbmcopy_severity_e severity,
                          const char *format, ...)
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

static int my_status_cb(int blocks_written)
{
    if(!no_progress) printf("."); fflush(stdout);
    return 0;
}


static void help(const char *prog)
{
    printf(
"Usage: %s [OPTION]... [DRIVE] [FILE]...\n"
"Copy files to a CBM-15[478]1 or compatible drive and vice versa\n"
"\n"
"Options:\n"
"  -r, --read                 transfer 15x1->PC\n"
"                             (default when started as 'cbmread')\n"
"  -w, --write                transfer PC->15x1\n"
"                             (default when started as 'cbmwrite')\n"
"                             -r and -w are mutually exclusive\n"
"\n"
"  -h, --help                 display this help and exit\n"
"  -V, --version              display version information and exit\n"
"  -q, --quiet                quiet output\n"
"  -v, --verbose              control verbosity (repeatedly, up to 3 times)\n"
"  -n, --no-progress          do not display progress information\n"
"\n"
"  -t, --transfer=TRANSFER    set transfermode; valid modes:\n"
"                               serial1 or s1  (slowest)\n"
"                               serial2 or s2\n"
"                               parallel       (fastest)\n"
"                             (can be abbreviated, if unambiguous)\n"
"                             `serial1' should work in any case;\n"
"                             `serial2' won't work if more than one device is\n"
"                             connected to the IEC bus;\n"
"                             `parallel' needs a XP1541/XP1571 cable in addition\n"
"                             to the serial one.\n"
"  -d, --drive-type=TYPE      specify drive type, one of:\n"
"                               1541, 1570, 1571, 1581\n"
"  -a, --address=ADDRESS      override file start address\n"
"  -o, --output=NAME          specifies target name (ASCII, even for writing).\n"
"\n"
"Options for writing:\n"
"  -f, --file-type            specify CBM file type (D,P,S,U)\n"
"  -R, --raw                  skip test for PC64 (.p00) and T64 input file\n"
"\n", prog);
}

static void hint(char *prog)
{
    printf("Try `%s' --help for more information.\n", prog);
}


static void ARCH_SIGNALDECL reset(int dummy)
{
    fprintf(stderr, "\nSIGINT caught X-(  Resetting IEC bus...\n");
    arch_sleep(1);
    cbm_reset(fd_cbm);
    cbm_driver_close(fd_cbm);
    exit(1);
}


extern input_reader cbmwrite_raw;
extern input_reader cbmwrite_pc64;
extern input_reader cbmwrite_t64;


static void char_star_opt_once(const char **arg,
                               const char *long_name,
                               char **argv)
{
    if(*arg)
    {
        my_message_cb(sev_fatal, "%s given more than once.", long_name);
        hint(argv[0]);
        exit(1);
    }
    *arg = optarg;
}

static void copy_filename_without_path(char *dest, const char *src, int length)
{
    char *p;
    int posEndPath = -1;

    // search for backslash (Windows)

    p = strrchr(src, '\\');
    if (p)
    {
        posEndPath = p - src;
    }

    // search for slash (Linux and Windows)

    p = strrchr(src, '/');
    if (p)
    {
        int posTmp = p - src;

        if (posTmp > posEndPath)
        {
            posEndPath = posTmp;
        }
    }
    strncpy(dest, &src[posEndPath+1], length);
    dest[length] = 0;
}


int ARCH_MAINDECL main(int argc, char **argv)
{
    CBM_FILE fd;
    FILE *file;
    char *fname;

    int mode;
    int c;
    unsigned char *filedata;
    size_t filesize;
    unsigned char buf[48];
    int num_entries;
    int num_files;
    int rv;
    int i;
    int write;
    cbmcopy_settings *settings;
    char auto_name[17];
    char auto_type = '\0';
    char output_type = '\0';
    char *tail;
    char *ext;

    unsigned char drive;
    const char *tm = NULL;
    const char *dt = NULL;
    int force_raw = 0;
    int address = -1;
    const char *output_name = NULL;
    const char *address_str = NULL;
    char *fs_name;

    input_reader *readers[] =
    {
        &cbmwrite_raw,   /* must be first, as it is default */
        &cbmwrite_pc64,
        &cbmwrite_t64,
        NULL
    };

    input_reader *rd;

    struct option longopts[] =
    {
        { "help"            , no_argument      , NULL, 'h' },
        { "verbose"         , no_argument      , NULL, 'v' },
        { "quiet"           , no_argument      , NULL, 'q' },
        { "version"         , no_argument      , NULL, 'V' },
        { "no-progress"     , no_argument      , NULL, 'n' },
        { "read"            , no_argument      , NULL, 'r' },
        { "write"           , no_argument      , NULL, 'w' },
        { "transfer"        , required_argument, NULL, 't' },
        { "drive-type"      , required_argument, NULL, 'd' },
        { "file-type"       , required_argument, NULL, 'f' },
        { "output"          , required_argument, NULL, 'o' },
        { "raw"             , no_argument      , NULL, 'R' },
        { "address"         , no_argument      , NULL, 'a' },
        { NULL              , 0                , NULL, 0   }
    };

    const char shortopts[] ="hVqvrwnt:d:f:o:Ra:";

    if(NULL == (tail = strrchr(argv[0], '/')))
    {
        tail = argv[0];
    }
    else
    {
        tail++;
    }
    if(strcmp(tail, "cbmread") == 0)
    {
        mode = 'r'; /* read */
    }
    else if(strcmp(tail, "cbmwrite") == 0)
    {
        mode = 'w'; /* write */
    }
    else
    {
        mode = EOF; /* mode must be given later */
    }

    settings = cbmcopy_get_default_settings();

    /* loop over cmd line opts */
    while((c=getopt_long(argc, argv, shortopts, longopts, NULL)) != -1)
    {
        switch(c)
        {
            case 'h': /* --help */
                help(argv[0]);
                return 0;
            case 'V': /* --version */
                printf("cbmcopy %s\n", VERSION);
                return 0;
            case 'q': /* -- quiet */
                if(verbosity > sev_fatal)
                    verbosity--;
                break;
            case 'v': /* --verbose */
                if(verbosity < sev_debug)
                    verbosity++;
                break;
            case 'n': /* --no-progress */
                no_progress = 1;
                break;

            case 'r': /* --read */
            case 'w': /* --write */
                if(mode != EOF)
                {
                    my_message_cb(sev_fatal, "-r/-w given more than once");
                    hint(argv[0]);
                    return 1;
                }
                mode = c;
                break;

            case 't': /* --transfer */
                char_star_opt_once(&tm, "--transfer", argv);
                break;
            case 'd': /* --drive-type */
                char_star_opt_once(&dt, "--drive-type", argv);
                break;
            case 'o': /* --output */
                char_star_opt_once(&output_name, "--output", argv);
                break;
            case 'f': /* --file-type */
                output_type = (char) toupper(*optarg);
                break;
            case 'R': /* --raw */
                force_raw = 1;
                break;
            case 'a': /* override-address */
                char_star_opt_once(&address_str, "--address", argv);
                break;
            default : /* unknown */
                hint(argv[0]);
                return 1;
        }
    }

    /* check -r/-w */
    switch(mode)
    {
        case 'r' :
            write = 0;
            break;
        case 'w' :
            write = 1;
            break;
        default:
            my_message_cb(sev_fatal,
                          "-r or -w must be given when started as `%s'",
                          argv[0]);
            hint(argv[0]);
            return 1;
    }

    /* check transfer mode */
    settings->transfer_mode = cbmcopy_get_transfer_mode_index(tm);
    if(settings->transfer_mode < 0)
    {
        my_message_cb(sev_fatal, "Unknown transfer mode: %s", tm);
        return 1;
    }

    /* check device type */
    if(dt)
    {
        const struct
        {
            const char *str;
            enum cbm_device_type_e type;
        }
        *p, types[] =
        {
            { "1541", cbm_dt_cbm1541 }, { "1571", cbm_dt_cbm1571 },
            { "1570", cbm_dt_cbm1570 }, { "1581", cbm_dt_cbm1581 },
            { NULL  , cbm_dt_unknown }
        };

        for(p = types; p->str && strcmp(dt, p->str); p++)
            ; /* nothing */

        if(!p->str)
        {
            my_message_cb(sev_fatal, "Unknown drive type: %s", dt);
            return 1;
        }
        settings->drive_type = p->type;
    }

    /* check CBM file type */
    if(output_type)
    {
        if(write)
        {
            if(strchr("DSPU", output_type) == NULL)
            {
                my_message_cb(sev_fatal, "Invalid file type : %c", output_type);
            }
        }
        else
        {
            my_message_cb(sev_warning, "--file-type ignored");
        }
    }

    /* check load address override */
    if(address_str)
    {
        address = strtol(address_str, &tail, 0);
        if(*tail || address < 0 || address > 0xffff)
        {
            my_message_cb(sev_fatal, "--address invalid: %s", address_str);
            hint(argv[0]);
            return 1;
        }
    }

    /* first non-option is device number */
    if(optind == argc)
    {
        my_message_cb(sev_fatal, "%s: No drive number given", argv[0]);
        hint(argv[0]);
        return 1;
    }

    drive = (unsigned char) strtol(argv[optind], &tail, 0);
    if(drive < 8 || drive > 11 || *tail)
    {
        my_message_cb(sev_fatal, "invalid drive: `%s'", argv[optind]);
        return 1;
    }

    /* remaining args are file names */
    num_files = argc - optind - 1;

    if(num_files == 0)
    {
        my_message_cb(sev_fatal, "%s: No files?", argv[0]);
        hint(argv[0]);
        return 1;
    }

    /* more than one file name given, avoid -o option */
    if(num_files > 1 && output_name)
    {
        my_message_cb(sev_fatal, "--output requires exactly one file name");
        return 1;
    }

    rv = cbm_driver_open( &fd, 0 );

    if(0 == rv)
    {
        fd_cbm = fd;
        signal( SIGINT, reset );

        while(++optind < argc)
        {
            fname = argv[optind];
            if(write)
            {
                rd = readers[0];

                file = fopen(fname, "rb");
                if(file)
                {
                    num_entries = 0;
                    if(!force_raw)
                    {
                        /* try to detect file format */
                        for(i = 1; readers[i] && !num_entries; i++)
                        {
                            num_entries =
                                readers[i]->probe( file, fname, my_message_cb );
                            if(num_entries)
                                rd = readers[i];
                        }
                    }
                    if(!num_entries) num_entries = 1; /* raw file */

                    for(i = 0; i < num_entries; i++)
                    {
                        my_message_cb( sev_debug,
                                       "processing entry %d from %s",
                                       i, fname );
                        if(rd->read(file, fname, i,
                                    auto_name, &auto_type,
                                    &filedata, &filesize, my_message_cb ) == 0)
                        {
                            if(output_name)
                            {
                                copy_filename_without_path(buf, output_name, 16);
                                cbm_ascii2petscii(buf);
                            }
                            else
                            {
                                /* no charset conversion */
                                copy_filename_without_path(buf, auto_name, 16);
                            }
                            strcat(buf, ",x");
                            buf[strlen(buf)-1] =
                                output_type ? output_type : auto_type;
                            strcat(buf, ",W");

                            my_message_cb( sev_info,
                                           "writing %s -> %s", fname, buf );

                            if(address >= 0 && filesize > 1)
                            {
                                filedata[0] = address % 0x100;
                                filedata[1] = address / 0x100;

                                my_message_cb( sev_debug, 
                                               "override address: $%02x%02x",
                                               filedata[1], filedata[0] );

                            }
                            if(cbmcopy_write_file(fd, settings, drive,
                                                  buf, strlen(buf),
                                                  filedata, filesize,
                                                  my_message_cb,
                                                  my_status_cb) == 0)
                            {
                                rv = cbm_device_status( fd, drive,
                                                        buf, sizeof(buf) );
                                my_message_cb( rv ?  sev_warning : sev_info,
                                               "%s", buf );
                            }
                            if(filedata)
                            {
                                free(filedata);
                            }
                        }
                        else
                        {
                            my_message_cb( sev_warning,
                                           "error processing entry %d from %s",
                                           i, fname );
                        }
                    }
                }
                else
                {
                    my_message_cb( sev_warning,
                                   "warning could not read %s: %s",
                                   fname, arch_strerror(arch_get_errno()) );
                }
            }
            else
            {
                // check if there is a path specified. If so, do not copy that
                copy_filename_without_path(buf, fname, 16);
                cbm_ascii2petscii(buf);

                if(output_name)
                {
                    fs_name = arch_strdup(output_name);
                }
                else
                {
                    for(tail = fname; *tail && *tail != ','; tail++);

                    ext = "prg"; /* default */

                    if(*tail)
                    {
                        tail++;
                        switch(*tail)
                        {
                            case 'D': ext = "del"; break;
                            case 'S': ext = "seq"; break;
                            case 'U': ext = "usr"; break;
                        }
                    }
                    fs_name = malloc(strlen(fname) + strlen(ext) + 2);
                    if(fs_name) sprintf(fs_name, "%s.%s", fname, ext);
                }

                if(fs_name)
                {
                    for(tail = fs_name; *tail; tail++)
                    {
                        if(*tail == '/') *tail = '_';
                    }
                }
                else
                {
                    /* should not happen... */
                    cbm_driver_close( fd );
                    my_message_cb(sev_fatal, "Out of memory");
                    exit(1);
                }

                my_message_cb( sev_info, "reading %s -> %s", buf, fs_name );

                if(cbmcopy_read_file(fd, settings, drive, buf, strlen(buf),
                                     &filedata, &filesize,
                                     my_message_cb, my_status_cb) == 0)
                {
                    rv = cbm_device_status( fd, drive, buf, sizeof(buf) );
                    my_message_cb( rv ? sev_warning : sev_info, "%s", buf );

                    file = fopen(fs_name, "wb");
                    if(file)
                    {
                        if(filedata)
                        {
                            if(address >= 0 && filesize > 1)
                            {
                                filedata[0] = address % 0x100;
                                filedata[1] = address / 0x100;

                                my_message_cb( sev_debug, 
                                               "override address: $%02x%02x",
                                               filedata[1], filedata[0] );
                            }
                            if(fwrite(filedata, filesize, 1, file) != 1)
                            {
                                my_message_cb(sev_warning,
                                              "could not write %s: %s",
                                              fs_name, arch_strerror(arch_get_errno()));
                            }
                        }
                        fclose(file);
                    }
                    else
                    {
                        my_message_cb(sev_warning,
                                      "could not open %s: %s",
                                      fs_name, arch_strerror(arch_get_errno()));
                    }

                    if(filedata)
                    {
                        free(filedata);
                    }
                }
                else
                {
                    my_message_cb(sev_warning, "error reading %s", buf);
                }
                if(fs_name)
                {
                    free(fs_name);
                }
            }
        }
        cbm_driver_close( fd );

        if(rv)
        {
            my_message_cb(sev_warning, "there was at least one error" );
        }
    }

    return rv;
}
