/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2001 Michael Klein <michael.klein@puffin.lb.shuttle.de>
 *  Modifications for cbm4win Copyright 2001-2004 Spiro Trikaliotis
*/

#ifdef SAVE_RCSID
static char *rcsid =
    "$Id: cbmctrl.c,v 1.3 2004-11-16 19:54:33 strik Exp $";
#endif

#include "opencbm.h"

// #define USE_BUFFERED_READ 1

#ifndef WIN32
        #include <ctype.h>
        #include <errno.h>
        #include <error.h>
#endif
        #include <stdio.h>
        #include <stdlib.h>
        #include <string.h>
#ifndef WIN32
        #include <sys/stat.h>
#endif

typedef int (*mainfunc)(CBM_FILE fd, char *argv[]);

#ifdef WIN32
    #include "unixcompat.h"
    #include <fcntl.h>
#endif

UCHAR atoc(const char *str)
{
    return (UCHAR) atoi(str);
}

/*
 * Simple wrapper for reset
 */
static int do_reset(CBM_FILE fd, char *argv[])
{
    return cbm_reset(fd);
}

/*
 * Simple wrapper for listen
 */
static int do_listen(CBM_FILE fd, char *argv[])
{
    return cbm_listen(fd, atoc(argv[0]), atoc(argv[1]));
}

/*
 * Simple wrapper for talk
 */
static int do_talk(CBM_FILE fd, char *argv[])
{
    return cbm_talk(fd, atoc(argv[0]), atoc(argv[1]));
}

/*
 * Simple wrapper for unlisten
 */
static int do_unlisten(CBM_FILE fd, char *argv[])
{
    return cbm_unlisten(fd);
}

/*
 * Simple wrapper for untalk
 */
static int do_untalk(CBM_FILE fd, char *argv[])
{
    return cbm_untalk(fd);
}

/*
 * Simple wrapper for open
 */
static int do_open(CBM_FILE fd, char *argv[])
{
    return cbm_open(fd, atoc(argv[0]), atoc(argv[1]), argv[2], strlen(argv[2]));
}

/*
 * Simple wrapper for close
 */
static int do_close(CBM_FILE fd, char *argv[])
{
    return cbm_close(fd, atoc(argv[0]), atoc(argv[1]));
}

/*
 * display device status w/ PetSCII conversion
 */
static int do_status(CBM_FILE fd, char *argv[])
{
    unsigned char buf[40];
    UCHAR unit;
    int rv;

    unit = atoc(argv[0]);

    rv = cbm_device_status(fd, unit, buf, sizeof(buf));
    fprintf(stderr, cbm_petscii2ascii(buf));

    return (rv == 99) ? 1 : 0;
}

/*
 * send device command
 */
static int do_command(CBM_FILE fd, char *argv[])
{
    int  rv;

    rv = cbm_listen(fd, atoc(argv[0]), 15);
    if (rv == 0)
    {
        cbm_raw_write(fd, argv[1], strlen(argv[1]));
        rv = cbm_unlisten(fd);
    }
    return rv;
}

#ifdef USE_BUFFERED_READ

static int buffered_read(CBM_FILE HandleDevice, void *Buffer, size_t Count)
{
    // quick 'n' dirty

#define BUFFER_SIZE 256


    static size_t already_read = 0;
    static char buffer[BUFFER_SIZE];

    if (Count > already_read)
    {
        // we are in troubles... ;-)
        int newcount;
            
        newcount = cbm_raw_read(HandleDevice, &buffer[already_read], BUFFER_SIZE-already_read);

        if (newcount >= 0)
        {
            already_read += newcount;
        }

        if (already_read == 0)
        {
            return -1;
        }
    }

    if (Count > already_read)
    {
        Count = already_read;
    }

    if (Count)
    {
        memcpy(Buffer, buffer, Count);

        already_read -= Count;

        if (already_read)
        {
            memmove(buffer, &buffer[Count], already_read);
        }
    }

    return Count;
}

#else // #ifdef USE_BUFFERED_READ

    #define buffered_read(_x_, _y_, _z_) cbm_raw_read(_x_, _y_, _z_)

#endif // #ifdef USE_BUFFERED_READ


/*
 * display directory
 */
