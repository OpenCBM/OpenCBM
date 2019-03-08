/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2018 Spiro Trikaliotis
 */

#include "opencbm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>

#include "arch.h"
#include "libmisc.h"

#if 0

typedef
enum {
    PA_UNSPEC = 0,
    PA_PETSCII,
    PA_RAW
} PETSCII_RAW;

//! struct to remember the general options to the program
typedef struct {
    int argc;    //!< a (modifieable) copy of the number of arguments, as given to main()
    char **argv; //!< a (modifieable) copy of the argument list, as given to main()
    int ownargv; //!< remember: This is the original argv (=0), or this is a malloc()ed copy of it (=1)
    int error;   //!< there was an error in processing the options (=1), or not (=0)
    int help;    //!< option: the user requested help for the specified command
    int version; //!< option: print version information
    char *adapter; //!< option: an explicit adapter was specified
    PETSCII_RAW petsciiraw; //!< option: The user requested PETSCII or RAW, or nothing
} OPTIONS;

typedef int (*mainfunc)(CBM_FILE fd, OPTIONS * const options);


static const unsigned char prog_tdchange[] = {
#include "tdchange.inc"
};


/*
 * Output version information
 */
static int do_version(CBM_FILE fd, OPTIONS * const options)
{
    printf("cbmctrl version " OPENCBM_VERSION ", built on " __DATE__ " at " __TIME__ "\n");

    return 0;
}

static int check_if_parameters_ok(OPTIONS * const options)
{
    if (options->argc != 0)
    {
        fprintf(stderr, "Extra parameter, aborting...\n");
        return 1;
    }
    return 0;
}

static int get_argument_char(OPTIONS * const options, unsigned char *where)
{
    if (options->argc > 0)
    {
        *where = arch_atoc(options->argv[0]);
        options->argv++;
        options->argc--;
        return 0;
    }
    else
    {
        fprintf(stderr, "Not enough parameters, aborting!\n");
        return 1;
    }
}

static int
get_argument_string(OPTIONS * const options, char *where[], unsigned int *len)
{
    if (options->argc > 0)
    {
        if (where)
           *where = options->argv[0];

        if (len)
            *len = strlen(*where);

        options->argv++;
        options->argc--;
        return 0;
    }
    else
    {
        fprintf(stderr, "Not enough parameters, aborting!\n");
        return 1;
    }
}

static int get_argument_file_for_write(OPTIONS * const options, FILE **f)
{
    char *filename = NULL;

    assert(f);

    if (options->argc > 0)
    {
        get_argument_string(options, &filename, NULL);

        if (filename && strcmp(filename, "-") == 0)
            filename = NULL;
    }

    if (check_if_parameters_ok(options))
        return 1;

    if(filename != NULL)
    {
        /* a filename (other than simply "-") was given, open that file */

        *f = fopen(filename, "wb");
    }
    else
    {
        /* no filename was given, open stdout in binary mode */

        *f = arch_fdopen(arch_fileno(stdout), "wb");

        /* set binary mode for output stream */

        if (*f != NULL)
            arch_setbinmode(arch_fileno(stdout));
    }

    if(*f == NULL)
    {
        arch_error(0, arch_get_errno(), "could not open output file: %s",
              (filename != 0) ? filename : "stdout");

        return 1;
    }

    return 0;
}

static int get_argument_file_for_read(OPTIONS * const options, FILE **f, char **fn)
{
    char *filename = NULL;

    assert(f);

    if (options->argc > 0)
    {
        get_argument_string(options, &filename, NULL);

        if (filename && strcmp(filename, "-") == 0)
            filename = NULL;
    }

    if (check_if_parameters_ok(options))
        return 1;

    if(filename == NULL)
    {
        filename = "(stdin)";
        *f = stdin;

        // set binary mode for input stream

        arch_setbinmode(arch_fileno(stdin));
    }
    else
    {
        off_t filesize;

        *f = fopen(filename, "rb");
        if(*f == NULL)
        {
            arch_error(0, arch_get_errno(), "could not open %s", filename);
            return 1;
        }
        if(arch_filesize(filename, &filesize))
        {
            arch_error(0, arch_get_errno(), "could not stat %s", filename);
            return 1;
        }
    }

    if (fn)
        *fn = filename;

    return 0;
}

