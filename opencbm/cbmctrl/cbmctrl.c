/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2004 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Modifications for cbm4win Copyright 2001-2004 Spiro Trikaliotis
*/

#ifdef SAVE_RCSID
static char *rcsid =
    "@(#) $Id: cbmctrl.c,v 1.11 2006-02-25 10:27:54 strik Exp $";
#endif

#include "opencbm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
typedef int (*mainfunc)(CBM_FILE fd, char *argv[]);

#include "arch.h"

static const unsigned char prog_tdchange[] = {
#include "tdchange.inc"
};


static int do_help(CBM_FILE fd, char *argv[]);

/*
 * Output version information
 */
static int do_version(CBM_FILE fd, char *argv[])
{
    printf("cbmctrl version " OPENCBM_VERSION ", built on " __DATE__ " at " __TIME__ "\n");

    return 0;
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
    return cbm_listen(fd, arch_atoc(argv[0]), arch_atoc(argv[1]));
}

/*
 * Simple wrapper for talk
 */
static int do_talk(CBM_FILE fd, char *argv[])
{
    return cbm_talk(fd, arch_atoc(argv[0]), arch_atoc(argv[1]));
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
    return cbm_open(fd, arch_atoc(argv[0]), arch_atoc(argv[1]), argv[2], strlen(argv[2]));
}

/*
 * Simple wrapper for close
 */
static int do_close(CBM_FILE fd, char *argv[])
{
    return cbm_close(fd, arch_atoc(argv[0]), arch_atoc(argv[1]));
}

/*
 * display device status w/ PetSCII conversion
 */
static int do_status(CBM_FILE fd, char *argv[])
{
    unsigned char buf[40];
    unsigned char unit;
    int rv;

    unit = arch_atoc(argv[0]);

    rv = cbm_device_status(fd, unit, buf, sizeof(buf));
    fprintf(stderr, "%s", cbm_petscii2ascii(buf));

    return (rv == 99) ? 1 : 0;
}

/*
 * send device command
 */
static int do_command(CBM_FILE fd, char *argv[])
{
    int  rv;

    rv = cbm_listen(fd, arch_atoc(argv[0]), 15);
    if(rv == 0)
    {
        cbm_raw_write(fd, argv[1], strlen(argv[1]));
        rv = cbm_unlisten(fd);
    }
    return rv;
}

/*
 * display directory
 */