static int do_dir(CBM_FILE fd, char *argv[])
{
    unsigned char c, buf[40];
    int rv;
    UCHAR unit;

    unit = atoc(argv[0]);
    rv = cbm_open(fd, unit, 0, "$", strlen("$"));
    if (rv == 0)
    {
        if (cbm_device_status(fd, unit, buf, sizeof(buf)) == 0)
        {
            cbm_talk(fd, unit, 0);
            if (buffered_read(fd, buf, 2) == 2)
            {
                while (buffered_read(fd, buf, 2) == 2)
                {
                    if (buffered_read(fd, buf, 2) == 2)
                    {
                        printf("%u ", buf[0] | (buf[1] << 8));
                        while ((buffered_read(fd, &c, 1) == 1) && c)
                        {
                            putchar(cbm_petscii2ascii_c(c));
                        }
                        putchar('\n');
                    }
                }
                cbm_device_status(fd, unit, buf, sizeof(buf));
                fprintf(stderr, cbm_petscii2ascii(buf));
            }
            cbm_untalk(fd);
        }
        else
        {
            fprintf(stderr, cbm_petscii2ascii(buf));
        }
        cbm_close(fd, unit, 0);
    }
    return rv;
}

/*
 * read device memory, dump to stdout or a file
 */
static int do_download(CBM_FILE fd, char *argv[])
{
    UCHAR unit;
    USHORT c;
    int addr, count, i, rv = 0;
    char *tail, buf[32], cmd[7];
    FILE *f;

    unit = atoc(argv[0]);

    addr = strtol(argv[1], &tail, 0);
    if (addr < 0 || addr > 0xffff || *tail)
    {
        unix_error(0, 0, "invalid address: %s", argv[1]);
        return 1;
    }

    count = strtol(argv[2], &tail, 0);
    if ((count + addr) > 0x10000 || *tail)
    {
        unix_error(0, get_errno(), "invalid byte count %s", argv[2]);
        return 1;
    }

    if (argv[3] && strcmp(argv[3],"-") != 0)
    {
        // a filename (other than simply "-") was given, open that file

        f = fopen(argv[3], "wb");
    }
    else
    {
        // no filename was given, open stdout in binary mode

        f = fdopen(fileno(stdout), "wb");

#ifdef WIN32

        // set binary mode for output stream

        _setmode(fileno(stdout), _O_BINARY);
#endif
    }

    if (!f)
    {
        unix_error(0, get_errno(), "could not open output file", argv[3] ? argv[3] : "stdout");
        return 1;
    }

    for (i = 0; (rv == 0) && (i < count); i+=32)
    {
        c = count - i;
        if (c > 32) 
        {
            c = 32;
        }
        sprintf(cmd, "M-R%c%c%c", addr%256, addr/256, c);
        cbm_listen(fd, unit, 15);
        rv = cbm_raw_write(fd, cmd, 6) == 6 ? 0 : 1;
        cbm_unlisten(fd);
        if (rv == 0) {
            addr += c;
            cbm_talk(fd, unit, 15);
            rv = cbm_raw_read(fd, buf, c) == c ? 0 : 1;
            cbm_untalk(fd);
            if (rv == 0) {
                fwrite(buf, 1, c, f);
            }
        }
    }
    fclose(f);
    return rv;
}

/*
 * load binary data from file into device memory
 */