static int
process_individual_option(OPTIONS * const options, const char short_options[], struct option long_options[])
{
    static int firstcall = 1;
    int option;

    if (firstcall)
    {
        firstcall = 0;

        optind = 0;

        --options->argv;
        ++options->argc;
    }


    option = getopt_long(options->argc, options->argv, short_options, long_options, NULL);

    if (option == EOF)
    {
        // skip the options that are already processed
        //optind --;

        options->argc -= optind;
        options->argv += optind;
    }

    return option;
}


static int
skip_options(OPTIONS * const options)
{
    if (process_individual_option(options, "+", NULL) != EOF)
        return 1;
    else
        return 0;
}

static int hex2val(const char ch)
{
    if (ch>='0' && ch<='9')
        return ch - '0';

    if (ch>='A' && ch<='F')
        return ch - 'A' + 10;

    if (ch>='a' && ch<='f')
        return ch - 'a' + 10;

    fprintf(stderr,
        (ch==0) ? "Not enough digits for hex value given\n"
                : "Unknown hex character '%c'.\n", ch);
    return -1;
}

static int
process_specific_byte_parameter(char *string, int stringlen, PETSCII_RAW petsciiraw)
{
    int size = 0;
    char *pread = string;
    char *pwrite = pread;

    while (*pread != 0)
    {
        char ch = *pread++;

        if (ch == '%')
        {
            ch = *pread++;

            if (ch != '%')
            {
                int val = hex2val(ch);
                int val2;

                if (val < 0)
                    return -1;

                val2 = hex2val(*pread++);

                if (val2 < 0)
                    return -1;

                ch = (val << 4) | val2;
            }

            *pwrite++ = ch;
            size++;
        }
        else
        {
            if (petsciiraw == PA_PETSCII)
                ch = cbm_ascii2petscii_c(ch);

            *pwrite++ = ch;
            size++;
        }
    }

    *pwrite = 0;

    return size;
}

static char *
string_concat(const char *string1, int len1, const char *string2, int len2)
{
    char *buffer;

    if ((string1 != NULL) && (len1 == 0))
        len1 = strlen(string1);

    if ((string2 != NULL) && (len2 == 0))
        len2 = strlen(string2);

    buffer = calloc(1, len1 + len2 + 1);

    if (buffer)
    {
        if (string1)
            memcpy(buffer, string1, len1);

        if (string2)
            memcpy(buffer + len1, string2, len2);
    }

    return buffer;
}

static int
get_extended_argument_string(int extended,
                             OPTIONS * const options,
                             char *string[], unsigned int *stringlen)
{
    int rv;
    char *commandline = NULL;
    char *extended_commandline = NULL;
    unsigned int commandline_len = 0;
    unsigned int extended_commandline_len = 0;

    rv = get_argument_string(options, &commandline, &commandline_len);

    if (rv)
        return 1;

    if (extended)
    {
        int n = process_specific_byte_parameter(commandline, commandline_len, options->petsciiraw);

        if (n < 0)
            return 1;

        commandline_len = n;
    }
    else
    {
        // only convert ASCII -> PETSCII if we do not have extended syntax
        // (with extended syntax, this is done "on the fly" while converting
        // the % - style values into characters.)

        if (options->petsciiraw == PA_PETSCII)
            cbm_ascii2petscii(commandline);
    }

    // now, check if there are more command-line parameters

    if (options->argc > 0)
    {
        char *p;

        // get memory for the additional data

        // get 2 byte more than we have parameters, as we will append
        // \r and \0 at the end of the buffer!

        extended_commandline = malloc(options->argc + 1);

        if (extended_commandline == NULL)
        {
            fprintf(stderr, "Not enough memory for all parameters, aborting...\n");
            return 1;
        }

        // write data in the memory

        p = extended_commandline;

        while (--options->argc >= 0)
        {
            char *tail;
            long c;

            c = strtol(options->argv[0], &tail, 0);

            if(c < 0 || c > 0xff || *tail)
            {
                arch_error(0, 0, "invalid byte: %s", options->argv[0]);
                return 1;
            }
            *p++ = (char) c;
            extended_commandline_len++;

            options->argv++;
        }

        ++options->argc;

        // end the string with a \0. This is not really needed, but
        // convenient when debugging.

        *p   = 0;

    }

    *string = string_concat(commandline, commandline_len, extended_commandline, extended_commandline_len);
    *stringlen = commandline_len + extended_commandline_len;

    if (extended_commandline)
        free(extended_commandline);

    if (check_if_parameters_ok(options))
        return 1;

    return 0;
}

/*
 * Simple wrapper for lock
 */
static int do_lock(CBM_FILE fd, OPTIONS * const options)
{
    int rv = skip_options(options);

    rv = rv || check_if_parameters_ok(options);

    if (rv == 0)
        cbm_lock(fd);

    return rv;
}

