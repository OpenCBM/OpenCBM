// vim: set expandtab tabstop=4 shiftwidth=4 autoindent smartindent:

#include "opencbm.h"

#include "arch.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(_x) ( sizeof (_x) / sizeof ((_x)[0]) )
#endif

#define IGNORE_MASK 0xEFu

static const unsigned char irqdelay_1571[] = {
#include "irqdelay-1571.inc"
};

static const unsigned char irqdelay_1571_oneshot[] = {
#include "irqdelay-1571-oneshot.inc"
};

static const unsigned char irqdelay_1581[] = {
#include "irqdelay-1581.inc"
};

static const unsigned char irqdelay_1581_oneshot[] = {
#include "irqdelay-1581-oneshot.inc"
};

static const unsigned char cia_sdr_icr_1571[] = {
#include "cia-sdr-icr-1571.inc"
};

static const unsigned char cia_sdr_icr_1581[] = {
#include "cia-sdr-icr-1581.inc"
};


static const unsigned char dump1_4[] = {
#include "dumps/dump1-4.ibin"
};

static const unsigned char dump1_5[] = {
#include "dumps/dump1-5.ibin"
};

static const unsigned char dump1_6[] = {
#include "dumps/dump1-6.ibin"
};

static const unsigned char dump1_7[] = {
#include "dumps/dump1-7.ibin"
};

static const unsigned char dump1_8[] = {
#include "dumps/dump1-8.ibin"
};

static const unsigned char dump1_50[] = {
#include "dumps/dump1-50.ibin"
};

static const unsigned char dump1_51[] = {
#include "dumps/dump1-51.ibin"
};

static const unsigned char dump1_64[] = {
#include "dumps/dump1-64.ibin"
};

static const unsigned char dump2_4[] = {
#include "dumps/dump2-4.ibin"
};

static const unsigned char dump2_5[] = {
#include "dumps/dump2-5.ibin"
};

static const unsigned char dump2_6[] = {
#include "dumps/dump2-6.ibin"
};

static const unsigned char dump2_7[] = {
#include "dumps/dump2-7.ibin"
};

static const unsigned char dump2_8[] = {
#include "dumps/dump2-8.ibin"
};

static const unsigned char dump2_50[] = {
#include "dumps/dump2-50.ibin"
};

static const unsigned char dump2_51[] = {
#include "dumps/dump2-51.ibin"
};

static const unsigned char dump2_64[] = {
#include "dumps/dump2-64.ibin"
};

static const unsigned char dump2_4485_7[] = {
#include "dumps/dump2-4485-7.ibin"
};

static const unsigned char dump2_4485_8[] = {
#include "dumps/dump2-4485-8.ibin"
};

static const unsigned char dump2_4485_50[] = {
#include "dumps/dump2-4485-50.ibin"
};

static const unsigned char dump2_4485_51[] = {
#include "dumps/dump2-4485-51.ibin"
};

static const unsigned char dump2_4485_64[] = {
#include "dumps/dump2-4485-64.ibin"
};

static const struct dump_s {
    const unsigned int    baudrate;
    const unsigned char * dump;
} dumps[3][8] =
{
    {
        {  4, dump1_4  },
        {  5, dump1_5  },
        {  6, dump1_6  },
        {  7, dump1_7  },
        {  8, dump1_8  },
        { 50, dump1_50 },
        { 51, dump1_51 },
        { 64, dump1_64 },
    },
    {
        {  4, dump2_4  },
        {  5, dump2_5  },
        {  6, dump2_6  },
        {  7, dump2_7  },
        {  8, dump2_8  },
        { 50, dump2_50 },
        { 51, dump2_51 },
        { 64, dump2_64 },
    },
    {
        {  4, NULL          },
        {  5, NULL          },
        {  6, NULL          },
        {  7, dump2_4485_7  },
        {  8, dump2_4485_8  },
        { 50, dump2_4485_50 },
        { 51, dump2_4485_51 },
        { 64, dump2_4485_64 },
    }
};