static int do_upload(CBM_FILE fd, char *argv[])
{
    UCHAR unit;
    int addr;
    size_t size;
    char *tail, *fn;
    unsigned char addr_buf[2];
    unsigned char buf[65537];
    struct _stat statrec;
    FILE *f;

    unit = atoc(argv[0]);

    addr = strtoul(argv[1], &tail, 0);
    if (addr < -1 || addr > 0xffff || *tail) {
        unix_error(0, 0, "invalid address: %s", argv[1]);
        return 1;
    }

    if (!argv[2] || strcmp(argv[2], "-") == 0 || strcmp(argv[2], "") == 0)
    {
        fn = "(stdin)";
        f = stdin;

#ifdef WIN32

        // set binary mode for input stream

        _setmode(fileno(stdin), _O_BINARY);
#endif
    }
    else
    {
        fn = argv[2];
        f = fopen(argv[2], "rb");
        if(f == NULL)
        {
            unix_error(0, errno, "could not open %s", fn);
            return 1;
        }
        if(stat(argv[2], &statrec))
        {
            unix_error(0, errno, "could not stat %s", fn);
            return 1;
        }
    }

    if (addr == -1)
    {
        /* read address from file */
        if (fread(addr_buf, 2, 1, f) != 1) {
            unix_error(0, get_errno(), "could not read %s", fn);
            if (f != stdin) fclose(f);
            return 1;
        }

        /* don't assume a particular endianess, although the cbm4linux 
         * package is only i386 for now  */
        addr = addr_buf[0] | (addr_buf[1] << 8);
    }

    size = fread(buf, 1, sizeof(buf), f);
    if(ferror(f))
    {
        unix_error(0, 0, "could not read %s", fn);
        if (f != stdin) fclose(f);
        return 1;
    }
    else if (size == 0 && feof(f))
    {
        unix_error(0, 0, "no data: %s", fn);
        if(f != stdin) fclose(f);
        return 1;
    }

    if(addr + size > 0x10000)
    {
        unix_error(0, 0, "program too big: %s", fn);
        if (f != stdin) fclose(f);
        return 1;
    }

    if (f != stdin) fclose(f);

    return (cbm_upload(fd, unit, addr, buf, size) == (int)size) ? 0 : 1;
}

/*
 * identify connected devices
 */
static int do_detect(CBM_FILE fd, char *argv[])
{
    unsigned int num_devices;
    UCHAR device;
    const char *type_str;

    num_devices = 0;

    for (device = 8; device < 16; device++)
    {
        if (cbm_identify(fd, device, NULL, &type_str) == 0)
        {
            num_devices++;
            printf( "%2d: %s\n", device, type_str );
        }
    }
    set_errno(0);
    return num_devices > 0 ? 0 : 1;
}

struct prog
{
    char    *name;
    mainfunc prog;
    int      req_args_min;
    int      req_args_max;
    char    *arglist;
};

static struct prog prog_table[] =
{
    {"listen"  , do_listen  , 2, 2, "<device> <secadr>"               },
    {"talk"    , do_talk    , 2, 2, "<device> <secadr>"               },
    {"unlisten", do_unlisten, 0, 0, ""                                },
    {"untalk"  , do_untalk  , 0, 0, ""                                },
    {"open"    , do_open    , 3, 3, "<device> <secadr> <filename>"    },
    {"close"   , do_close   , 2, 2, "<device> <secadr>"               },
    {"status"  , do_status  , 1, 1, "<device>"                        },
    {"command" , do_command , 2, 2, "<device> <cmdstr>"               },
    {"dir"     , do_dir     , 1, 1, "<device>"                        },
    {"download", do_download, 3, 4, "<device> <adr> <count> [<file>]" },
    {"upload"  , do_upload  , 2, 3, "<device> <adr> [<file>]"         },
    {"reset"   , do_reset   , 0, 0, ""                                },
    {"detect"  , do_detect  , 0, 0, ""                                },
    {NULL,NULL}
};

static struct prog *find_main(char *name)
{
    int i;

    for (i=0; prog_table[i].name; i++) {
        if (strcmp(name, prog_table[i].name) == 0) {
            return &prog_table[i];
        }
    }
    return NULL;
}


int __cdecl main(int argc, char *argv[])
{
    struct prog *p;
    int i;

    p = argc < 2 ? NULL : find_main(argv[1]);
    if (p)
    {
        if ((p->req_args_min <= argc-2) && (p->req_args_max >= argc-2))
        {
            CBM_FILE fd;
            int rv;

            rv = cbm_driver_open(&fd, 0);
            
            if (rv == 0)
            {
                rv = p->prog(fd, &argv[2]) != 0;
                if (rv && get_errno())
                {
                    unix_error(0, get_errno(), "%s", argv[1]);
                }
                cbm_driver_close(fd);
            }
            else
            {
                if (get_errno())
                {
                    unix_error(0, get_errno(), "%s", cbm_get_driver_name(0));
                }
                rv = 1;
            }
            return rv;
        }
        else
        {
            unix_error(0, get_errno(), "wrong number of arguments:\n\n  %s %s %s\n",
                        argv[0], argv[1], p->arglist);
        }
    }
    else
    {
        printf("invalid command, available ones are:\n\n");
        for (i=0; prog_table[i].prog; i++) {
            printf("  %s %s\n", prog_table[i].name, prog_table[i].arglist);
        }
    }
    return 2;
}