/*
 * Simple wrapper for unlock
 */
static int do_unlock(CBM_FILE fd, OPTIONS * const options)
{
    int rv = skip_options(options);

    rv = rv || check_if_parameters_ok(options);

    if (rv == 0)
        cbm_unlock(fd);

    return rv;
}

/*
 * Simple wrapper for reset
 */
static int do_reset(CBM_FILE fd, OPTIONS * const options)
{
    int rv = skip_options(options);

    rv = rv || check_if_parameters_ok(options);

    if (rv == 0)
        rv = cbm_reset(fd);

    return rv;
}

/*
 * Simple wrapper for clk
 */
static int do_iec_clk(CBM_FILE fd, OPTIONS * const options)
{
    int rv = skip_options(options);

    rv = rv || check_if_parameters_ok(options);

    if (rv == 0)
        cbm_iec_set(fd, IEC_CLOCK);

    return rv;
}

/*
 * Simple wrapper for uclk
 */
static int do_iec_uclk(CBM_FILE fd, OPTIONS * const options)
{
    int rv = skip_options(options);

    rv = rv || check_if_parameters_ok(options);

    if (rv == 0)
        cbm_iec_release(fd, IEC_CLOCK);

    return rv;
}

/*
 * Simple wrapper for listen
 */
static int do_listen(CBM_FILE fd, OPTIONS * const options)
{
    int rv;
    unsigned char unit;
    unsigned char secondary;

    rv = skip_options(options);

    rv = rv || get_argument_char(options, &unit);
    rv = rv || get_argument_char(options, &secondary);

    if (rv || check_if_parameters_ok(options))
        return 1;

    return cbm_listen(fd, unit, secondary);
}

/*
 * Simple wrapper for talk
 */
static int do_talk(CBM_FILE fd, OPTIONS * const options)
{
    int rv;
    unsigned char unit;
    unsigned char secondary;

    rv = skip_options(options);

    rv = rv || get_argument_char(options, &unit);
    rv = rv || get_argument_char(options, &secondary);

    if (rv || check_if_parameters_ok(options))
        return 1;

    return cbm_talk(fd, unit, secondary);
}

/*
 * Simple wrapper for unlisten
 */
static int do_unlisten(CBM_FILE fd, OPTIONS * const options)
{
    int rv = skip_options(options);

    rv = rv || check_if_parameters_ok(options);

    if (rv == 0)
        rv = cbm_unlisten(fd);

    return rv;
}

/*
 * Simple wrapper for untalk
 */
static int do_untalk(CBM_FILE fd, OPTIONS * const options)
{
    int rv = skip_options(options);

    rv = rv || check_if_parameters_ok(options);

    if (rv == 0)
        rv = cbm_untalk(fd);

    return rv;
}

/*
 * Simple wrapper for open
 */
static int do_open(CBM_FILE fd, OPTIONS * const options)
{
    int rv;
    unsigned char unit;
    unsigned char secondary;
    char *filename;
    unsigned int filenamelen = 0;

    int extended = 0;
    int c;
    static const char short_options[] = "+e";
    static struct option long_options[] =
    {
        {"extended", no_argument, NULL, 'e'},
        {NULL,       no_argument, NULL, 0  }
    };

    // first of all, process the options given

    while ((c = process_individual_option(options, short_options, long_options)) != EOF)
    {
        switch (c)
        {
        case 'e':
            extended = 1;
            break;

        default:
            return 1;
        }
    }

    rv = get_argument_char(options, &unit);
    rv = rv || get_argument_char(options, &secondary);
    rv = rv || get_extended_argument_string(extended, options, &filename, &filenamelen);

    if (rv)
        return 1;

    rv = cbm_open(fd, unit, secondary, filename, filenamelen);

    free(filename);

    return rv;
}

/*
 * Simple wrapper for close
 */
static int do_close(CBM_FILE fd, OPTIONS * const options)
{
    int rv;
    unsigned char unit;
    unsigned char secondary;

    rv = skip_options(options);

    rv = rv || get_argument_char(options, &unit);
    rv = rv || get_argument_char(options, &secondary);

    if (rv || check_if_parameters_ok(options))
        return 1;

    return cbm_close(fd, unit, secondary);
}

/*
 * read raw data from the IEC bus
 */
