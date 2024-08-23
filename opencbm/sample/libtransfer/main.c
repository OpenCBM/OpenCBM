/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2006, 2009, 2011, 2016, 2020, 2024 Spiro Trikaliotis
*/

#include "opencbm.h"
#include "libtrans.h"

#include "arch.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef LIBOCT_STATE_DEBUG
# define DEBUG_STATEDEBUG
#endif
#include "statedebug.h"


/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! The name of the executable */
#define DBG_PROGNAME "LIBTRANS.EXE"

#include "debug.h"

static int startaddress = 0xc000;
static int transferlength = 0x120;
static int writedumpfile = 0;
static int outputdump = 0;
static int compare = 0;
static int do_read = 0;
static int do_write = 0;
static unsigned char drive = 8;
static unsigned int count = 1;

static CBM_FILE fd;

static void ARCH_SIGNALDECL
handle_CTRL_C(int dummy)
{
    fprintf(stderr, "\nSIGINT caught, resetting IEC bus...\n");
    DEBUG_PRINTDEBUGCOUNTERS();

    arch_sleep(1);
    cbm_reset(fd);

    cbm_driver_close(fd);
    exit(1);
}

static void
write_to_file(const unsigned char Buffer[], const unsigned int BufferSize, const char * const Filename)
{
    FILE *f;

    f = fopen(Filename, "wb");

    if (f)
    {
        fwrite(Buffer, 1, BufferSize, f);
        fclose(f);
    }
}

static void
read_from_file(unsigned char Buffer[], const unsigned int BufferSize, const char * const Filename)
{
    FILE *f;

    f = fopen(Filename, "rb");

    if (f)
    {
        if ( ! (fread(Buffer, 1, BufferSize, f) == BufferSize) )
        {
            /*! \bug cannot return error code */
        }
        fclose(f);
    }
}

static void
processParameter(const int argc, char ** argv)
{
    int i;

    for (i=1; i<argc; i++)
    {
        if (argv[i][0] == '-')
        {
            // process switches
            switch (argv[i][1])
            {
            case 't':
                if (strcmp(&argv[i][2], "s1") == 0)
                {
                    libopencbmtransfer_set_transfer(opencbm_transfer_serial1);
                }
                else if (strcmp(&argv[i][2], "s2") == 0)
                {
                    libopencbmtransfer_set_transfer(opencbm_transfer_serial2);
                }
                else if ((strcmp(&argv[i][2], "s3") == 0) || (strcmp(&argv[i][2], "srq") == 0))
                {
                    libopencbmtransfer_set_transfer(opencbm_transfer_serial3);
                }
                else if (strcmp(&argv[i][2], "pp") == 0)
                {
                    libopencbmtransfer_set_transfer(opencbm_transfer_parallel);
                }
                else
                {
                    printf("unknown transfer protocol '%s'\n", &argv[i][2]);
                    exit(1);
                }
                break;

            case 'c':
                count = atoi(&argv[i][2]);
                break;

            case 'C':
                compare = 1;
                break;

            case 'D':
                drive = (char) atoi(&argv[i][2]);
                break;

            case 'd':
                outputdump = 1;
                break;

            case 'W':
                writedumpfile = 1;
                break;

            case 'r':
                do_read = 1;
                break;

            case 'w':
                do_write = 1;
                break;

            case 'a':
                startaddress = atoi(&argv[i][2]);
                printf("using start address of 0x%x\n", startaddress);
                break;

            default:
                printf("unknown option '%s'\n", argv[i]);
                exit(1);
            }
        }
        else
        {
            transferlength = atoi(argv[i]);
            if (transferlength != 0)
            {
                printf("using length of 0x%x\n", transferlength);
            }
            else
            {
                printf("unknown argument '%s'\n", argv[i]);
                exit(1);
            }
        }
    }

    if (do_read == 0 && do_write == 0) {
        do_read = 1;
        do_write = 1;
    }

}

#if 0

#include "o65.h"

static int
main_o65(int argc, char **argv)
{
    int i;
    int count;
    PVOID o65file[10];

    FUNC_ENTER();

#if DBG
    DbgFlags = DBGF_SUCCESS | DBGF_ERROR | DBGF_WARNING | DBGF_ASSERT;
#endif

    count = argc > 10 ? 10 : argc - 1;

    for (i=0; i < count; i++)
    {
        o65_file_load(argv[i+1], &o65file[i]);
    }

    for (i=0; i < count; i++)
    {
        o65_file_reloc(o65file[i], i*0x204);
    }

    for (i=0; i < count; i++)
    {
        o65_file_delete(o65file[i]);
    }

    FUNC_LEAVE_INT(0);
}
#endif