static int do_dir(CBM_FILE fd, char *argv[])
{
    unsigned char c, buf[40];
    int rv;
    unsigned char unit;

    unit = arch_atoc(argv[0]);
    rv = cbm_open(fd, unit, 0, "$", strlen("$"));
    if(rv == 0)
    {
        if(cbm_device_status(fd, unit, buf, sizeof(buf)) == 0)
        {
            cbm_talk(fd, unit, 0);
            if(cbm_raw_read(fd, buf, 2) == 2)
            {
                while(cbm_raw_read(fd, buf, 2) == 2)
                {
                    if(cbm_raw_read(fd, buf, 2) == 2)
                    {
                        printf("%u ", buf[0] | (buf[1] << 8));
                        while((cbm_raw_read(fd, &c, 1) == 1) && c)
                        {
                            putchar(cbm_petscii2ascii_c(c));
                        }
                        putchar('\n');
                    }
                }
                cbm_device_status(fd, unit, buf, sizeof(buf));
                fprintf(stderr, "%s", cbm_petscii2ascii(buf));
            }
            cbm_untalk(fd);
        }
        else
        {
            fprintf(stderr, "%s", cbm_petscii2ascii(buf));
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
    unsigned char unit;
    unsigned short c;
    int addr, count, i, rv = 0;
    char *tail, buf[32], cmd[7];
    FILE *f;

    unit = arch_atoc(argv[0]);

    addr = strtol(argv[1], &tail, 0);
    if(addr < 0 || addr > 0xffff || *tail)
    {
        arch_error(0, 0, "invalid address: %s", argv[1]);
        return 1;
    }

    count = strtol(argv[2], &tail, 0);
    if((count + addr) > 0x10000 || *tail)
    {
        arch_error(0, arch_get_errno(), "invalid byte count %s", argv[2]);
        return 1;
    }

    if(argv[3] && strcmp(argv[3],"-") != 0)
    {
        /* a filename (other than simply "-") was given, open that file */

        f = fopen(argv[3], "wb");
    }
    else
    {
        /* no filename was given, open stdout in binary mode */

        f = arch_fdopen(arch_fileno(stdout), "wb");

        /* set binary mode for output stream */

        arch_setbinmode(arch_fileno(stdout));
    }

    if(!f)
    {
        arch_error(0, arch_get_errno(), "could not open output file: %s",
              (argv[3] && strcmp(argv[3], "-") != 0) ? argv[3] : "stdout");
        return 1;
    }

    for(i = 0; (rv == 0) && (i < count); i+=32)
    {
        c = count - i;
        if(c > 32) 
        {
            c = 32;
        }
        sprintf(cmd, "M-R%c%c%c", addr%256, addr/256, c);
        cbm_listen(fd, unit, 15);
        rv = cbm_raw_write(fd, cmd, 6) == 6 ? 0 : 1;
        cbm_unlisten(fd);
        if(rv == 0)
        {
            addr += c;
            cbm_talk(fd, unit, 15);
            rv = cbm_raw_read(fd, buf, c) == c ? 0 : 1;
            cbm_untalk(fd);
            if(rv == 0)
            {
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
    unsigned char unit;
    int addr;
    size_t size;
    char *tail, *fn;
    unsigned char addr_buf[2];
    static unsigned char buf[65537];
    FILE *f;

    unit = arch_atoc(argv[0]);

    addr = strtoul(argv[1], &tail, 0);
    if(addr < -1 || addr > 0xffff || *tail)
    {
        arch_error(0, 0, "invalid address: %s", argv[1]);
        return 1;
    }

    if(!argv[2] || strcmp(argv[2], "-") == 0 || strcmp(argv[2], "") == 0)
    {
        fn = "(stdin)";
        f = stdin;

        // set binary mode for input stream

        arch_setbinmode(arch_fileno(stdin));
    }
    else
    {
        off_t filesize;

        fn = argv[2];
        f = fopen(argv[2], "rb");
        if(f == NULL)
        {
            arch_error(0, arch_get_errno(), "could not open %s", fn);
            return 1;
        }
        if(arch_filesize(argv[2], &filesize))
        {
            arch_error(0, arch_get_errno(), "could not stat %s", fn);
            return 1;
        }
    }

    if(addr == -1)
    {
        /* read address from file */
        if(fread(addr_buf, 2, 1, f) != 1)
        {
            arch_error(0, arch_get_errno(), "could not read %s", fn);
            if(f != stdin) fclose(f);
            return 1;
        }

        /* don't assume a particular endianess, although the cbm4linux
         * package is only i386 for now  */
        addr = addr_buf[0] | (addr_buf[1] << 8);
    }

    size = fread(buf, 1, sizeof(buf), f);
    if(ferror(f))
    {
        arch_error(0, 0, "could not read %s", fn);
        if(f != stdin) fclose(f);
        return 1;
    }
    else if(size == 0 && feof(f))
    {
        arch_error(0, 0, "no data: %s", fn);
        if(f != stdin) fclose(f);
        return 1;
    }

    if(addr + size > 0x10000)
    {
        arch_error(0, 0, "program too big: %s", fn);
        if (f != stdin) fclose(f);
        return 1;
    }

    if(f != stdin) fclose(f);

    return (cbm_upload(fd, unit, addr, buf, size) == (int)size) ? 0 : 1;
}

/*
 * identify connected devices
 */
static int do_detect(CBM_FILE fd, char *argv[])
{
    unsigned int num_devices;
    unsigned char device;
    const char *type_str;

    num_devices = 0;

    for( device = 8; device < 16; device++ )
    {
        enum cbm_device_type_e device_type;
        if( cbm_identify( fd, device, &device_type, &type_str ) == 0 )
        {
            enum cbm_cable_type_e cable_type;
            const char *cable_str = "(cannot determine cable type)";
 
            num_devices++;

            if ( cbm_identify_xp1541( fd, device, &device_type, &cable_type ) == 0 )
            {
                switch (cable_type)
                {
                case cbm_ct_none:
                    cable_str = "";
                    break;

                case cbm_ct_xp1541:
                    cable_str = "(XP1541)";
                    break;

                case cbm_ct_unknown:
                default:
                    break;
                }
            }
            printf( "%2d: %s %s\n", device, type_str, cable_str );
        }
    }
    arch_set_errno(0);
    return num_devices > 0 ? 0 : 1;
}

/*
 * wait until user changes the disk
 */
static int do_change(CBM_FILE fd, char *argv[])
{
    unsigned char unit;
    int rv;

    unit = arch_atoc(argv[0]);

    do
    {
        rv = cbm_upload(fd, unit, 0x500, prog_tdchange, sizeof(prog_tdchange));
    
        if (rv != sizeof(prog_tdchange))
        {
            rv = 1;
            break;
        }

        cbm_exec_command(fd, unit, "U3:", 0);
        cbm_iec_release(fd, IEC_ATN | IEC_DATA | IEC_CLOCK | IEC_RESET);

        /*
         * Now, wait for the drive routine to signal its starting
         */
        cbm_iec_wait(fd, IEC_DATA, 1);

        /*
         * Now, wait until CLOCK is high, too, which tells us that
         * a new disk has been successfully read.
         */
        cbm_iec_wait(fd, IEC_CLOCK, 1);

        /*
         * Signal: We recognized this
         */
        cbm_iec_set(fd, IEC_ATN);

        /*
         * Wait for routine ending.
         */
        cbm_iec_wait(fd, IEC_CLOCK, 0);

        /*
         * Release ATN again
         */
        cbm_iec_release(fd, IEC_ATN);

    } while (0);

    return rv;
}

struct prog
{
    int      need_driver;
    char    *name;
    mainfunc prog;
    int      req_args_min;
    int      req_args_max;
    char    *arglist;
    char    *shorthelp_text;
    char    *help_text;
};

static struct prog prog_table[] =
{
    {0, "--help"  , do_help    , 0, 1, "[<command>]",
        "output this help screen",
        "This command outputs some help information for cbmctrl.\n\n"
        "<command> is the (optional) command to get information about.\n\n"
        "If you use it without parameter, it outputs a list of all\n"
        "available commands." },

    {0, "-h"      , do_help    , 0, 1, "",
        "same as --help",
        "for more info, use \"cbmctrl --help --help\"." },

    {0, "--version",do_version , 0, 0, "",
        "output version information",
        "This command just outputs the version number and\n"
        "build date of cbmctrl." },

    {0, "-V"      , do_version , 0, 0, "",
        "same as --version",
        "for more info, use \"cbmctrl --help --version\"." },

    {1, "listen"  , do_listen  , 2, 2, "<device> <secadr>",
        "perform a listen on the IEC bus",
        "Output a listen command on the IEC bus.\n"
        "<device> is the device number,\n"
        "<secadr> the secondary address to use for this.\n\n"
        "This has to be undone later with an unlisten command." },

    {1, "talk"    , do_talk    , 2, 2, "<device> <secadr>",
        "perform a talk on the IEC bus",
        "Output a talk command on the IEC bus.\n"
        "<device> is the device number,\n"
        "<secadr> the secondary address to use for this.\n\n"
        "This has to be undone later with an untalk command." },

    {1, "unlisten", do_unlisten, 0, 0, "",
        "perform an unlisten on the IEC bus",
        "Undo one or more previous listen commands.\n"
        "This affects all drives." },

    {1, "untalk"  , do_untalk  , 0, 0, "",
        "perform an untalk on the IEC bus",
        "Undo one or more previous talk commands.\n"
        "This affects all drives." },

    {1, "open"    , do_open    , 3, 3, "<device> <secadr> <filename>",
        "perform an open on the IEC bus",
        "Output an open command on the IEC bus.\n"
        "<device> is the device number,\n"
        "<secadr> the secondary address to use for this.\n"
        "<filename> is the name of the file to be opened.\n\n"
        "This has to be undone later with a close command.\n\n"
        "NOTE: You cannot do an open without a filename.\n"
        "      Although a CBM machine (i.e., a C64) allows this,\n"
        "      this is an internal operation to the computer only." },

    {1, "close"   , do_close   , 2, 2, "<device> <secadr>",
        "perform a close on the IEC bus",
        "Undo a previous open command." },

    {1, "status"  , do_status  , 1, 1, "<device>",
        "give the status of the specified drive",
        "This command gets the status (the so-called 'error channel')"
        "of the given drive and outputs it on the screen.\n"
        "<device> is the device number of the drive."
    },

    {1, "command" , do_command , 2, 2, "<device> <cmdstr>",
        "issue a command to the specified drive",
        "This command issues a command to a specific drive.\n"
        "This command is a command that you normally give to\n"
        "channel 15 (i.e., N: to format a drive, V: to validate, etc.).\n\n"
        "<device> is the device number of the drive.\n\n"
        "<cmdstr> is the command to execute in the drive.\n"
        "NOTE: You have to give the commands in upper-case letters.\n"
        "      Lower case will NOT work!" },

    {1, "dir"     , do_dir     , 1, 1, "<device>",
        "output the directory of the disk in the specified drive",
        "This command gets the directory of the disk in the drive.\n\n"
        "<device> is the device number of the drive." },

    {1, "download", do_download, 3, 4, "<device> <adr> <count> [<file>]",
        "download memory contents from the floppy drive",
        "With this command, you can get data from the floppy drive memory.\n"
        "<device> is the device number of the drive.\n"
        "<adr>    is the starting address of the memory region to get.\n"
        "         it can be given in decimal or in hex (with a 0x prefix).\n"
        "<count>  is the number of bytes to read.\n"
        "         it can be given in decimal or in hex (with a 0x prefix).\n"
        "<file>   (optional) file name of a file to write the contents to.\n"
        "         If this name is not given (or it is a dash ('-'), the\n"
        "         contents will be written to stdout, normally the console.\n\n" 
        "Example:\n"
        " cbmctrl download 8 0xc000 0x4000 1541ROM.BIN\n"
        " * reads the 1541 ROM (from $C000 to $FFFF) from drive 8 into 1541ROM.BIN" },

    {1, "upload"  , do_upload  , 2, 3, "<device> <adr> [<file>]",
        "upload memory contents to the floppy drive",
        "With this command, you can write data to the floppy drive memory.\n"
        "<device> is the device number of the drive.\n"
        "<adr>    is the starting address of the memory region to write to.\n"
        "         it can be given in decimal or in hex (with a 0x prefix).\n"
        "<file>   (optional) file name of a file to read the values from.\n"
        "         If this name is not given (or it is a dash ('-'), the\n"
        "         contents will be read from stdin, normally the console."
        "Example:\n"
        " cbmctrl upload 8 0x500 BUFFER2.BIN\n"
        " * writes the file BUFFER2.BIN to drive 8, address $500." },

    {1, "reset"   , do_reset   , 0, 0, "",
        "reset all drives on the IEC bus",
        "This command performs a (physical) reset of all drives on the IEC bus." },

    {1, "detect"  , do_detect  , 0, 0, "",
        "detect all drives on the IEC bus",
        "This command tries to detect all drives on the IEC bus.\n"
        "For this, this command access all possible drives and tries to read\n"
        "some bytes from its memory. If a drive is detected, its name is output.\n"
        "Additionally, this routine determines if the drive is connected via a\n"
        "parallel cable (XP1541 companion cable)." },

    {1, "change"  , do_change  , 1, 1, "<device>",
        "wait for a disk to be changed in the specified drive",
        "This command waits for a disk to be changed in the specified drive.\n\n"
        "For this, it makes the following assumptions:\n\n"
        "* there is already a disk in the drive.\n"
        "* that disk will be removed and replaced by another disk.\n"
        "* we do not want to return from this command until the disk is completely\n"
        "  inserted and ready to be read/written.\n\n"
        "Because of this, just opening the drive and closing it again (without\n"
        "actually removing the disk) will not work in most cases." },

    {0, NULL,NULL}
};

static struct prog *find_main(char *name)
{
    int i;

    for(i=0; prog_table[i].name; i++)
    {
        if(strcmp(name, prog_table[i].name) == 0)
        {
            return &prog_table[i];
        }
    }
    return NULL;
}

/*
 * Output a help screen
 */
static int do_help(CBM_FILE fd, char *argv[])
{
    int i;

    do_version(fd, argv);

    printf("\n");

    if (*argv == 0)
    {
        for(i=0; prog_table[i].prog; i++)
        {
            printf("  %-9s %s\n", prog_table[i].name, prog_table[i].shorthelp_text);
        }

        printf("\nFor more information on a specific command, try --help <COMMAND>.\n");

    }
    else
    {
        struct prog *p;

        p = find_main(argv[0]);

        if (p)
        {
            printf(" cbmctrl %s %s\n\n  %s\n\n%s\n", p->name, p->arglist, p->shorthelp_text, p->help_text);
        }
        else
        {
            printf(" Nothing known about \"cbmctrl %s\".\n", argv[0]);
        }
    }


    return 0;
}

int ARCH_MAINDECL main(int argc, char *argv[])
{
    struct prog *p;

    p = argc < 2 ? NULL : find_main(argv[1]);
    if(p)
    {
        if((p->req_args_min <= argc-2) && (p->req_args_max >= argc-2))
        {
            CBM_FILE fd;
            int rv;

            if(p->need_driver)
                rv = cbm_driver_open(&fd, 0);
            else
                rv = 0;

            if(rv == 0)
            {
                rv = p->prog(fd, &argv[2]) != 0;
                if(rv && arch_get_errno())
                {
                    arch_error(0, arch_get_errno(), "%s", argv[1]);
                }

                if(p->need_driver)
                    cbm_driver_close(fd);
            }
            else
            {
                if(arch_get_errno())
                {
                    arch_error(0, arch_get_errno(), "%s", cbm_get_driver_name(0));
                }
                rv = 1;
            }
            return rv;
        }
        else
        {
            fprintf(stderr, "wrong number of arguments:\n\n  %s %s %s\n",
                        argv[0], argv[1], p->arglist);
        }
    }
    else
    {
        printf("invalid command. For info on possible commands, try the --help parameter.\n\n");
    }
    return 2;
}