static int do_read(CBM_FILE fd, OPTIONS * const options)
{
    int size, rv = 0;
    unsigned char buf[2048];
    FILE *f;

    if (skip_options(options))
        return 1;

    if (get_argument_file_for_write(options, &f))
        return 1;

    /* fill a buffer with up to 64k of bytes from the IEC bus */
    while(0 < (size = cbm_raw_read(fd, buf, sizeof(buf))))
    {
        // if PETSCII was recognized, convert the data before writing

        if (options->petsciiraw == PA_PETSCII)
        {
            int i;
            for (i=0; i < size; i++)
                buf[i] = cbm_petscii2ascii_c(buf[i]);
        }

        /* write that to the file */
        if(size != (int) fwrite(buf, 1, size, f))
        {
            rv=1;   /* error condition from cbm_raw_read */
            break;  /* do fclose(f) before exiting       */
        }
        /* if nobody complained, repeat filling the buffer */
    }

    if(size < 0) rv=1; /* error condition from cbm_raw_read */

    fclose(f);
    return rv;
}

/*
 * write raw data to the IEC bus
 */
static int do_write(CBM_FILE fd, OPTIONS * const options)
{
    char *fn = NULL;
    int size;
    unsigned char buf[2048];
    FILE *f;

    if (skip_options(options))
        return 1;

    if (get_argument_file_for_read(options, &f, &fn))
        return 1;

    /* fill a buffer with up to 64k of bytes from file/console */
    size = fread(buf, 1, sizeof(buf), f);
    /* do this test only on the very first run */
    if(size == 0 && feof(f))
    {
        arch_error(0, 0, "no data: %s", fn);
        if(f != stdin) fclose(f);
        return 1;
    }

    /* as long as no error occurred */
    while( ! ferror(f))
    {
        /* if requested, convert to PETSCII before writing */
        if (options->petsciiraw == PA_PETSCII)
        {
            int i;
            for (i=0; i < size; i++)
                buf[i] = cbm_ascii2petscii_c(buf[i]);
        }

        /* write that to the the IEC bus */
        if(size != cbm_raw_write(fd, buf, size))
        {
            /* exit the loop with another error condition */
            break;
        }

        /* fill a buffer with up to 64k of bytes from file/console */
        size = fread(buf, 1, sizeof(buf), f);
        if(size == 0 && feof(f))
        {
                /* nothing more to read */
            if(f != stdin) fclose(f);
            return 0;
        }
    }

    /* the loop has exited, because of an error, check, which one */
    if(ferror(f))
    {
        arch_error(0, 0, "could not read %s", fn);
    }
    /* else : size number of bytes could not be written to IEC bus */

    if(f != stdin) fclose(f);
    return 1;
}

/*
 * put specified data to the IEC bus
 */
static int do_put(CBM_FILE fd, OPTIONS * const options)
{
    int  rv;
    char *commandline;
    unsigned int commandlinelen = 0;


    int extended = 0;
    int c;
    static const char short_options[] = "+e";
    static struct option long_options[] =
    {
        {"extended", no_argument, NULL, 'e'},
        {NULL,       no_argument, NULL, 0  }
    };

    // first of all, process the options given

    while ((c = process_individual_option(options, short_options, long_options)) != EOF)
    {
        switch (c)
        {
        case 'e':
            extended = 1;
            break;

        default:
            return 1;
        }
    }

    rv = get_extended_argument_string(extended, options, &commandline, &commandlinelen);

    if (rv)
        return 1;

    /* if requested, convert to PETSCII before writing */
    if (options->petsciiraw == PA_PETSCII)
    {
        unsigned int i;
        for (i=0; i < commandlinelen; i++)
            commandline[i] = cbm_ascii2petscii_c(commandline[i]);
    }

    /* write that to the IEC bus */
    rv = cbm_raw_write(fd, commandline, commandlinelen);
    if(rv < 0 || commandlinelen != (unsigned int) rv)
    {
        rv = 1;
    }
    else
    {
        rv = 0;
    }

    if (commandline != NULL)
    {
        free(commandline);
    }

    return rv;
}

/*
 * display device status w/ PetSCII conversion
 */
static int do_status(CBM_FILE fd, OPTIONS * const options)
{
    char buf[40];
    unsigned char unit;
    int rv;

    rv = skip_options(options);

    rv = rv || get_argument_char(options, &unit);

    if (rv || check_if_parameters_ok(options))
        return 1;

    rv = cbm_device_status(fd, unit, buf, sizeof(buf));

    if (options->petsciiraw == PA_PETSCII)
        cbm_petscii2ascii(buf);

    printf("%s", buf);

    return (rv == 99) ? 1 : 0;
}