static void
perform_read(CBM_FILE fd, unsigned char *buffer, unsigned char *compare_buffer, unsigned int count, unsigned int error) {
        printf("read:  %i, error = %u: \n", count+1, error);
        libopencbmtransfer_read_mem(fd, drive, buffer, startaddress, transferlength);
        if (compare)
        {
            if (memcmp(buffer, compare_buffer, transferlength) != 0)
            {
                char filename[128];
                int n;

                printf("\n\n***** ERROR COMPARING DATA! *****\n\n");
                ++error;

                strcpy(filename, "image.err.");
                n = strlen(filename);

                if (error > 9999) filename[n++] = ((error / 10000) % 10) + '0';
                if (error >  999) filename[n++] = ((error /  1000) % 10) + '0';
                if (error >   99) filename[n++] = ((error /   100) % 10) + '0';
                if (error >    9) filename[n++] = ((error /    10) % 10) + '0';
                if (error >    0) filename[n++] = ((error /     1) % 10) + '0';

                filename[n] = 0;
                write_to_file(buffer, transferlength, filename);
            }
            else
            {
                printf("       compare success\n");
            }
        }
}

static void
perform_write(CBM_FILE fd, unsigned char *compare_buffer, unsigned int count, unsigned int error) {
    printf("write: %i, error = %u: \n", count+1, error);
    libopencbmtransfer_write_mem(fd, drive, compare_buffer, startaddress, transferlength);
}

static int
main_testtransfer(int argc, char **argv)
{
    /* CBM_FILE fd; */
    unsigned char compare_buffer[0x8000];
    unsigned char buffer[0x8000];
    unsigned int error = 0;
    int rv;

#if DBG
    DbgFlags = 0x0;
#endif

    printf(" libtransfer sample: " __DATE__ " " __TIME__ "\n");

    if (argc > 1)
    {
        processParameter(argc, argv);
    }

    rv = cbm_driver_open(&fd, 0);

    if(rv != 0)
    {
        DBG_ERROR((DBG_PREFIX "cbm_driver_open() failed!"));
        return 1;
    }

    DBG_PRINT((DBG_PREFIX "before install"));
    if (libopencbmtransfer_install(fd, drive)) {
        cbm_driver_close(fd);
        return 1;
    }

    memset(buffer, 0, sizeof(buffer));

    if (compare)
        read_from_file(compare_buffer, sizeof(compare_buffer), "rom.image");

    DBG_PRINT((DBG_PREFIX "before read_mem"));

#if 0
    transferlength = 0x100;
    printf("write $0300\n");
    libopencbmtransfer_write_mem(fd, drive, compare_buffer, 0x300, transferlength);
    printf("read $0300\n");
    libopencbmtransfer_read_mem(fd, drive, buffer, 0x300, transferlength);

    printf("compare\n");
    if (memcmp(buffer, compare_buffer, transferlength) != 0)
    {
        printf("differing!\n");
    }
#else
    while (count--)
    {
        if (do_read) {
            perform_read(fd, buffer, compare_buffer, count, error);
        }

        if (do_write) {
            perform_write(fd, compare_buffer, count, error);
        }
    }
#endif

    if (outputdump) {
        char address[10];
        sprintf(address, "%04x:", startaddress);
        DBG_MEMDUMP(address, buffer, transferlength + 0x10);
    }

    if (writedumpfile)
        write_to_file(buffer, transferlength, "image.bin");

    DBG_PRINT((DBG_PREFIX "before remove"));
    libopencbmtransfer_remove(fd, drive);

    cbm_driver_close(fd);

    FUNC_LEAVE_INT(0);
}


static void
read_line_status(CBM_FILE fd)
{
    DBGDO(int data;)
    DBGDO(int clock;)
    DBGDO(int atn;)
    DBGDO(int reset;)
    DBGDO(int pp;)

    FUNC_ENTER();

    DBGDO(data  = cbm_iec_get(fd, IEC_DATA);)
    DBGDO(clock = cbm_iec_get(fd, IEC_CLOCK);)
    DBGDO(atn   = cbm_iec_get(fd, IEC_ATN);)
    DBGDO(reset = cbm_iec_get(fd, IEC_RESET);)
    DBGDO(pp    = cbm_pp_read(fd));

    DBG_PRINT((DBG_PREFIX
        "READ: DATA = %s, CLOCK = %s, ATN = %s, RESET = %s, PP = $%02x",
        data  ? "TRUE " : "FALSE",
        clock ? "TRUE " : "FALSE",
        atn   ? "TRUE " : "FALSE",
        reset ? "TRUE " : "FALSE",
        pp));

    FUNC_LEAVE();
}