struct drive_functions_s {
    const unsigned char * irqdelay;
    const unsigned int    irqdelay_len;

    const unsigned char * irqdelay_oneshot;
    const unsigned int    irqdelay_oneshot_len;

    const unsigned char * cia_sdr_icr;
    const unsigned int    cia_sdr_icr_len;

    const unsigned int    cia_sdr_icr_startaddress;
    const unsigned int    cia_sdr_icr_startaddress_len;

    const unsigned int    cia_sdr_icr_contaddress;
    const unsigned int    cia_sdr_icr_contaddress_len;
    const unsigned int    cia_sdr_icr_jsr_swap_address;

    const unsigned int    cia_sdr_result_address;
    const unsigned int    cia_sdr_result2_address;
    const unsigned int    cia_sdr_tottest;
};

static struct drive_functions_s drive_functions[] = {
    {
        irqdelay_1571,
        sizeof irqdelay_1571,
        irqdelay_1571_oneshot,
        sizeof irqdelay_1571_oneshot,
        cia_sdr_icr_1571,
        sizeof cia_sdr_icr_1571,
        0x0146,             // start of first block
        0x01A0 - 0x0146,    // length of first block
        0x0401,             // start of second block
        0x046d - 0x0401,    // length of second block
        0x0427,             // where are the JSR to change between results and results2
        0x0500,             // results are found here
        0x0500,             // results2 are found here
        767                 // tottests
    },
    {
        irqdelay_1581,
        sizeof irqdelay_1581,
        irqdelay_1581_oneshot,
        sizeof irqdelay_1581_oneshot,
        cia_sdr_icr_1581,
        sizeof cia_sdr_icr_1581,
        0x0401,             // start of first block
        0x04C7 - 0x0401,    // length of first block
        0x0,                // start of second block (none)
        0,                  // length of second block (none)
        0,                  // where are the JSR to change between results and results2
        0x0500,             // results are found here
        0x0900,             // results2 are found here
        1000 - 1            // tottests
    },
};

static unsigned int test_baudrates[] = { 4, 5, 6, 7, 8, 50, 51, 64 };

static unsigned int current_drive = 0;

void set_1571()
{
    current_drive = 0;
}

void set_1581()
{
    current_drive = 1;
}

#define DUMP_BYTE_PER_ROW 16

void memdump(const char * text, const unsigned char * buffer, unsigned int bufferlen, unsigned int offset)
{
    unsigned int row, col;
    printf("DUMP of %s:", text);

    for (row = 0; row < bufferlen; row += DUMP_BYTE_PER_ROW) {
        printf("\n%04X: ", offset + row);
        for (col = row; (col < bufferlen) && (col < row + DUMP_BYTE_PER_ROW); col++) {
            printf("%02X ", buffer[col]);
        }
    }
    printf("\n");
}

void mod_and(unsigned char * buffer, unsigned int bufferlen, unsigned char mask)
{
    unsigned int i;
    for (i = 0; i < bufferlen; i++) {
        buffer[i] &= mask;
    }
}

unsigned int check_result(const unsigned char * buffer_in, unsigned int len, unsigned int baudrate, unsigned int index)
{
    unsigned int match = 0;

    unsigned int i;

    unsigned int baudrate_index = 0;

    for (baudrate_index = 0; baudrate_index < ARRAYSIZE(dumps[0]); baudrate_index++) {
        if (dumps[index][baudrate_index].baudrate == baudrate) {
            break;
        }
    }

    if (baudrate_index == ARRAYSIZE(dumps[0])) return match;

    if (dumps[index][baudrate_index].dump == NULL) return match;

    for (i = 0; i < len; i++) {
        if ( (dumps[index][baudrate_index].dump[i] & IGNORE_MASK) != ( buffer_in[i] & IGNORE_MASK) ) {
            return match;
        }
    }

    match = 1;

    return match;
}