/*
 * send device command
 */
static int do_command(CBM_FILE fd, OPTIONS * const options)
{
    int  rv;
    unsigned char unit;
    char *commandline;
    unsigned int commandlinelen = 0;

    int extended = 0;
    int c;
    static const char short_options[] = "+e";
    static struct option long_options[] =
    {
        {"extended", no_argument, NULL, 'e'},
        {NULL,       no_argument, NULL, 0  }
    };

    // first of all, process the options given

    while ((c = process_individual_option(options, short_options, long_options)) != EOF)
    {
        switch (c)
        {
        case 'e':
            extended = 1;
            break;

        default:
            return 1;
        }
    }

    rv = get_argument_char(options, &unit);
    rv = rv || get_extended_argument_string(extended, options, &commandline, &commandlinelen);

    if (rv)
        return 1;

    rv = cbm_listen(fd, unit, 15);
    if(rv == 0)
    {
        if (commandlinelen > 0)
            cbm_raw_write(fd, commandline, commandlinelen);

        // make sure the buffer is ended with a '\r'; this is needed
        // only if the command ends with a '\r', to work around a bug
        // in the floppy code.

        cbm_raw_write(fd, "\r", 1);

        rv = cbm_unlisten(fd);
    }

    if (commandline != NULL)
    {
        free(commandline);
    }

    return rv;
}

/*
 * display directory
 */
static int do_dir(CBM_FILE fd, OPTIONS * const options)
{
    char c, buf[40];
    unsigned char command[] = { '$', '0' };
    int rv;
    unsigned char unit;

    rv = skip_options(options);

    rv = rv || get_argument_char(options, &unit);
    /* default is drive '0' */
    if (options->argc > 0)
    {
        rv = rv || get_argument_char(options, command+1);
    }

    if (rv || check_if_parameters_ok(options))
        return 1;

    rv = cbm_open(fd, unit, 0, command, sizeof(command));
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
                        printf("%u ", (unsigned char)buf[0] | (unsigned char)buf[1] << 8 );
                        while((cbm_raw_read(fd, &c, 1) == 1) && c)
                        {
                            if (options->petsciiraw == PA_PETSCII)
                                putchar(cbm_petscii2ascii_c(c));
                            else
                                putchar(c);
                        }
                        putchar('\n');
                    }
                }
                cbm_untalk(fd);
                cbm_device_status(fd, unit, buf, sizeof(buf));
                printf("%s", cbm_petscii2ascii(buf));
            }
            else
            {
                cbm_untalk(fd);
            }

        }
        else
        {
            printf("%s", cbm_petscii2ascii(buf));
        }
        cbm_close(fd, unit, 0);
    }
    return rv;
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

/*
 * read device memory, dump to stdout or a file
 */
static int do_download(CBM_FILE fd, OPTIONS * const options)
{
    unsigned char unit;
    unsigned short c;
    int addr, count, rv = 0;
    char *tail, buf[256];
    FILE *f;

    char *tmpstring;

    if (skip_options(options))
        return 1;

    // process the drive number (unit)

    if (get_argument_char(options, &unit))
        return 1;


    // process the address

    if (get_argument_string(options, &tmpstring, NULL))
        return 1;

    addr = strtol(tmpstring, &tail, 0);
    if(addr < 0 || addr > 0xffff || *tail)
    {
        arch_error(0, 0, "invalid address: %s", tmpstring);
        return 1;
    }


    // process the count of bytes

    if (get_argument_string(options, &tmpstring, NULL))
        return 1;

    count = strtol(tmpstring, &tail, 0);
    if((count + addr) > 0x10000 || *tail)
    {
        arch_error(0, arch_get_errno(), "invalid byte count %s", tmpstring);
        return 1;
    }


    // process the filename, if any

    if (get_argument_file_for_write(options, &f))
        return 1;


    // download in chunks of sizeof(buf) (currently: 256) bytes
    while(count > 0)
    {
        show_monkey(count / sizeof(buf));

        c = (count > sizeof(buf)) ? sizeof(buf) : count;

        if (c + (addr & 0xFF) > 0x100) {
            c = 0x100 - (addr & 0xFF);
        }

        if(c != cbm_download(fd, unit, addr, buf, c))
        {
            rv = 1;
            fprintf(stderr, "A transfer error occurred!\n");
            break;
        }

        // If the user wants to convert them from PETSCII, do this
        // (I find it hard to believe someone would want to do this,
        // but who knows?)

        if (options->petsciiraw == PA_PETSCII)
        {
            int i;
            for (i = 0; i < c; i++)
                buf[i] = cbm_petscii2ascii_c(buf[i]);
        }

        fwrite(buf, 1, c, f);

        addr  += c;
        count -= c;
    }

    fclose(f);
    return rv;
}