static int
main_showlines(int argc, char **argv)
{
    /* CBM_FILE fd; */
    int rv;

    FUNC_ENTER();

    do
    {
        int data = -1;
        int clock = -1;
        int atn = -1;
        int reset = -1;
        int pp = -1;

        rv = cbm_driver_open(&fd, 0);

        if (rv != 0)
            break;

        while (1) {
                int data1  = cbm_iec_get(fd, IEC_DATA);
                int clock1 = cbm_iec_get(fd, IEC_CLOCK);
                int atn1   = cbm_iec_get(fd, IEC_ATN);
                int reset1 = cbm_iec_get(fd, IEC_RESET);
                int pp1    = cbm_pp_read(fd);

                if ( (data != data1)
                        || (clock != clock1)
                        || (atn != atn1)
                        || (reset != reset1)
                        || (pp != pp1)
                   )
                {
                    data = data1;
                    clock = clock1;
                    atn = atn1;
                    reset = reset1;
                    pp = pp1;

                    fprintf(stderr,
                        "READ: DATA = %s, CLOCK = %s, ATN = %s, RESET = %s, PP = $%02x\n",
                        data  ? "TRUE " : "FALSE",
                        clock ? "TRUE " : "FALSE",
                        atn   ? "TRUE " : "FALSE",
                        reset ? "TRUE " : "FALSE",
                        pp);
                }
        }
    } while (0);

    return 0;
}

static int
main_testlines(int argc, char **argv)
{
    /* CBM_FILE fd; */
    int rv;

    FUNC_ENTER();

    do
    {
        rv = cbm_driver_open(&fd, 0);

        if (rv != 0)
            break;

        read_line_status(fd);

        DBG_PRINT((DBG_PREFIX ""));
        DBG_PRINT((DBG_PREFIX "Releasing all lines"));
        cbm_iec_release(fd, IEC_CLOCK | IEC_DATA | IEC_ATN | IEC_RESET);
        read_line_status(fd);

        DBG_PRINT((DBG_PREFIX ""));
        DBG_PRINT((DBG_PREFIX "Setting CLOCK"));
        cbm_iec_set(fd, IEC_CLOCK);
        read_line_status(fd);

        DBG_PRINT((DBG_PREFIX ""));
        DBG_PRINT((DBG_PREFIX "Setting DATA"));
        cbm_iec_set(fd, IEC_DATA);
        read_line_status(fd);

        DBG_PRINT((DBG_PREFIX ""));
        DBG_PRINT((DBG_PREFIX "Setting ATN"));
        cbm_iec_set(fd, IEC_ATN);
        read_line_status(fd);

        DBG_PRINT((DBG_PREFIX ""));
        DBG_PRINT((DBG_PREFIX "Setting RESET"));
        cbm_iec_set(fd, IEC_RESET);
        read_line_status(fd);

        DBG_PRINT((DBG_PREFIX ""));
        DBG_PRINT((DBG_PREFIX "Releasing CLOCK"));
        cbm_iec_release(fd, IEC_CLOCK);
        read_line_status(fd);

        DBG_PRINT((DBG_PREFIX ""));
        DBG_PRINT((DBG_PREFIX "Releasing DATA"));
        cbm_iec_release(fd, IEC_DATA);
        read_line_status(fd);

        DBG_PRINT((DBG_PREFIX ""));
        DBG_PRINT((DBG_PREFIX "Releasing ATN"));
        cbm_iec_release(fd, IEC_ATN);
        read_line_status(fd);

        DBG_PRINT((DBG_PREFIX ""));
        DBG_PRINT((DBG_PREFIX "Releasing RESET"));
        cbm_iec_release(fd, IEC_RESET);
        read_line_status(fd);

        DBG_PRINT((DBG_PREFIX ""));
        DBG_PRINT((DBG_PREFIX "Setting DATA and ATN"));
        cbm_iec_set(fd, IEC_DATA | IEC_ATN);
        read_line_status(fd);

        DBG_PRINT((DBG_PREFIX ""));
        DBG_PRINT((DBG_PREFIX "Setting CLOCK, releasing ATN"));
        cbm_iec_setrelease(fd, IEC_ATN | IEC_CLOCK, IEC_CLOCK);
        read_line_status(fd);

        DBG_PRINT((DBG_PREFIX ""));
        DBG_PRINT((DBG_PREFIX "Releasing all lines"));
        cbm_iec_release(fd, IEC_CLOCK | IEC_DATA | IEC_ATN | IEC_RESET);
        read_line_status(fd);

    } while (0);

    if (rv == 0)
        cbm_driver_close(fd);

    FUNC_LEAVE_INT(0);
}

// #define TEST_DEFAULT_LINES
#define TEST_DEFAULT_TRANSFER
// #define TEST_DEFAULT_O65

int
ARCH_MAINDECL main(int argc, char **argv)
{
    arch_set_ctrlbreak_handler(handle_CTRL_C);

    if (argc > 1) {
        if (strcmp(argv[1], "--show")==0) {
            return main_showlines(argc, argv);
        }
        if (strcmp(argv[1], "--lines")==0) {
            return main_testlines(argc, argv);
        }
        if (strcmp(argv[1], "--transfer")==0) {
            return main_testtransfer(argc, argv);
        }
#if 0
        if (strcmp(argv[1], "--o65")==0) {
            return main_o65(argc, argv);
        }
#endif
    }

#ifdef TEST_DEFAULT_LINES
    return main_testlines(argc, argv);
#endif

#ifdef TEST_DEFAULT_TRANSFER
    return main_testtransfer(argc, argv);
#endif

#ifdef TEST_DEFAULT_O65
    return main_o65(argc, argv);
#endif
}