void test_cia_sdr(CBM_FILE fd, unsigned char drv, struct drive_functions_s *drive_functions)
{
    char execute_command[] = "M-E  ";
    const unsigned int drvaddress_baudrate = drive_functions->cia_sdr_icr_startaddress;
    const unsigned int drvaddress_start    = drvaddress_baudrate + 2;

    static unsigned char result[1000] = { 0 };
    static unsigned char result2[1000] = { 0 };

    static unsigned char jsrfunc_orig[6];
    static unsigned char jsrfunc_swapped[6];

    unsigned int baudindex;

    printf("\n\nTest CIA SDR:\n");

    execute_command[3] = drvaddress_start & 0xFFu;
    execute_command[4] = (drvaddress_start >> 8) & 0xFFu;

    if (drive_functions->cia_sdr_icr_jsr_swap_address) {
        unsigned int offset = drive_functions->cia_sdr_icr_jsr_swap_address - drive_functions->cia_sdr_icr_contaddress + drive_functions->cia_sdr_icr_startaddress_len;
        memcpy(jsrfunc_orig, &drive_functions->cia_sdr_icr[offset], 6);
        jsrfunc_swapped[0] = jsrfunc_orig[0];
        jsrfunc_swapped[1] = jsrfunc_orig[4];
        jsrfunc_swapped[2] = jsrfunc_orig[5];
        jsrfunc_swapped[3] = jsrfunc_orig[3];
        jsrfunc_swapped[4] = jsrfunc_orig[1];
        jsrfunc_swapped[5] = jsrfunc_orig[2];

#if DBG_PROGRAM
        memdump("orig jsrfunc", jsrfunc_orig, 6, drive_functions->cia_sdr_icr_jsr_swap_address);
        memdump("mod. jsrfunc", jsrfunc_swapped, 6, drive_functions->cia_sdr_icr_jsr_swap_address);
        printf("\n");
#endif
    }

    // initial upload of the program
    cbm_upload(
            fd,
            drv,
            drive_functions->cia_sdr_icr_startaddress,
            drive_functions->cia_sdr_icr,
            drive_functions->cia_sdr_icr_startaddress_len
            );
#if DBG_PROGRAM
    memdump("up1",
            drive_functions->cia_sdr_icr,
            drive_functions->cia_sdr_icr_startaddress_len,
            drive_functions->cia_sdr_icr_startaddress);
#endif

    if (drive_functions->cia_sdr_icr_contaddress_len) {
        cbm_upload(
                fd,
                drv,
                drive_functions->cia_sdr_icr_contaddress,
                drive_functions->cia_sdr_icr + drive_functions->cia_sdr_icr_startaddress_len,
                drive_functions->cia_sdr_icr_contaddress_len
                );
#if DBG_PROGRAM
        memdump("up2",
                drive_functions->cia_sdr_icr + drive_functions->cia_sdr_icr_startaddress_len,
                drive_functions->cia_sdr_icr_contaddress_len,
                drive_functions->cia_sdr_icr_contaddress);
#endif
    }

    for (baudindex = 0; baudindex < ARRAYSIZE(test_baudrates); baudindex++) {
        unsigned char baudrate[2];

        printf("\n- Test baudrate %u:\n", test_baudrates[baudindex]);

        baudrate[0] = test_baudrates[baudindex] & 0xFFu;
        baudrate[1] = (test_baudrates[baudindex] >> 8) & 0xFFu;

        cbm_upload(fd, drv, drvaddress_baudrate, baudrate, sizeof baudrate);

        if (drive_functions->cia_sdr_icr_jsr_swap_address) {
            cbm_upload(fd, drv, drive_functions->cia_sdr_icr_jsr_swap_address, jsrfunc_swapped, sizeof jsrfunc_swapped);
            cbm_exec_command(fd, drv, execute_command, 5);
        }

        // now, get the results from the drive
        cbm_download(fd, drv, drive_functions->cia_sdr_result_address,  result,  drive_functions->cia_sdr_tottest);


        if (check_result(result, drive_functions->cia_sdr_tottest, test_baudrates[baudindex], 0)) {
            printf("result for normal CIA\n");
        }
        else {
//          mod_and(result, drive_functions->cia_sdr_tottest, IGNORE_MASK);
            memdump("result",  result,  drive_functions->cia_sdr_tottest, 0);
        }

        if (drive_functions->cia_sdr_icr_jsr_swap_address) {
            cbm_upload(fd, drv, drive_functions->cia_sdr_icr_jsr_swap_address, jsrfunc_orig, sizeof jsrfunc_orig);
            cbm_exec_command(fd, drv, execute_command, 5);
        }

        if (drive_functions->cia_sdr_result2_address) {
            cbm_download(fd, drv, drive_functions->cia_sdr_result2_address, result2, drive_functions->cia_sdr_tottest);


            if (check_result(result2, drive_functions->cia_sdr_tottest, test_baudrates[baudindex], 1)) {
                printf("result2 for normal CIA\n");
            }
            else {
                if (check_result(result, drive_functions->cia_sdr_tottest, test_baudrates[baudindex], 2)) {
                    printf("result2 for 4485 CIA\n");
                }
                else {
//                  mod_and(result2, drive_functions->cia_sdr_tottest, IGNORE_MASK);
                    memdump("result2", result2, drive_functions->cia_sdr_tottest, 0);
                }
            }
        }

    }
}