/*
 * load binary data from file into device memory
 */
static int do_upload(CBM_FILE fd, OPTIONS * const options)
{
    unsigned char unit;
    int addr;
    int rv;
    size_t size;
    char *tail, *fn;
    char *tmpstring;
    unsigned char addr_buf[2];
    unsigned int buflen = 65537;
    unsigned char *buf;
    FILE *f;

    if (skip_options(options))
        return 1;

    // process the drive number (unit)

    if (get_argument_char(options, &unit))
        return 1;


    // process the address

    if (get_argument_string(options, &tmpstring, NULL))
        return 1;

    addr = strtol(tmpstring, &tail, 0);
    if(addr < -1 || addr > 0xffff || *tail)
    {
        arch_error(0, 0, "invalid address: %s", tmpstring);
        return 1;
    }

    if (get_argument_file_for_read(options, &f, &fn))
        return 1;


    // allocate memory for the transfer

    buf = malloc(buflen);
    if (!buf)
    {
        fprintf(stderr, "Not enough memory for buffer.\n");
        return 1;
    }

    if(addr == -1)
    {
        /* read address from file */
        if(fread(addr_buf, 2, 1, f) != 1)
        {
            arch_error(0, arch_get_errno(), "could not read %s", fn);
            if(f != stdin) fclose(f);
            free(buf);
            return 1;
        }

        /* don't assume a particular endianess, although the cbm4linux
         * package is only i386 for now  */
        addr = addr_buf[0] | (addr_buf[1] << 8);
    }

    size = fread(buf, 1, buflen, f);
    if(ferror(f))
    {
        arch_error(0, 0, "could not read %s", fn);
        if(f != stdin) fclose(f);
        free(buf);
        return 1;
    }
    else if(size == 0 && feof(f))
    {
        arch_error(0, 0, "no data: %s", fn);
        if(f != stdin) fclose(f);
        free(buf);
        return 1;
    }

    if(addr + size > 0x10000)
    {
        arch_error(0, 0, "program too big: %s", fn);
        if (f != stdin) fclose(f);
        free(buf);
        return 1;
    }

    if(f != stdin) fclose(f);

    // If the user wants to convert them from PETSCII, do this
    // (I find it hard to believe someone would want to do this,
    // but who knows?)

    if (options->petsciiraw == PA_PETSCII)
    {
        unsigned int i;
        for (i = 0; i < size; i++)
            buf[i] = cbm_ascii2petscii_c(buf[i]);
    }

    rv = (cbm_upload(fd, unit, addr, buf, size) == (int)size) ? 0 : 1;

    if ( rv != 0 ) {
        fprintf(stderr, "A transfer error occurred!\n");
    }

    free(buf);

    return rv;
}

/*
 * identify connected devices
 */
static int do_detect(CBM_FILE fd, OPTIONS * const options)
{
    unsigned int num_devices;
    unsigned char device;
    const char *type_str;

    if (skip_options(options))
        return 1;

    if (check_if_parameters_ok(options))
        return 1;

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
static int do_change(CBM_FILE fd, OPTIONS * const options)
{
    unsigned char unit;
    int rv;

    rv = skip_options(options);

    rv = rv || get_argument_char(options, &unit);

    if (rv || check_if_parameters_ok(options))
        return 1;

    do
    {
        /*
         * Determine if we have a supported drive type.
         * Note: As we do not recognize all drives reliably,
         *       we only block a 1581. If we cannot determine
         *       the drive type, just allow using it!
         * \todo: Fix this!
         */

        enum cbm_device_type_e device_type;

        if (cbm_identify(fd, unit, &device_type, NULL) == 0)
        {
            if (device_type == cbm_dt_cbm1581)
            {
                fprintf(stderr, "Drive %u is a 1581, which is not supported (yet).\n", unit);
                rv = 1;
                break;
            }
        }

        /*
         * Make sure the drive is on track 18
         */
        if (cbm_exec_command(fd, unit, "I0:", 0) != 0)
        {
            /*
             * The drive did not react; most probably, there is none,
             * thus quit.
             */
            rv = 1;
            break;
        }

        if (cbm_upload(fd, unit, 0x500, prog_tdchange, sizeof(prog_tdchange)) != sizeof(prog_tdchange))
        {
            /*
             * The drive code wasn't uploaded fully, thus quit.
             */
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


int ARCH_MAINDECL main(int argc, char *argv[])
{
    struct prog *pprog;

    OPTIONS options;

    int rv = 0;

    do {
        CBM_FILE fd;

        // first of all, process the options which affect all commands

        rv = process_cmdline_common_options(argc, argv, &options);

        // check if we have --version or --help specified:

        if (process_version_help(&options))
            break;

        pprog = process_cmdline_find_command(&options);

        if (pprog == NULL)
        {
            printf("invalid command. For info on possible commands, try the --help parameter.\n\n");
            rv = 2;
            break;
        }

        // if neither PETSCII or RAW was specified, use default for that command
        if (options.petsciiraw == PA_UNSPEC)
            options.petsciiraw = pprog->petsciiraw;

        if (pprog->need_driver)
            rv = cbm_driver_open_ex(&fd, options.adapter);

        if (rv != 0)
        {
            if (arch_get_errno())
            {
                arch_error(0, arch_get_errno(), "%s", cbm_get_driver_name_ex(options.adapter));
            }
            else
            {
                fprintf(stderr, "An error occurred opening OpenCBM, aborting...\n");
            }
            break;
        }

        rv = pprog->prog(fd, &options) != 0;
        if (rv && arch_get_errno())
        {
            arch_error(0, arch_get_errno(), "%s", pprog->name);
        }

        if (pprog->need_driver)
            cbm_driver_close(fd);

    } while (0);

    free_options(&options);

    return rv;
}

#endif

//! struct to remember the general options to the program
typedef struct {
    int argc;    //!< a (modifieable) copy of the number of arguments, as given to main()
    char **argv; //!< a (modifieable) copy of the argument list, as given to main()
    int error;   //!< there was an error in processing the options (=1), or not (=0)
    int help;    //!< option: the user requested help for the specified command
    int version; //!< option: print version information
    char *adapter; //!< option: an explicit adapter was specified
} OPTIONS;

static int
set_option(int *Where, int NewValue, int OldValue, const char * const Description)
{
    int error = 0;

    if (*Where != OldValue)
    {
        error = 1;
        fprintf(stderr, "Specified option '%s', but there is a conflicting option,\n"
            "or it is specified twice!\n", Description);
    }

    *Where = NewValue;

    return error;
}

static int
process_cmdline_common_options(int argc, char **argv, OPTIONS *options)
{
    int option_index;
    int option;

    static const char short_options[] = "+hV@:";
    static struct option long_options[] =
    {
        { "adapter" , required_argument, NULL, '@' },
        { "help"    , no_argument,       NULL, 'h' },
        { "version" , no_argument,       NULL, 'V' },
        { NULL      , no_argument,       NULL, 0   }
    };

    // clear all options
    memset(options, 0, sizeof(*options));
    options->adapter = 0;

    // remember argc and argv, so they can be used later
    options->argc = argc;
    options->argv = argv;

    // first of all, parse the command-line (before the command):
    // is -f specified (thus, we should ignore .opencbmrc)?
    //
    optind = 0;

    while ((option = getopt_long(options->argc, options->argv, short_options, long_options, &option_index)) != EOF)
    {
        switch (option)
        {
        case '?':
        default:
            fprintf(stderr, "unknown option %s specified!\n",
                options->argv[optind-1]);
            options->error = 1;
            break;

        case 'h':
            set_option(&options->help, 1, 0, "--help");
            break;

        case '@':
            if (options->adapter != NULL)
                options->error = 1;
            else
                options->adapter = cbmlibmisc_strdup(optarg);
            break;

        case 'V':
            set_option(&options->version, 1, 0, "--version");
            break;

        };
    }

    // skip the options that are already processed
    options->argc -= optind;
    options->argv += optind;

    return options->error;
}

static void
free_options(OPTIONS * const options)
{
    cbmlibmisc_strfree(options->adapter);
}

/*
 * Output version information
 */
static int do_version(OPTIONS * const options)
{
    printf("cbmctrl version " OPENCBM_VERSION ", built on " __DATE__ " at " __TIME__ "\n");

    return 0;
}

/*
 * Output a help screen
 */
static int do_help(OPTIONS * const options)
{
    do_version(options);

    printf("\n");

    printf("control serial CBM devices\n"
        "\n"
        " Synopsis:  linetest [options]\n"
        "\n"
        " Options:\n"
        "\n"
        "   -h, --help:    Output this help screen if specified without an action.\n"
        "                  Outputs some help information about a specific action\n"
        "                  if an action is specified.\n"
        "   -V, --version: Output version information\n"
        "   -@, --adapter: Tell OpenCBM which backend plugin and bus to use. This option\n"
        "                  requires an argument of the form <plugin>[:<bus>].\n"
        "                  <plugin> is the backend plugin's name to use (e.g.: xa1541)\n"
        "                  <bus>    is a bus unit identifier, if supported by the backend;\n"
        "                           look up the backend's documentation for the supported\n"
        "                           bus unit identifier(s) and the format for <bus>\n"
        "\n");

    return 0;
}


static int
process_version_help(OPTIONS * const options)
{
    int processed = 0;

    if (options->help)
    {
        do_help(options);
        processed = 1;
    }

    if (options->version)
    {
        do_version(NULL);
        processed = 1;
    }

    if (processed && options->argc > 0)
        fprintf(stderr, "Extra parameters on command-line are ignored.\n");

    return processed;
}

static const char *
getLineStatus(unsigned int status)
{
    static char strstatus[25];

    arch_snprintf(strstatus, sizeof strstatus, "%s %s %s %s",
        (status & IEC_RESET) ? "RESET" : "     ",
        (status & IEC_ATN  ) ? "ATN  " : "     ",
        (status & IEC_DATA ) ? "DATA " : "     ",
        (status & IEC_CLOCK) ? "CLOCK" : "     ");

    return strstatus;
}

#if 0
static void
printpoll(CBM_FILE fd)
{
    /* read the line status */
    int poll_status;

    poll_status = cbm_iec_poll(fd);

    printf("line status: %s\n", getLineStatus(poll_status));
}
#endif

static unsigned int
output_status(CBM_FILE fd, const char * text, unsigned int expected)
{
    unsigned int poll = cbm_iec_poll(fd);

    printf("%-15s %02X (%02X) %s%s\n", text, poll, expected, getLineStatus(poll), (poll != expected) ? "*** FAIL ***" : "");

    return poll != expected;
}

static int
do_test(CBM_FILE fd)
{
    int rv = 0;

    do {
        output_status(fd, "start:", 0);
        cbm_reset(fd);
        arch_usleep(250000);
        output_status(fd, "RESET:", 0);

        cbm_iec_set(fd, IEC_ATN);
        output_status(fd, "ATN:", 0x05);
        arch_usleep(200000);
        output_status(fd, "ATN + 200ms:", 0x04);
        cbm_iec_release(fd, IEC_ATN);
        output_status(fd, "-ATN:", 0x00);
        arch_usleep(200000);
        output_status(fd, "-ATN + 200ms:", 0x00);

        cbm_iec_set(fd, IEC_DATA);
        output_status(fd, "DATA:", 0x01);
        arch_usleep(200000);
        output_status(fd, "DATA + 200ms:", 0x01);
        cbm_iec_release(fd, IEC_DATA);
        output_status(fd, "-DATA:", 0x00);
        arch_usleep(200000);
        output_status(fd, "-DATA + 200ms:", 0x00);

        cbm_iec_set(fd, IEC_CLOCK);
        output_status(fd, "CLK:", 0x02);
        arch_usleep(200000);
        output_status(fd, "CLK + 200ms:", 0x02);
        cbm_iec_release(fd, IEC_CLOCK);
        output_status(fd, "-CLK:", 0x00);
        arch_usleep(200000);
        output_status(fd, "-CLK + 200ms:", 0x00);

        rv = 0;
    } while (0);

    return rv;
}

int ARCH_MAINDECL main(int argc, char *argv[])
{
    OPTIONS options;

    int rv = 0;

    do {
        CBM_FILE fd;

        // first of all, process the options which affect all commands

        rv = process_cmdline_common_options(argc, argv, &options);

        // check if we have --version or --help specified:

        if (process_version_help(&options))
            break;

        rv = cbm_driver_open_ex(&fd, options.adapter);

        if (rv != 0)
        {
            if (arch_get_errno())
            {
                arch_error(0, arch_get_errno(), "%s", cbm_get_driver_name_ex(options.adapter));
            }
            else
            {
                fprintf(stderr, "An error occurred opening OpenCBM, aborting...\n");
            }
            break;
        }

        rv = do_test(fd);
        if (rv && arch_get_errno())
        {
            arch_error(0, arch_get_errno(), "test");
        }

        cbm_driver_close(fd);

    } while (0);

    free_options(&options);

    return rv;
}