void test_irqdelay(CBM_FILE fd, unsigned char drv, const unsigned char * func, unsigned int func_len, char * str)
{
    char result[2];
    unsigned int count;

    printf("\n\n%s:\n", str);

    cbm_upload(fd, drv, 0x0500, func, func_len);

    for (count = 0; count < 10; count++) {
        char * cia_str = "new CIA";

        cbm_exec_command(fd, drv, "U3:", 0);
        cbm_download(fd, drv, 0x0300, result, sizeof(result));

        if (result[0]) cia_str = "old CIA";

        printf("%4d: Result is: %02X (%s), %02X\n", count, (unsigned int) result[0], cia_str, (unsigned int) result[0]);
    };
}

void testdrive(CBM_FILE fd, unsigned char drv)
{
    test_irqdelay(fd, drv, drive_functions[current_drive].irqdelay,         drive_functions[current_drive].irqdelay_len,         "irqdelay");
    test_irqdelay(fd, drv, drive_functions[current_drive].irqdelay_oneshot, drive_functions[current_drive].irqdelay_oneshot_len, "irqdelay_oneshot");

    test_cia_sdr(fd, drv, &drive_functions[current_drive]);

    // reset the drive
    cbm_exec_command(fd, drv, "UJ:", 0);
}


int ARCH_MAINDECL main(int argc, char *argv[])
{
    unsigned char drv = argc > 1 ? arch_atoc(argv[1]) : 8;
    CBM_FILE fd;

    if(cbm_driver_open_ex(&fd, NULL) == 0)
    {
        enum cbm_device_type_e devicetype = cbm_dt_unknown;
        const char *drivetype_str;

        if (cbm_identify(fd, drv, &devicetype, &drivetype_str)) {
            printf("Could not identify drive! Assuming 1571...\n");
        }
        else {
            printf("Identified drive %d: as %s\n", drv, drivetype_str);

            switch (devicetype) {
                case cbm_dt_cbm1570:
                case cbm_dt_cbm1571:
                    set_1571();
                    break;

                case cbm_dt_cbm1581:
                    set_1581();
                    break;

                default:
                    printf("Device does not contain a CIA 6526, aborting...\n");
                    exit(1);
            };
        }

        testdrive(fd, drv);

        cbm_driver_close(fd);
        return 0;
    }
    return 1;
}
